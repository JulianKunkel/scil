#include <scil-compression-chain.h>

#include <scil-compressor.h>
#include <scil-error.h>
#include <scil-debug.h>

#include <string.h>

int scilU_chain_create(scil_compression_chain_t* chain, const char* str_in)
{
    char *saveptr, *token;
    char str[4096];
    strncpy(str, str_in, 4096);
    token = strtok_r(str, ",", &saveptr);

    int stage                   = 0; // first pre-conditioner
    chain->precond_first_count  = 0;
    chain->precond_second_count = 0;
    chain->total_size           = 0;

    char lossy = 0;
    for (int i = 0; token != NULL; i++) {
        scilU_algorithm_t* algo = scilU_find_compressor_by_name(token);
        if (algo == NULL) {
            printf("Error: could not find compressor: %s\n", token);
            return SCIL_EINVAL;
        }
        chain->total_size++;
        lossy += algo->is_lossy;
        switch (algo->type) {
            case (SCIL_COMPRESSOR_TYPE_DATATYPES_PRECONDITIONER_FIRST): {
                if (stage != 0) {
                    return -1; // INVALID CHAIN 
                }
                chain->pre_cond_first[(int)chain->precond_first_count] = algo;
                chain->precond_first_count++;
                break;
            }
			case (SCIL_COMPRESSOR_TYPE_DATATYPES_CONVERTER) : {
				if (stage != 0) {
                    return -1; // INVALID CHAIN
                }
                stage                  = 1;
                chain->converter = algo;
                break;
			}
			case (SCIL_COMPRESSOR_TYPE_DATATYPES_PRECONDITIONER_SECOND): {
                if (stage != 1) {
                    return -1; // INVALID CHAIN 
                }
                chain->pre_cond_second[(int)chain->precond_second_count] = algo;
                chain->precond_second_count++;
                break;
            }
            case (SCIL_COMPRESSOR_TYPE_DATATYPES): {
                if (stage > 1) {
                    return -1; // INVALID CHAIN
                }
                stage                  = 2;
                chain->data_compressor = algo;
                break;
            }
			case (SCIL_COMPRESSOR_TYPE_INDIVIDUAL_BYTES): {
                if (stage > 2) {
                    return -1; // INVALID CHAIN
                }
                stage                  = 3;
                chain->byte_compressor = algo;
                break;
            }
        }
        token = strtok_r(NULL, ",", &saveptr);
    }
    chain->is_lossy = lossy > 0;

    // at least one algo should be set
    if (chain->total_size == 0) {
        return SCIL_EINVAL;
    }

    return SCIL_NO_ERR;
}

int scilU_chain_is_applicable(const scil_compression_chain_t* chain, SCIL_Datatype_t datatype){
  // TODO complete me
  if(chain->data_compressor){
    scilU_algorithm_t* algo = chain->data_compressor;
    switch (datatype) {
      case (SCIL_TYPE_FLOAT):
        if ( ! algo->c.DNtype.compress_float ){
          return SCIL_EINVAL;
        }
        break;
      case (SCIL_TYPE_DOUBLE):
        if ( ! algo->c.DNtype.compress_double ){
          return SCIL_EINVAL;
        }
        break;
      case (SCIL_TYPE_INT8) :
        if ( ! algo->c.DNtype.compress_int8 ){
          return SCIL_EINVAL;
        }
        break;
      case(SCIL_TYPE_INT16) :
        if ( ! algo->c.DNtype.compress_int16){
          return SCIL_EINVAL;
        }
        break;
      case(SCIL_TYPE_INT32) :
        if ( ! algo->c.DNtype.compress_int32 ){
          return SCIL_EINVAL;
        }
        break;
      case(SCIL_TYPE_INT64) :
        if ( ! algo->c.DNtype.compress_int64 ){
          return SCIL_EINVAL;
        }
        break;
      case(SCIL_TYPE_UNKNOWN) :
      case(SCIL_TYPE_BINARY) :
      case(SCIL_TYPE_STRING) :
        return SCIL_EINVAL;
      }
  }else if (chain->converter){
    // elementary datatype must be supported.
    scilU_algorithm_t* algo = chain->converter;
    switch (datatype) {
    case (SCIL_TYPE_FLOAT):
      if ( ! algo->c.Ctype.compress_float ){
        return SCIL_EINVAL;
      }
      break;
    case (SCIL_TYPE_DOUBLE):
      if ( ! algo->c.Ctype.compress_double ){
        return SCIL_EINVAL;
      }
      break;
    case (SCIL_TYPE_INT8) :
      if ( ! algo->c.Ctype.compress_int8 ){
        return SCIL_EINVAL;
      }
      break;
    case(SCIL_TYPE_INT16) :
      if ( ! algo->c.Ctype.compress_int16){
        return SCIL_EINVAL;
      }
      break;
    case(SCIL_TYPE_INT32) :
      if ( ! algo->c.Ctype.compress_int32 ){
        return SCIL_EINVAL;
      }
      break;
    case(SCIL_TYPE_INT64) :
      if ( ! algo->c.Ctype.compress_int64 ){
        return SCIL_EINVAL;
      }
      break;
    case(SCIL_TYPE_UNKNOWN) :
    case(SCIL_TYPE_BINARY) :
    case(SCIL_TYPE_STRING) :
      return SCIL_EINVAL;
    }
  }
  return SCIL_NO_ERR;
}

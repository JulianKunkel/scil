#include <scil-chain.h>

#include <scil-algorithm.h>
#include <scil-error.h>
#include <scil-internal.h>

#include <string.h>

int scilI_create_chain(scilI_chain_t* chain, const char* str_in)
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
        scilI_algorithm_t* algo = scilI_find_compressor_by_name(token);
        if (algo == NULL) {
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

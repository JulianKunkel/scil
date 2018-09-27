#include <scil-quantizer.h>
#include <scil-error.h>

#include <assert.h>

#include <scil-util.h>

//Supported datatypes: float double
// Repeat for each data type


uint8_t scil_calculate_bits_needed_<DATATYPE>(<DATATYPE> minimum, <DATATYPE> maximum,  double absolute_tolerance, int reserved_numbers, uint64_t * next_free_number){
  absolute_tolerance = absolute_tolerance * 2.0;
  if(absolute_tolerance <= 0.0 || (double)(maximum - minimum) < absolute_tolerance){
    if(next_free_number != NULL){
      *next_free_number = 0;
    }
    return 0;
  }
  assert((next_free_number == NULL && reserved_numbers == 0) || (next_free_number != NULL) );
  double mx = ((double) maximum - (double) minimum) / absolute_tolerance;
  if(next_free_number != NULL){
    *next_free_number = (uint64_t) (mx + 1.5);
    return (uint8_t) ceil( log2( mx + 1.5 + reserved_numbers ));
  }
  return (uint8_t) ceil( log2( mx + reserved_numbers ));
}

int scil_quantize_buffer_minmax_<DATATYPE>(uint64_t* restrict dest,
                                           const <DATATYPE>* restrict source,
                                           size_t count,
                                           double absolute_tolerance,
                                           <DATATYPE> minimum,
                                           <DATATYPE> maximum){

    assert(dest != NULL);
    assert(source != NULL);

    double real_tolerance = (1 / 1.0) / absolute_tolerance;
    double min_fixed = (double) minimum;

    for(size_t i = 0; i < count; ++i){
        dest[i] = (((uint64_t) ( ((double) source[i] - min_fixed) * real_tolerance )) + 1)>>1;
    }

    return SCIL_NO_ERR;
}

int scil_unquantize_buffer_<DATATYPE>(<DATATYPE>* restrict dest,
                                      const uint64_t* restrict source,
                                      size_t count,
                                      double absolute_tolerance,
                                      <DATATYPE> minimum){

    assert(dest != NULL);
    assert(source != NULL);

    double real_tolerance = 2.0 * absolute_tolerance;

    for(size_t i = 0; i < count; ++i){
        dest[i] = (<DATATYPE>)minimum + (<DATATYPE>)(source[i] * real_tolerance);
    }

    return SCIL_NO_ERR;
}


int scil_quantize_buffer_minmax_fill_<DATATYPE>(uint64_t* restrict dest,
                                           const <DATATYPE>* restrict source,
                                           size_t count,
                                           double absolute_tolerance,
                                           <DATATYPE> minimum,
                                           <DATATYPE> maximum,
                                           double fill_value,
                                           uint64_t next_free_number){

    assert(dest != NULL);
    assert(source != NULL);

    double real_tolerance = (1 / 1.0) / absolute_tolerance;
    double min_fixed = (double) minimum;

    for(size_t i = 0; i < count; ++i){
      if(source[i] != fill_value){
        dest[i] = (((uint64_t) ( ((double) source[i] - min_fixed) * real_tolerance )) + 1)>>1;
      }else{
        dest[i] = next_free_number;
      }
    }

    return SCIL_NO_ERR;
}

int scil_unquantize_buffer_fill_<DATATYPE>(<DATATYPE>* restrict dest,
                                      const uint64_t* restrict source,
                                      size_t count,
                                      double absolute_tolerance,
                                      <DATATYPE> minimum,
                                      double fill_value,
                                      uint64_t next_free_number){

    assert(dest != NULL);
    assert(source != NULL);

    double real_tolerance = 2.0 * absolute_tolerance;

    for(size_t i = 0; i < count; ++i){
      if(source[i] != next_free_number){
        dest[i] = (<DATATYPE>)minimum + (<DATATYPE>)(source[i] * real_tolerance);
      }else{
        dest[i] = fill_value;
      }
    }

    return SCIL_NO_ERR;
}





int scil_quantize_buffer_<DATATYPE>(uint64_t* restrict dest,
                                    const <DATATYPE>* restrict source,
                                    size_t count,
                                    double absolute_tolerance){

    assert(dest != NULL);
    assert(source != NULL);

    <DATATYPE> minimum, maximum;
    scilU_find_minimum_maximum_<DATATYPE>(source, count, &minimum, &maximum);

    return scil_quantize_buffer_minmax_<DATATYPE>(dest, source, count, absolute_tolerance, minimum, maximum);
}
// End repeat

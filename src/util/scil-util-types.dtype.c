#include <assert.h>

#include <scil-util.h>
#include <scil-debug.h>

//Supported datatypes: int8_t int16_t int32_t int64_t float double
// Repeat for each data type

void scilU_find_minimum_maximum_with_excluded_points_<DATATYPE>(const <DATATYPE>* restrict buffer, size_t count, <DATATYPE>* minimum, <DATATYPE>* maximum, double ignore_up_to, double ignore_from, double fill_value){

    assert(buffer != NULL);
    assert(minimum != NULL);
    assert(maximum != NULL);

    <DATATYPE> min = INFINITY_<DATATYPE>;
    <DATATYPE> max = NINFINITY_<DATATYPE>;
    if (fill_value != DBL_MAX && ignore_up_to != -DBL_MAX && ignore_from != DBL_MAX){
      for(size_t i = 0; i < count; ++i){
        if (fabs(buffer[i] - fill_value) < FLT_EPSILON){
          debugI("%s Ignored some fill value\n", __PRETTY_FUNCTION__);
          continue;
        }
        if ((double) buffer[i] > ignore_from)  continue;
        if ((double) buffer[i] < ignore_up_to) continue;
        if (buffer[i] < min) { min = buffer[i]; }
        if (buffer[i] > max) { max = buffer[i]; }
      }
    }else if (fill_value != DBL_MAX){
      for(size_t i = 0; i < count; ++i){
        if (fabs(buffer[i] - fill_value) < FLT_EPSILON){
          debugI("%s Ignored some fill value\n", __PRETTY_FUNCTION__);
          continue;
        }
        if (buffer[i] < min) { min = buffer[i]; }
        if (buffer[i] > max) { max = buffer[i]; }
      }
    } else if (ignore_up_to == -DBL_MAX && ignore_from == DBL_MAX){
      for(size_t i = 0; i < count; ++i){
        if (buffer[i] < min) { min = buffer[i]; }
        if (buffer[i] > max) { max = buffer[i]; }
      }
    }else{
      for(size_t i = 0; i < count; ++i){
        if ((double) buffer[i] > ignore_from)  continue;
        if ((double) buffer[i] < ignore_up_to) continue;
        if (buffer[i] < min) { min = buffer[i]; }
        if (buffer[i] > max) { max = buffer[i]; }
      }
    }

    *minimum = min;
    *maximum = max;
}


void scilU_find_minimum_maximum_<DATATYPE>(const <DATATYPE>* restrict buffer,
                                          size_t count,
                                          <DATATYPE>* minimum,
                                          <DATATYPE>* maximum){

    assert(buffer != NULL);
    assert(minimum != NULL);
    assert(maximum != NULL);

    <DATATYPE> min = INFINITY_<DATATYPE>;
    <DATATYPE> max = NINFINITY_<DATATYPE>;

    for(size_t i = 0; i < count; ++i){
        if (buffer[i] < min) { min = buffer[i]; }
        if (buffer[i] > max) { max = buffer[i]; }
    }

    *minimum = min;
    *maximum = max;
}

void scilU_subtract_data_<DATATYPE>(const <DATATYPE>* restrict in, <DATATYPE>* restrict inout, size_t count){
  for(size_t i = 0 ; i < count; i++){
    inout[i] = in[i] - inout[i];
  }
}

// End repeat

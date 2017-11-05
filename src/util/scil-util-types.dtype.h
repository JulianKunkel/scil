#ifndef SCIL_UTIL_TYPES_H
#define SCIL_UTIL_TYPES_H

#include <math.h>
#include <float.h>
#include <limits.h>

#include <scil-datatypes.h>

#define INFINITY_double INFINITY
#define INFINITY_float INFINITY

#define NINFINITY_double -INFINITY
#define NINFINITY_float -INFINITY

#define INFINITY_int8_t CHAR_MAX
#define NINFINITY_int8_t CHAR_MIN

#define INFINITY_int16_t SHRT_MAX
#define NINFINITY_int16_t SHRT_MIN

#define INFINITY_int32_t INT_MAX
#define NINFINITY_int32_t INT_MIN

#define INFINITY_int64_t LONG_MAX
#define NINFINITY_int64_t LONG_MIN

#define MAX_EXPONENT_double 2047
#define MAX_EXPONENT_float 255

#define MAX_PRECISION_double 15
#define MAX_PRECISION_float 7

#define FLT_FINEST_SUB_double  0.0000000000001
#define FLT_FINEST_SUB_float 0.000001


//Supported datatypes: int8_t int16_t int32_t int64_t float double
// Repeat for each data type

/**
 Compute the difference between two data vectors.
 Subtract in from inout and store the result in inout
 */
void scilU_subtract_data_<DATATYPE>(const <DATATYPE>* restrict in, <DATATYPE>* restrict inout, size_t count);


/**
 * \brief Finds the smallest and biggest values of a buffer and stores them
 *        to the given memory locations.
 * \param buffer The buffer to scan
 * \param count Element count of the buffer
 * \param minimum The memory location where the smallest value will be stored
 * \param maximum The memory location where the biggest value will be stored
 * \pre buffer != NULL
 * \pre minimum != NULL
 * \pre maximum != NULL
 * \return Biggest value in the buffer
 */
void scilU_find_minimum_maximum_<DATATYPE>(const <DATATYPE>* restrict buffer,
                                          size_t count,
                                          <DATATYPE>* minimum,
                                          <DATATYPE>* maximum);

void scilU_find_minimum_maximum_with_excluded_points_<DATATYPE>(const <DATATYPE>* restrict buffer,
                                          size_t count,
                                          <DATATYPE>* minimum,
                                          <DATATYPE>* maximum,
                                          double ignore_up_to, double ignore_from, double fill_value);

// End repeat

#endif

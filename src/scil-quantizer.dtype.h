#ifndef SCIL_QUANTIZER_H_
#define SCIL_QUANTIZER_H_

#include <stdlib.h>
#include <stdint.h>

//Supported datatypes: int8_t int16_t int32_t int64_t float double
// Repeat for each data type
/**
 * \brief Finds and returns the smallest value of the given buffer.
 * \param buffer The buffer to scan
 * \param count Element count of the buffer
 * \pre buffer != NULL
 * \return Smallest value in the buffer
 */
<DATATYPE> scil_find_minimum_<DATATYPE>(const <DATATYPE>* buffer,
                                        size_t count);

/**
 * \brief Finds and returns the biggest value of the given buffer.
 * \param buffer The buffer to scan
 * \param count Element count of the buffer
 * \pre buffer != NULL
 * \return Biggest value in the buffer
 */
<DATATYPE> scil_find_maximum_<DATATYPE>(const <DATATYPE>* buffer,
                                        size_t count);



/**
 * \brief Calculates how many bit are needed per value, considering
 *        the quantization relevant parameters.
 * \param minimum The minimum value to quantize
 * \param maximum The maximum value to quantize
 * \param absolute_tolerance The maximum, tolerated, absolute error
 * \return Bits needed per value
 */
uint64_t scil_calculate_bits_needed_<DATATYPE>(<DATATYPE> minimum,
                                               <DATATYPE> maximum,
                                               double absolute_tolerance);

/**
 * \brief Quantizes the values of the given buffer with a known minimum
 *        and maximum value.
 * \param buf_in The Buffer containing the data to quantize
 * \param buf_out The Buffer which will hold the quantized data.
 * \param count Number of elements in the buffer
 * \param absolute_tolerance The maximal tolerated error through quantizing
 * \param minimum The minimum value of the buffer
 * \param minimum The maximum value of the buffer
 * \pre buf_out != NULL
 * \pre buf_in != NULL
 * \return SCIL error code
 */
int scil_quantize_buffer_minmax_<DATATYPE>(uint64_t* restrict buf_out,
                                           const <DATATYPE>* restrict buf_in,
                                           size_t count,
                                           double absolute_tolerance,
                                           <DATATYPE> minimum,
                                           <DATATYPE> maximum);

/**
* \brief Quantizes the values of the given buffer.
* \param buf_in The Buffer containing the data to quantize
* \param buf_out The Buffer which will hold the quantized data.
* \param count Number of elements in the buffer
* \param absolute_tolerance The maximal tolerated error through quantizing
* \pre buf_out != NULL
* \pre buf_in != NULL
* \return 0 (success) or
*         1 (quantizing would result in integer values bigger than UINT64_MAX)
*/
int scil_quantize_buffer_<DATATYPE>(uint64_t* restrict buf_out,
                                    const <DATATYPE>* restrict buf_in,
                                    size_t count,
                                    double absolute_tolerance);

/**
 * \brief Unquantizes the values of the given buffer.
 * \param buf_in The Buffer containing the data to unquantize
 * \param buf_out The Buffer which will hold the unquantized data.
 * \param count Number of elements in the buffer
 * \param absolute_tolerance The maximal tolerated error through quantizing
 * \param minimum The minimum value of the buffer
 * \pre buf_out != NULL
 * \pre buf_in != NULL
 * \return SCIL error code
 */
int scil_unquantize_buffer_<DATATYPE>(<DATATYPE>* restrict buf_out,
                                      const uint64_t* restrict buf_in,
                                      size_t count,
                                      double absolute_tolerance,
                                      <DATATYPE> minimum);

// End repeat

#endif /* SCIL_QUANTIZER_H_<DATATYPE> */

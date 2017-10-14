#ifndef SCIL_QUANTIZER_H_
#define SCIL_QUANTIZER_H_

#include <stdlib.h>
#include <stdint.h>

//Supported datatypes: int8_t int16_t int32_t int64_t float double
// Repeat for each data type


/**
 * \brief Calculates how many bit are needed per value, considering
 *        the quantization relevant parameters.
 * \param minimum The minimum value to quantize
 * \param maximum The maximum value to quantize
 * \param absolute_tolerance The maximum, tolerated, absolute error
 * \param reserved_numbers additionally reserved numbers to distinguish
 * \param the next free integer number, max be used for the reserved numbers
 * \return Bits needed per value
 */
uint8_t scil_calculate_bits_needed_<DATATYPE>(<DATATYPE> minimum, <DATATYPE> maximum, double absolute_tolerance, int reserved_numbers, uint64_t * next_number);

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
int scil_quantize_buffer_minmax_fill_<DATATYPE>(uint64_t* restrict buf_out,
                                          const <DATATYPE>* restrict buf_in,
                                          size_t count,
                                          double absolute_tolerance,
                                          <DATATYPE> minimum,
                                          <DATATYPE> maximum,
                                          double fill, uint64_t next_free_number);
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
int scil_unquantize_buffer_fill_<DATATYPE>(<DATATYPE>* restrict buf_out,
                                      const uint64_t* restrict buf_in,
                                      size_t count,
                                      double absolute_tolerance,
                                      <DATATYPE> minimum,
                                      double fill_value, uint64_t next_free_number);
// End repeat

#endif /* SCIL_QUANTIZER_H_<DATATYPE> */

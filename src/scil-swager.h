#ifndef SCIL_SWAGER_H
#define SCIL_SWAGER_H

#include <stdlib.h>
#include <stdint.h>

#include <scil.h>

/**
 * \brief Packs data in a given buffer bit-perfectly
 * \param buf_out Destination buffer for packed data
 * \param buf_in Source buffer of unpacked data
 * \param count Element count in unpacked buffer
 * \param bits_per_value Bit size of each compressed value
 * \pre buf_out != NULL
 * \pre buf_in != NULL
 * \return scil error code
 */
int scil_swage(byte* restrict buf_out,
               const uint64_t* restrict buf_in,
               const size_t count,
               const uint8_t bits_per_value);

/**
 * \brief Unacks data from a bit-perfectly packed buffer
 * \param buf_out Destination buffer for unpacked data
 * \param buf_in Source buffer of packed data
 * \param count Element count in unpacked buffer
 * \param bits_per_value Bit size of each compressed value
 * \pre buf_out != NULL
 * \pre buf_in != NULL
 * \return scil error code
 */
int scil_unswage(uint64_t* restrict buf_out,
                 const byte* restrict buf_in,
                 const size_t count,
                 const uint8_t bits_per_value);

#endif /* SCIL_SWAGER_H */

#ifndef SCIL_DIMS_H
#define SCIL_DIMS_H

#include <stdint.h>
#include <stdlib.h>

#include <scil-datatypes.h>

/*
 This amount of data may be needed for a block header.
 */
#define SCIL_BLOCK_HEADER_MAX_SIZE 1024

/** \brief Struct to contain the dimensional configuration of data. */
typedef struct
{
    /** \brief Number of dimensions. */
    uint8_t dims;

    // dimensions as long as they are <= 4
    size_t length[4];
} scil_dims;

void scil_init_dims_1d(scil_dims* dims, size_t dim1);
void scil_init_dims_2d(scil_dims* dims, size_t dim1, size_t dim2);
void scil_init_dims_3d(scil_dims* dims, size_t dim1, size_t dim2, size_t dim3);
void scil_init_dims_4d(scil_dims* dims, size_t dim1, size_t dim2, size_t dim3, size_t dim4);

/*
 */
void scil_init_dims_array(scil_dims* dims,
                          uint8_t dimensions_count,
                          const size_t* dimensions_length);

void scil_copy_dims_array(scil_dims* out_dims, scil_dims in_dims);

/*
 * \brief Method to get the number of actual data points in multidimensional
 * data.
 * \param dims Information about the dimensional configuration of the data
 * \return Number of data points in the data
 */
size_t scil_get_data_count(const scil_dims* dims);

size_t scil_get_data_size(enum SCIL_Datatype type, const scil_dims* dims);

/*
 * \brief Return the minimum size of the compression buffer needed.
 */
size_t scil_compress_buffer_size_bound(enum SCIL_Datatype datatype, const scil_dims* dims);

#endif // SCIL_DIMS_H

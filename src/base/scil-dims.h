#ifndef SCIL_DIMS_H
#define SCIL_DIMS_H

#include <stdint.h>
#include <stdlib.h>

/*
 This amount of data may be needed for a block header.
 */
#define SCIL_BLOCK_HEADER_MAX_SIZE 1024

#define SCIL_DIMS_MAX 5 //4

/** \brief Struct to contain the dimensional configuration of data. */
typedef struct scil_dims
{
    /** \brief Number of dimensions. */
    uint8_t dims;

    /** \brief Array containing each dimensions element count. */
    size_t length[SCIL_DIMS_MAX];
} scil_dims_t;

void scil_dims_initialize_1d(scil_dims_t* dims, size_t dim1);
void scil_dims_initialize_2d(scil_dims_t* dims, size_t dim1, size_t dim2);
void scil_dims_initialize_3d(scil_dims_t* dims, size_t dim1, size_t dim2, size_t dim3);
void scil_dims_initialize_4d(scil_dims_t* dims, size_t dim1, size_t dim2, size_t dim3, size_t dim4);
void scil_dims_initialize_5d(scil_dims_t* dims, size_t dim1, size_t dim2, size_t dim3, size_t dim4, size_t dim5);

/*
 */
void scil_dims_initialize_array(scil_dims_t* dims,
                                  uint8_t dimensions_count,
                                  const size_t* dimensions_length);

void scil_dims_copy(scil_dims_t* out_dims, const scil_dims_t* in_dims);

/*
 * \brief Returns the number of actual data points in multidimensional
 * data.
 * \param dims Dimensional configuration of the data
 * \return Number of data points in the data
 */
size_t scil_dims_get_count(const scil_dims_t* dims);

#endif // SCIL_DIMS_H

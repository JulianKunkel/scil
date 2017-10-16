#ifndef SCIL_DIMS_H
#define SCIL_DIMS_H

#include <stdint.h>
#include <stdlib.h>

/*
 This amount of data may be needed for a block header.
 */
#define SCIL_BLOCK_HEADER_MAX_SIZE 1024

#define SCIL_DIMS_MAX 4

/** \brief Struct to contain the dimensional configuration of data. */
typedef struct scil_dims
{
    /** \brief Number of dimensions. */
    uint8_t dims;

    /** \brief Array containing each dimensions element count. */
    size_t length[SCIL_DIMS_MAX];
} scil_dims_t;

void scilPr_initialize_dims_1d(scil_dims_t* dims, size_t dim1);
void scilPr_initialize_dims_2d(scil_dims_t* dims, size_t dim1, size_t dim2);
void scilPr_initialize_dims_3d(scil_dims_t* dims, size_t dim1, size_t dim2, size_t dim3);
void scilPr_initialize_dims_4d(scil_dims_t* dims, size_t dim1, size_t dim2, size_t dim3, size_t dim4);

/*
 */
void scilPr_initialize_dims_array(scil_dims_t* dims,
                                  uint8_t dimensions_count,
                                  const size_t* dimensions_length);

void scilPr_copy_dims(scil_dims_t* out_dims, const scil_dims_t* in_dims);

/*
 * \brief Returns the number of actual data points in multidimensional
 * data.
 * \param dims Dimensional configuration of the data
 * \return Number of data points in the data
 */
size_t scilPr_get_dims_count(const scil_dims_t* dims);

#endif // SCIL_DIMS_H

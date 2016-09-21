// This file is part of SCIL.
//
// SCIL is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SCIL is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with SCIL.  If not, see <http://www.gnu.org/licenses/>.
/**
 * \file
 * \brief Header containing the Scientific Compression Interface Library
 * \author Julian Kunkel <juliankunkel@googlemail.com>
 * \author Armin Schaare <3schaare@informatik.uni-hamburg.de>

\mainpage SCIL - Scientific Compression Interface Library

@startuml{scil-components.png}
title Components of SCIL

folder "src/" {

  folder "compression" {
    component "algo-chooser" #Wheat
    component "chain execution" #Wheat
  }

  folder "util"{
    component "print" #Wheat
    component "accuracy" #Wheat
    component "validate" #Wheat
    component "error" #Wheat
  }

  folder "prepare"{
    component "memory-allocation" #Wheat
    component "user hints" #Wheat
    component "context" #Wheat
    component "dims" #Wheat
  }

  folder "internal"{
    component "memory" #Wheat
    component "timer" #Wheat
    component "marshalling" #Wheat
    component "data statistics" #Wheat
    component "data conversion" #Wheat
    component "dictionary" #Wheat
    component "grid iterators" #Wheat
  }

  folder "algo"{
    frame "FP Preconditoner"{
    }
    frame "FP Converter INT"{
    }
    frame "INT Preconditioner"{
    }
    frame "DATATYPE Byte"{
    }
    frame "Byte"{
    }
  }

  folder "pattern"{
      interface "constant" #Blue
      interface "random" #Blue
      interface "steps" #Blue
      interface "sin" #Blue
      interface "simplex_noise" #Blue
      interface "poly4" #Blue

    frame "mutators"{
      ' Mutators are only applicable to double
      interface "interpolator" #Orange
      interface "repeater" #Orange

      interface "noise" #Orange
    }

    frame "noise" #white{
      'They are implemented using mutators
      interface "white" #green
      interface "brownian/red" #green
      interface "pink" #green
    }
  }

  folder "tools" {
    artifact [scil-benchmark]
    artifact [scil-pattern-plotter]
    artifact [scil-compress-csv]
    artifact [scil-compress-csv]
    artifact [scil-gen-chooser-data]
    artifact [scil-plot-csv.py]
    artifact [scil-plot-csv.R]
  }

  folder "test"{
    folder "complex"{
    }
    'note left of complex: Advanced test suite, not run automatically
  }
  'note left of test: Tests that are run automatically, they do not depend on arguments
}

folder "Additional tools in tools/"{
  folder "hdf5-plugin"{
    component [libhdf5-filter-scil]
  }
}
@enduml

@startuml{scil-public-interfaces.png}
title Interfaces of SCIL

folder "src/" {

  frame "libscil-patterns"{
    interface "scil-pattern.h" #Purple
    interface "scil-mutators.h" #Purple
    interface "scil-noise.h" #Purple
  }

  frame "libscil" {
      interface "scil.h" #Orange
      interface "scil-compression.h" #Orange
      interface "scil-util.h" #Orange
      interface "scil-prepare.h" #Orange

      frame "internals" #LightYellow{
        interface "scil-internal-memory.h"
        interface "scil-internal-timer.h"
        interface "scil-internal-marshalling.h"
        interface "scil-internal-conversion.h"
        interface "scil-internal-dictionary.h"
        interface "scil-internal-grid.h"
      }
  }
}


folder "Install directory"{

  folder "share/scil/"{
    artifact [scil.conf]
  }
  folder "lib/pkgconfig"{
    artifact [scil.pc]
  }

  folder "include/"{
  artifact [ scil.h]
  artifact [ scil-compression.h]
  artifact [ scil-util.h]
  artifact [ scil-prepare.h]

  artifact [ scil-pattern.h]
  artifact [ scil-mutators.h]
  artifact [ scil-noise.h]
  }

  folder "lib/"{
    artifact [libscil-patterns.so] #PowderBlue
    artifact [libscil.so] #PowderBlue
    artifact [libhdf5-filter-scil.so] #PowderBlue
  }

  folder "bin/"{
    artifact [scil-benchmark] #LightGreen
    artifact [scil-gen-chooser-data] #LightGreen
    artifact [scil-compress-csv] #LightGreen
    artifact [scil-pattern-plotter] #LightGreen
    artifact [scil-add-noise-csv] #LightGreen
    artifact [scil-plot-csv.R] #LightGreen
    artifact [scil-plot-csv.py] #LightGreen
  }
}
@enduml

@startuml{scil-public-artefacts.png}
   title Interfaces of SCIL

   folder "Core in src/" {

     folder "pattern"{
       frame "libscil-patterns"{
         interface "scil-patterns.h" #Purple
       }
     }

     frame "libscil" {
         'component X #PowderBlue
         interface "scil.h" #Orange
         component "scil-algo-chooser" #Wheat

         'note left of X
         'end note
         '[Thread] ..> [SIOX-LL] : use
       }

     folder "tools" {
       artifact [scil-benchmark]
     }
   }

   actor admin
   admin --> [scil-benchmark] : runs

   folder "Install directory"{
     artifact [scil.conf]
   }

   [scil-benchmark]..> [scil.conf] : creates
   [scil-algo-chooser]..> [scil.conf] : reads

   [scil-algo-chooser]-->[scil-patterns.h] : use

   folder "Additional tools in tools/"{
     folder "hdf5-plugin"{
       component [libhdf5-filter-scil]
     }
   }

   [libhdf5-filter-scil] --> [scil.h] : use
 @enduml
  */

#ifndef SCIL_HEADER_
#define SCIL_HEADER_

#include <stdint.h>
#include <stdlib.h>

#include <scil-dict.h>
#include <scil-error.h>

#include <scil-prepare.h>

typedef unsigned char byte;
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

enum SCIL_Datatype {
  SCIL_TYPE_FLOAT = 0,
  SCIL_TYPE_DOUBLE,
  SCIL_TYPE_INT8,
  SCIL_TYPE_INT16,
  SCIL_TYPE_INT32,
  SCIL_TYPE_INT64,
  SCIL_TYPE_STRING
};

/*
 This amount of data may be needed for a block header.
 */
#define SCIL_BLOCK_HEADER_MAX_SIZE 1024

typedef struct scil_context_t* scil_context_p;

/** \brief Struct to contain the dimensional configuration of data. */
typedef struct
{
    /** \brief Number of dimensions. */
    uint8_t dims;

    // dimensions as long as they are <= 4
    size_t length[4];
} scil_dims;

/*
 \brief Returns the number of available compression schemes.
 */
int scil_compressors_available();
const char* scil_compressor_name(int num);
int scil_compressor_num_by_name(const char* name);

void scil_compression_sprint_last_algorithm_chain(scil_context_p ctx,
                                                  char* out,
                                                  int buff_length);

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

scilPr_user_hints_t scil_retrieve_effective_hints(scil_context_p ctx);

/**
 * \brief Creation of a compression context
 * \param datatype The datatype of the data (float, double, etc...)
 * \param out_ctx reference to the created context
 * \param hints information on the tolerable error margin
 * \pre hints != NULL
 * \param special values are special values that must be preserved, we support a list of  values
 * \return success state of the creation
 */
int scil_create_compression_context(scil_context_p* out_ctx,
                                    enum SCIL_Datatype datatype,
                                    int special_values_count,
                                    void * special_values,
                                    const scilPr_user_hints_t* hints);

int scil_destroy_compression_context(scil_context_p* out_ctx);

/**
 * \brief Method to compress a data buffer
 * \param dest Destination of the compressed buffer
 * \param dest_size Reference to the compressed buffer byte size, max size is
 * given as argument, if a pipeline is processed it should be 3x the memory size
 * of the input size + the HEADER.
 * \param source Source buffer of the data to compress
 * \param dims struct containing information about dimension count and length of
 * buffer in each dimension
 * \param ctx Reference to the compression context
 * \pre datatype == 0 || datatype == 1
 * \pre dest != NULL
 * \pre dest_size != NULL
 * \pre source != NULL
 * \pre ctx != NULL
 * \return Success state of the compression
 */
int scil_compress(byte* restrict dest,
                  size_t dest_size,
                  void* restrict source,
                  scil_dims* dims,
                  size_t* restrict out_size,
                  scil_context_p ctx);

/**
 * \brief Method to decompress a data buffer
 * \param datatype The datatype of the data (float, double, etc...)
 * \param dest Destination of the decompressed buffer
 * \param expected_dims Dimensional information about the decompressed buffer
 * \param source Source buffer of data to decompress
 * \param source_size Byte size of compressed data source buffer
 * \pre datatype == 0 || datatype == 1
 * \pre dest != NULL
 * \pre source != NULL
 * \pre tmp_buff != NULL with a size of scil_compress_buffer_size_bound() / 2
 * \return Success state of the decompression
 */
int scil_decompress(enum SCIL_Datatype datatype,
                    void* restrict dest,
                    scil_dims* const expected_dims,
                    byte* restrict source,
                    const size_t source_size,
                    byte* restrict tmp_buff);

void scil_determine_accuracy(enum SCIL_Datatype datatype,
                             const void* restrict data_1,
                             const void* restrict data_2,
                             scil_dims* dims,
                             const double relative_err_finest_abs_tolerance,
                             scilPr_user_hints_t* out_hints);

/**
 \brief Test method: check if the conditions as specified by ctx are met by
 comparing compressed and decompressed data.
 out_accuracy contains a set of hints with the observed finest
 resolution/required precision to accept the data.
 */
int scil_validate_compression(enum SCIL_Datatype datatype,
                              const void* restrict data_uncompressed,
                              scil_dims* dims,
                              byte* restrict data_compressed,
                              const size_t compressed_size,
                              const scil_context_p ctx,
                              scilPr_user_hints_t* out_accuracy);

#endif

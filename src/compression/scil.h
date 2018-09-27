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

@startuml{scil-refactored.png}
title Refactoring Proposal

package "public interface" {

    class "util" {
        subtract_data_<T>()
        find_minimum_maximum_<T>()
        find_minimum_maximum_with_excluded_points_<T>()
        determine_accuracy()
        validate_compresssion()
    }

    class "compress" {
        get_compressor_number()
        get_compressor_name()
        abstol_compress_<T>()
        abstol_decompress_<T>()
        algo_chooser_initialize()
        algo_chooser_execute()
    }

    class "patterns" {
        create_library_patterns_if_needed()
        create_pattern()
        change_data_scale()
    }

    class "noise" {
        void some_method()
    }

    class "file_format" {
        void some_method()
    }
}

package "io" {
    file_format -> csv
    file_format -> bin
    file_format -> netcdf
    file_format -> "file-plugin"
    file_format -> "brick-of-floats"
}


class "brick-of-floats"

"algo-utils" -down-> option
patterns -> "simplex-noise"
patterns -> "basic-patterns"

class "simplex-noise" {

}

class "basic-patterns" {

}

class "algos" {
}

algos -down-> lz4
algos -down-> gzip


class "algo-utils" {
}

util -> "types"
util -> "error"

"swage" <-down- "algo-utils"
"quantizer" <-down- "algo-utils"

class "swage" {
    swage()
    unswage()
}

class "quantizer" {
}

util -> "scil-core"
noise -> "scil-core"
patterns -> "scil-core"
compress -> "scil-core"
file_format -> "scil-core"
compress -down-> algos
algos -down-> "algo-utils"

class "scil-core" {
}

class "option" {

}

"scil-core" -down-> "scil-chain"
"scil-core" -down-> "scil-dict"
"scil-core" -down-> "scil-internal"
"scil-core" -down-> "scil-algorithm"
"scil-core" -down-> "scil-data-characteristics"
"scil-core" -down-> "scil.hardware-limits"
"scil-core" -down-> "scil-cca"

@enduml

@startuml{scil-components.png}
title Components of SCIL

folder "src/" {

  folder "compression" {
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
    component "algo-chooser" #Wheat
    component "chain execution" #Wheat
  }

  folder "util"{
    component "print" #Wheat
    component "accuracy" #Wheat
    component "validate" #Wheat
    component "grid iterators" #Wheat
    component "memory" #Wheat
    component "timer" #Wheat
    component "marshalling" #Wheat
    component "data statistics" #Wheat
    component "data conversion" #Wheat
    component "dictionary" #Wheat
    component "error" #Wheat
  }

  folder "base"{
    component "memory-allocation" #Wheat
    component "user hints" #Wheat
    component "context" #Wheat
    component "dims" #Wheat
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
      interface "scil.h" #Orange
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

  folder "etc/"{
    artifact [scil.conf]
  }
  folder "lib/pkgconfig"{
    artifact [scil.pc]
  }

  folder "include/"{
  artifact [ scil.h]
  artifact [ scil.h]
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

#ifndef SCIL_H
#define SCIL_H

#include <stdint.h>
#include <stdlib.h>

#include <scil-dims.h>
#include <scil-context.h>
#include <scil-error.h>
#include <scil-user-hints.h>

#include <scil-datatypes.h>

void scil_compression_sprint_last_algorithm_chain(scil_context_t* ctx,
                                                  char* out,
                                                  int buff_length);

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
                  scil_dims_t* dims,
                  size_t* restrict out_size,
                  scil_context_t* ctx);

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
 * \pre tmp_buff != NULL with a size of scil_get_compressed_data_size_limit() / 2
 * \return Success state of the decompression
 */
int scil_decompress(SCIL_Datatype_t datatype,
                    void* restrict dest,
                    scil_dims_t* expected_dims,
                    byte* restrict source,
                    const size_t source_size,
                    byte* restrict tmp_buff);

void scil_determine_accuracy(SCIL_Datatype_t datatype,
                             const void* restrict data_1,
                             const void* restrict data_2,
                             scil_dims_t* dims,
                             const double relative_err_finest_abs_tolerance,
                             scil_user_hints_t *out_hints,
                             scil_validate_params_t *out_validation);

/**
 \brief Test method: check if the conditions as specified by ctx are met by
 comparing compressed and decompressed data.
 out_accuracy contains a set of hints with the observed finest
 resolution/required precision to accept the data.
 */
int scil_validate_compression(SCIL_Datatype_t datatype,
                              const void* restrict data_uncompressed,
                              scil_dims_t* dims,
                              byte* restrict data_compressed,
                              const size_t compressed_size,
                              const scil_context_t* ctx,
                              scil_user_hints_t *out_accuracy,
                              scil_validate_params_t *out_validation);


#endif

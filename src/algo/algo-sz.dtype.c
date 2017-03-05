// This file is part of SCIL.
//
// SCIL is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SCIL is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FpITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with SCIL.  If not, see <http://www.gnu.org/licenses/>.

#include <algo/algo-sz.h>

#include <scil-util.h>

#include <assert.h>
#include <math.h>
#include <string.h>

//Repeat for each data type
//Supported datatypes: double float

int scil_sz_compress_<DATATYPE>(const scil_context_t* ctx,
                                    byte* restrict dest,
                                    size_t* restrict dest_size,
                                    <DATATYPE>* restrict source,
                                    const scil_dims_t* dims){

    return SCIL_NO_ERR;
}

int scil_sz_decompress_<DATATYPE>(<DATATYPE>* restrict dest,
                                      scil_dims_t* dims,
                                      byte* restrict source,
                                      size_t source_size){


    return SCIL_NO_ERR;
}
// End repeat



scilI_algorithm_t algo_sz = {
    .c.DNtype = {
        CREATE_INITIALIZER(scil_sz)
    },
    "sz",
    13,
    SCIL_COMPRESSOR_TYPE_DATATYPES,
    1
};

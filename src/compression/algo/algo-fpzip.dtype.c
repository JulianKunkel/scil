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

#include <assert.h>
#include <algo/algo-fpzip.h>

#include <fpzip.h>

// Repeat for each data type

#pragma GCC diagnostic ignored "-Wunused-parameter"
int scil_fpzip_compress_<DATATYPE>(const scil_context_t* ctx,
                        byte * restrict dest,
                        size_t* restrict dest_size,
                        <DATATYPE>*restrict source,
                        const scil_dims_t* dims)
{
    FPZ* fpz = fpzip_write_to_buffer(dest, *dest_size);
    fpz->type = (SCIL_TYPE_<DATATYPE_UPPER>==SCIL_TYPE_DOUBLE)?1:0; // in fpzip float is 0 and double 1

    fpz->prec = 9 + ctx->hints.significant_bits;

    fpz->nx = 1; //
    fpz->ny = 1; //
    fpz->nz = 1; //
    fpz->nf = 1; //

    assert(dims->dims < 5);
    if(dims->dims < 5){
        switch(dims->dims){
            case 4: fpz->nf = dims->length[3];
            case 3: fpz->nz = dims->length[2];
            case 2: fpz->ny = dims->length[1];
            case 1: fpz->nx = dims->length[0];
        }
    }

    if (! fpzip_write_header(fpz)) {
      fprintf(stderr, "Cannot write header in algo-fpzip compression: %s\n", fpzip_errstr[fpzip_errno]);
      return 1;
    }

    uint64_t outbytes = fpzip_write(fpz, (void*)source);
    if (!outbytes) {
      fprintf(stderr, "Compression failed in algo-fpzip compression: %s\n", fpzip_errstr[fpzip_errno]);
      return 1;
    }

    fpzip_write_close(fpz);

    *dest_size = outbytes;

    return 0;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
int  scil_fpzip_decompress_<DATATYPE>( <DATATYPE>*restrict data_out,
                            scil_dims_t* dims,
                            byte*restrict compressed_buf_in,
                            const size_t in_size)
{
    FPZ* fpz = fpzip_read_from_buffer(compressed_buf_in);
    fpz->nx = 1; //
    fpz->ny = 1; //
    fpz->nz = 1; //
    fpz->nf = 1; //

    assert(dims->dims < 5);
    if(dims->dims < 5){
        switch(dims->dims){
            case 4: fpz->nf = dims->length[3];
            case 3: fpz->nz = dims->length[2];
            case 2: fpz->ny = dims->length[1];
            case 1: fpz->nx = dims->length[0];
        }
    }

    if (!fpzip_read_header(fpz)) {
      fprintf(stderr, "Cannot read header in algo-fpzip decompression: %s\n", fpzip_errstr[fpzip_errno]);
      return 1;
    }

    if (!fpzip_read(fpz, (void*)data_out)) {
      fprintf(stderr, "Algo-fpzip decompression failed: %s\n", fpzip_errstr[fpzip_errno]);
      return 1;
    }

    fpzip_read_close(fpz);

    return 0;
}
// End repeat

scilU_algorithm_t algo_fpzip = {
    .c.DNtype = {
        CREATE_INITIALIZER(scil_fpzip)
    },
    "fpzip",
    4,
    SCIL_COMPRESSOR_TYPE_DATATYPES
};

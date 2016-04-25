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
// This file contains an example for a preconditioner

#ifndef SCIL_ALGO_PRECOND_DUMMY_H
#define SCIL_ALGO_PRECOND_DUMMY_H

int scil_dummy_precond_compress_float(const scil_context_p ctx, float* restrict data_out, byte*restrict header, int * header_size_out, float*restrict data_in, const scil_dims* dims);
int scil_dummy_precond_decompress_float(float*restrict data_inout, scil_dims*const dims, float*restrict compressed_buf_in, byte*restrict header, int * header_parsed_out);
int scil_dummy_precond_compress_double(const scil_context_p ctx, double* restrict data_out, byte*restrict header, int * header_size_out, double*restrict data_in, const scil_dims* dims);
int scil_dummy_precond_decompress_double(double*restrict data_inout, scil_dims*const dims, double*restrict compressed_buf_in, byte*restrict header, int * header_parsed_out);

scil_compression_algorithm algo_precond_dummy = {
    .c.Ptype = {
        scil_dummy_precond_compress_float,
        scil_dummy_precond_decompress_float,
        scil_dummy_precond_compress_double,
        scil_dummy_precond_decompress_double,
    },
    "dummy-precond",
    8,
    SCIL_COMPRESSOR_TYPE_DATATYPES_PRECONDITIONER,
    0
};

#endif

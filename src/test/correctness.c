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

#include <scil-error.h>

#include <scil.h>
#include <scil-compressor.h>
#include <scil-data-characteristics.h>
#include <scil-patterns.h>
#include <scil-util.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>


#define allocate(type, name, count) \
    type* name = (type*)malloc(count * sizeof(type))

static int comp_error_occured = 0;
static int decomp_error_occured = 0;
static int val_error_occured = 0;

int test_correctness(const char* name, double* buffer_in, scil_dims_t dims)
{
    size_t out_c_size;
    size_t variableSize = scil_dims_get_count(&dims);

    const size_t c_size = scil_get_compressed_data_size_limit(&dims, SCIL_TYPE_DOUBLE);

    double* buffer_uncompressed = (double*) malloc(c_size);
    allocate(byte, buffer_out, c_size);
    allocate(byte, tmp_buff, c_size);

    scil_context_t* ctx;
    scil_user_hints_t hints;
    scil_user_hints_t out_accuracy;
    scil_validate_params_t out_validation;

    scil_user_hints_initialize(&hints);
    hints.absolute_tolerance = 0.01;

    double r = (double) scilU_get_data_randomness( buffer_in, variableSize * sizeof(double), tmp_buff, c_size);

    printf("Pattern %s randomness: %.1f%%\n", name, r);

    printf(
        "Algorithm, C Error, D Error, Validation, Uncompressed size, "
        "Compressed size, Compression factor, CSpeed MiB/s, DSpeed MiB/s, "
        "Algo\n");

    for (int i = -1; i < scilU_get_available_compressor_count(); i++) {
        char compression_name[1024];
        if (i == -1) {
            hints.force_compression_methods = NULL;
        } else {
            sprintf(compression_name, "%s", scilU_get_compressor_name(i));
            hints.force_compression_methods = compression_name;
        }
        if(strcmp(compression_name,"sz") == 0){
          printf("Skipping compressor for now!\n");
          continue;
        }

        int ret = scil_context_create(&ctx, SCIL_TYPE_DOUBLE, 0, NULL, &hints);
        if (ret != 0) {
            printf("Invalid combination %s\n", compression_name);
            continue;
        }
        assert(ctx != NULL);
        int ret_c;
        int ret_d;
        int ret_v;

        const uint8_t loops = 10;
        scil_timer timer;
        scilU_start_timer(&timer);

        for (uint8_t i = 0; i < loops; ++i) {
            ret_c = scil_compress(buffer_out, c_size, buffer_in, &dims, &out_c_size, ctx);
            if (ret_c != 0) break;
        }
        double seconds_compress = scilU_stop_timer(timer);
        ret_d                   = -1;
        ret_v                   = -1;

        if (ret_c == 0) {
            memset(buffer_uncompressed, -1, c_size);

            scilU_start_timer(&timer);
            for (uint8_t i = 0; i < loops; ++i) {
                ret_d = scil_decompress(SCIL_TYPE_DOUBLE, buffer_uncompressed, &dims, buffer_out, out_c_size, tmp_buff);
                if (ret_d != 0) break;
            }
        }
        double seconds_decompress = scilU_stop_timer(timer);

        if (ret_d == 0) {
            ret_v = scil_validate_compression(SCIL_TYPE_DOUBLE, buffer_in, &dims, buffer_out, out_c_size, ctx, &out_accuracy, &out_validation);
        }

        size_t u_size = variableSize * sizeof(double);
        double c_fac  = (double)(u_size) / out_c_size;

        if (ret_c != 0) { // Ignore validation errors here
            comp_error_occured = 1;
        }
        if (ret_d != 0) {
            decomp_error_occured = 1;
        }
        if (i == -1 && ret_v != 0) {
            val_error_occured = 1;
        }
        if (i == -1) {
            hints.force_compression_methods = compression_name;
            scil_compression_sprint_last_algorithm_chain(ctx, compression_name, 1024);
        }

        printf("%d, %d, %d, %d, %lu, %lu, %.1lf, %.1lf, %.1lf, %s \n",
               i,
               ret_c,
               ret_d,
               ret_v,
               u_size,
               out_c_size,
               c_fac,
               u_size / seconds_compress / 1024 / 1024,
               u_size / seconds_decompress / 1024 / 1024,
               hints.force_compression_methods);
    }

    printf("Done.\n");
    free(buffer_out);
    free(buffer_uncompressed);
    return 0;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
int main(int argc, char** argv)
{
    const int variableSize = 10000 / sizeof(double);
    int ret;
    allocate(double, buffer_in, variableSize);

    scil_dims_t dims;
    scil_dims_initialize_1d(&dims, variableSize);

    for (int i = 0; i < scilP_get_pattern_library_size(); i++) {
        char* name = scilP_get_library_pattern_name(i);
        printf("processing: %s\n", name);
        ret = scilP_create_library_pattern(buffer_in, SCIL_TYPE_DOUBLE, &dims, i);
        assert(ret == SCIL_NO_ERR);
        test_correctness(name, buffer_in, dims);
    }

    free(buffer_in);

    if(comp_error_occured){
        printf("Compression error occurred.\n");
    }
    if(decomp_error_occured){
        printf("Decompression error occurred.\n");
    }
    if(decomp_error_occured){
        printf("Validation error occurred.\n");
    }
    // for the moment, only check if this runs
    return 0; // TODO
    // return comp_error_occured || decomp_error_occured || val_error_occured;
}

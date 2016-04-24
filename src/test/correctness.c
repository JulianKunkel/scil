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
#include <stdio.h>
#include <string.h>

#include <scil-util.h>

#define allocate(type, name, count) type* name = (type*)malloc(count * sizeof(type))

static int error_occured = 0;

int test_correctness(double * buffer_in, const int variableSize){
	size_t out_c_size;
	// accept a suboptimal compression ratio of 0.5
	allocate(double, buffer_uncompressed, variableSize);

	scil_dims dims;
	scil_init_dims_1d(&dims, variableSize);

	const size_t c_size = scil_compress_buffer_size_bound(SCIL_TYPE_DOUBLE, &dims);

	allocate(byte, buffer_out, c_size);
	allocate(byte, tmp_buff, c_size);

  scil_context_p ctx;
  scil_hints hints;
	scil_hints out_accuracy;

  scil_init_hints(&hints);
	hints.absolute_tolerance = 0.01;

	printf("Algorithm, C Error, D Error, Validation, Uncompressed size, Compressed size, Compression factor, CSpeed MiB/s, DSpeed MiB/s, Algo\n");

	for(int i=-1; i < scil_compressors_available(); i++ ){
		char compression_name[1024];
		if (i == -1){
			hints.force_compression_methods = NULL;
		}else{
			sprintf(compression_name, "%s", scil_compressor_name(i));
			hints.force_compression_methods = compression_name;
		}

		int ret = scil_create_compression_context(& ctx, SCIL_TYPE_DOUBLE, &hints);
		if (ret != 0){
			printf("Invalid combination %s\n", compression_name);
			continue;
		}
		assert(ctx != NULL);
		int ret_c;
		int ret_d;
		int ret_v;

		const uint8_t loops = 10;
		scil_timer timer;
		scilU_start_timer(& timer);

		for(uint8_t i = 0; i < loops; ++i){
			ret_c = scil_compress(buffer_out, c_size, buffer_in, & dims,&out_c_size, ctx);
			if(ret_c != 0) break;
		}
		double seconds_compress = scilU_stop_timer(timer);
		ret_d = -1;
		ret_v = -1;

		if(ret_c == 0){
			scilU_start_timer(& timer);
			for(uint8_t i = 0; i < loops; ++i){
				ret_d = scil_decompress(SCIL_TYPE_DOUBLE, buffer_uncompressed, & dims, buffer_out, out_c_size, tmp_buff);
				if(ret_d != 0) break;
			}
		}
		double seconds_decompress = scilU_stop_timer(timer);

		if(ret_d == 0){
			ret_v = scil_validate_compression(SCIL_TYPE_DOUBLE, buffer_in, & dims, buffer_out, out_c_size, ctx, & out_accuracy);
		}

		size_t u_size = variableSize * sizeof(double);
		double c_fac = (double)(u_size) / out_c_size;

		if (ret_c != 0 || ret_d != 0 ){ // Ignore validation errors here
			error_occured = 1;
		}
		if( i == -1 && ret_v != 0){
			error_occured = 1;
		}
		if ( i == -1){
			hints.force_compression_methods = compression_name;
			scil_compression_sprint_last_algorithm_chain(ctx, compression_name, 1024);
		}

		printf("%d, %d, %d, %d, %lu, %lu, %.1lf, %.1lf, %.1lf, %s \n",
			i, ret_c, ret_d, ret_v,
			u_size, out_c_size, c_fac,
			u_size/seconds_compress/1024 /1024, u_size/seconds_decompress/1024 /1024, hints.force_compression_methods);
  }

	printf("Done.\n");
	free(buffer_out);
	free(buffer_uncompressed);
	return 0;
}

void test_pattern0(double * buffer_in, const int variableSize){
	printf("Pattern 0\n");
	for (int i=0; i < variableSize; i++){
		buffer_in[i] = 0;
	}
	test_correctness(buffer_in, variableSize);
}

void test_patternRND(double * buffer_in, const int variableSize){
	printf("Pattern RND\n");
	for (int i=0; i < variableSize; i++){
		buffer_in[i] = random() / ((double) RAND_MAX);
	}
	test_correctness(buffer_in, variableSize);
}

void test_patternALT(double * buffer_in, const int variableSize){
	printf("Pattern ALTERNATING 0-99\n");
	for (int i=0; i < variableSize; i++){
		buffer_in[i] = i % 100;
	}
	test_correctness(buffer_in, variableSize);
}

void test_patternALT2(double * buffer_in, const int variableSize){
	printf("Pattern ALTERNATING 0-1\n");
	for (int i=0; i < variableSize; i++){
		buffer_in[i] = i % 2;
	}
	test_correctness(buffer_in, variableSize);
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
int main(int argc, char** argv){
	const int variableSize = 1000000/sizeof(double);
	allocate(double, buffer_in, variableSize);

	test_pattern0(buffer_in, variableSize);
	test_patternRND(buffer_in, variableSize);
	test_patternALT(buffer_in, variableSize);
	test_patternALT2(buffer_in, variableSize);

	free(buffer_in);
	return error_occured;
}

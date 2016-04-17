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

#include <stdio.h>
#include <string.h>

#include <scil.h>

#define allocate(type, name, count) type* name = (type*)malloc(count * sizeof(type))

int test_correctness(double * buffer_in, const int variableSize){
	size_t out_c_size;
	// accept a suboptimal compression ratio of 0.5
	const size_t c_size = 2 * variableSize * sizeof(double) + SCIL_BLOCK_HEADER_MAX_SIZE;

	allocate(byte, buffer_out, c_size);
	allocate(byte, tmp_buff, c_size);
	allocate(double, buffer_uncompressed, variableSize);
	allocate(size_t, length, 1);
	length[0] = variableSize;

	scil_dims_t dims = scil_init_dims(1, length);

  struct scil_context_t* ctx;
  scil_hints hints;
	scil_hints out_accuracy;

  scil_init_hints(&hints);
	hints.absolute_tolerance = 0.01;

	printf("C Error, D Error, Validation, Uncompressed size, Compressed size, Compression factor, CSpeed MiB/s, DSpeed MiB/s, Algo\n");

	for(int i=0; i < 1; i++ ){ // scil_compressors_available()
		char compression_name[1024];
		sprintf(compression_name, "abstol,7", scil_compressor_name(i)); //%s
		hints.force_compression_methods = compression_name;

		int ret = scil_create_compression_context(&ctx, &hints);
		if (ret != 0){
			printf("Error creating the context\n");
			continue;
		}
		int ret_c;
		int ret_d;
		int ret_v;

		double seconds = 0;
		const uint8_t loops = 10;
		scil_timer timer;
		scilU_start_timer(& timer);

		for(uint8_t i = 0; i < loops; ++i){
			ret_c = scil_compress(SCIL_TYPE_DOUBLE, buffer_out, c_size, buffer_in, dims,&out_c_size, ctx);
			if(ret_c != 0) break;
		}
		double seconds_compress = scilU_stop_timer(timer);
		ret_d = -1;
		ret_v = -1;

		seconds = 0;
		if(ret_c == 0){
			scilU_start_timer(& timer);
			for(uint8_t i = 0; i < loops; ++i){
				ret_d = scil_decompress(SCIL_TYPE_DOUBLE, buffer_uncompressed, dims, buffer_out, out_c_size, tmp_buff);
				if(ret_d != 0) break;
			}
		}
		double seconds_decompress = scilU_stop_timer(timer);

		if(ret_d == 0){
			ret_v = scil_validate_compression(SCIL_TYPE_DOUBLE, buffer_in, dims, buffer_out, out_c_size, ctx, & out_accuracy);
		}

		size_t u_size = variableSize * sizeof(double);
		double c_fac = (double)(u_size) / out_c_size;

		printf("%d, %d, %d, %lu, %lu, %.1lf, %.1lf, %.1lf, %s \n",
			ret_c, ret_d, ret_v,
			u_size, out_c_size, c_fac,
			u_size/seconds_compress/1024 /1024, u_size/seconds_decompress/1024 /1024, hints.force_compression_methods );
  }

	printf("Done.\n");
 	free(length);
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
	printf("Pattern ALTERNATING\n");
	for (int i=0; i < variableSize; i++){
		buffer_in[i] = i % 100;
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

	free(buffer_in);
	return 0;
}

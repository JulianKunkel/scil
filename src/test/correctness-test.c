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
	const size_t c_size = (variableSize * sizeof(double)+SCIL_BLOCK_HEADER_MAX_SIZE);
	size_t out_c_size;

	allocate(byte, buffer_uncompressed, c_size);

	allocate(byte, buffer_out, c_size);
	allocate(size_t, length, 1);
	length[0] = variableSize;

	scil_dims_t dims = scil_init_dims(1, length);

  struct scil_context_t* ctx;
  scil_hints hints;

	scil_hints out_accuracy;

  scil_init_hints(&hints);
	printf("Algo, C Error, D Error, Validation, Uncompressed size, Compressed size, Compression factor, CSpeed MiB/s, DSpeed MiB/s\n");
	hints.force_compression_method = 0;

	while(hints.force_compression_method < scil_compressors_available()){
		scil_create_compression_context(&ctx, &hints);
		int ret_c;
		int ret_d;
		int ret_v;

		double seconds = 0;
		const uint8_t loops = 10;
		for(uint8_t i = 0; i < loops; ++i){
			clock_t start, end;
			start = clock();
			ret_c = scil_compress(SCIL_DOUBLE, buffer_out, c_size, buffer_in, dims,&out_c_size, ctx);
			end = clock();
			seconds += (double)(end - start);
		}
		double seconds_compress = seconds / (loops * CLOCKS_PER_SEC);

		seconds = 0;
		for(uint8_t i = 0; i < loops; ++i){
			clock_t start, end;
			start = clock();
			ret_d = scil_decompress(SCIL_DOUBLE, buffer_uncompressed, dims, buffer_out, out_c_size);
			end = clock();
			seconds += (double)(end - start);
		}
		double seconds_decompress = seconds / (loops * CLOCKS_PER_SEC);

		ret_v = scil_validate_compression(SCIL_DOUBLE, buffer_in, dims, buffer_out, out_c_size, ctx, & out_accuracy);


		size_t u_size = variableSize * sizeof(double);
		double c_fac = (double)(u_size) / out_c_size;

		printf("%d, %d, %d, %d, %lu, %lu, %.1lf, %.1lf, %.1lf \n", hints.force_compression_method, ret_c, ret_d, ret_v, u_size, out_c_size, c_fac, u_size/seconds_compress/1024 /1024, u_size/seconds_decompress/1024 /1024 );
		hints.force_compression_method++;
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


#pragma GCC diagnostic ignored "-Wunused-parameter"
int main(int argc, char** argv){
	const int variableSize = 1000000/sizeof(double);
	allocate(double, buffer_in, variableSize);

	test_pattern0(buffer_in, variableSize);

	test_patternRND(buffer_in, variableSize);

	free(buffer_in);
	return 0;
}

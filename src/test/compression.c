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
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <sys/stat.h>
#include <math.h>
#include <time.h>

#include <scil.h>
#include <scil-prepare.h>
#include <scil-user-hints.h>
#include <scil-util.h>

char* read_data(const char* path){

	assert(path != NULL);

	FILE* file = SAFE_FOPEN(path, "rb");

	fseek(file, 0, SEEK_END);
	size_t size = (size_t)ftell(file);
	rewind(file);

	char* buf = (char*)SAFE_MALLOC(size+1);

	size_t result = fread(buf, 1, size, file);
	if(result != size){
		fprintf(stderr, "Reading error of file %s\n", path);
		exit(EXIT_FAILURE);
	}

	fclose(file);

	return buf;
}

void write_data(void* buf, size_t num, uint8_t size, char* path){

	assert(buf != NULL);
	assert(num != 0);
	assert(path != NULL);

	FILE* file = SAFE_FOPEN(path, "wb");
	fwrite(buf, size, num, file);
	fclose(file);
}

void print_bits_uint64(uint64_t a){

	uint8_t bits = 8 * sizeof(uint64_t);

	for(uint8_t i = (uint8_t)(bits - 1); i < bits; --i){
		printf("%lu", (a >> i) % 2);
	}
	printf("\n");

}

void print_bits_uint8(uint8_t a){

	uint8_t bits = 8 * sizeof(uint8_t);

	for(uint8_t i = (uint8_t)(bits - 1); i < bits; --i){
		printf("%d", (a >> i) % 2);
	}
	printf("\n");

}

int main(){
	scil_context_p ctx;
	scilPr_user_hints_t hints;
	int ret;

	scilPr_initialize_user_hints(& hints);
	hints.force_compression_methods = "1";
	hints.absolute_tolerance = 0.5;
	//hints.relative_tolerance_percent = 1.0;
	hints.significant_bits = 5;
	scil_create_compression_context(&ctx, SCIL_TYPE_DOUBLE, 0, NULL, &hints);

	const size_t count = 100;
	size_t u_buf_size = count * sizeof(double);

	double * u_buf = (double *)SAFE_MALLOC(u_buf_size);
	printf("U ");
	for(size_t i = 0; i < count; ++i)
	{
		u_buf[i] = (double)(i % 10-5.1);
		printf("%f ", u_buf[i]);
	}
	printf("\n\n");

	printf("U size: %lu\n", u_buf_size);

	size_t c_buf_size = u_buf_size + SCIL_BLOCK_HEADER_MAX_SIZE;
	byte * c_buf = (byte*)SAFE_MALLOC(c_buf_size*4);

	scil_dims dims;
	scil_init_dims_1d(& dims, count);

	ret = scil_compress(c_buf, c_buf_size, u_buf, & dims, &c_buf_size, ctx);

	printf("C size: %lu\n", c_buf_size);

	double * data_out = (double*)SAFE_MALLOC(u_buf_size);
	ret = scil_decompress(SCIL_TYPE_DOUBLE, data_out, & dims, c_buf, c_buf_size, & c_buf[c_buf_size*2]);

	printf("Decompression %d\n", ret);

	printf("\nD ");
	for(size_t i = 0; i < count; ++i)
	{
		printf("%f ", data_out[i]);
	}
	printf("\n");
	scilPr_user_hints_t accuracy;

	printf("Testing accuracy first\n");

	double f1 = 10.0;
	double f2 = 10.5;

	scil_dims dims1;
	scil_init_dims_1d(&dims1, 1);

	scil_determine_accuracy(SCIL_TYPE_DOUBLE, & f1, &f2, & dims1, 0.01, & accuracy);
	scilPr_print_user_hints(& accuracy);

	scil_determine_accuracy(SCIL_TYPE_DOUBLE, & f1, &f2, & dims1, 0.51, & accuracy);
	scilPr_print_user_hints(& accuracy);

	ret = scil_validate_compression(SCIL_TYPE_DOUBLE, u_buf, & dims, c_buf, c_buf_size, ctx, & accuracy);

	printf("\nscil_validate_compression returned %s\n", ret == 0 ? "OK" : "ERROR");
	scilPr_print_user_hints(& accuracy);

	free(c_buf);
	free(data_out);
	free(u_buf);
	free(ctx);

	return 0;
}

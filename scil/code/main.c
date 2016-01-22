#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <sys/stat.h>
#include <math.h>
#include <time.h>

#include <scil.h>
#include <util.h>

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

void print_comp_buf(const uint8_t * restrict buf, const size_t buf_size, uint8_t bits_per_num){


}

int main(){

	scil_context * ctx;
	scil_hints hints;
	hints.force_compression_method = 1;
	hints.absolute_tolerance = 0.5f;
	scil_create_compression_context(&ctx, &hints);

	size_t count = 10;
	size_t u_buf_size = count * sizeof(double);

	double * u_buf = (double *)SAFE_MALLOC(u_buf_size);
	printf("U ");
	for(size_t i = 0; i < count; ++i)
	{
		u_buf[i] = (double)i;
		printf("%f ", u_buf[i]);
	}
	printf("\n\n");

	size_t c_buf_size;
	char * c_buf = (char*)SAFE_MALLOC(u_buf_size+1);
	scil_compress(ctx, c_buf, &c_buf_size, u_buf, count);

	for (size_t i = 0; i < c_buf_size; i++) {
		print_bits_uint8(c_buf[i]);
	}
	printf("\n");

	double * data_out = (double*)SAFE_MALLOC(u_buf_size + SCIL_BLOCK_HEADER_MAX_SIZE);
	size_t outsize;
	scil_decompress(data_out, & outsize, c_buf, c_buf_size);

	printf("\nD ");
	for(size_t i = 0; i < count; ++i)
	{
		printf("%f ", data_out[i]);
	}
	printf("\n");

	free(c_buf);
	free(data_out);
	free(u_buf);
	free(ctx);
	/*
	size_t count = 1000;

	char* u_path = "uncomp_1000_d.data";

	char* c_path = "comp_1000_d.data";

	double* buf = (double*)read_data(u_path);
	int ret;

	scil_context * ctx;
	scil_hints hints;
	hints.absolute_tolerance = 1.0;

	ret = scil_create_compression_context(& ctx, & hints);

	char* c_buf = NULL;
	size_t c_size;
	scil_compress(ctx, &c_buf, &c_size, buf, count);

	ret = scil_validate_compression(ctx, count, buf, c_size, c_buf);

	free(buf);
	free(ctx);
	free(c_buf);

	if (ret != 0){
		printf("Error in the validation!\n");
		return 1;
	}

	write_data(c_buf, c_size, sizeof(char), c_path);


	double* buf = (double*)SAFE_MALLOC(count * sizeof(double));

	for(size_t i = 0; i < count; ++i){
		buf[i] = (double)(i % 10);
	}

	write_data(buf, count, sizeof(double), u_path);
	*/

	return 0;
}

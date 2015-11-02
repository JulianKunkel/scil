#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <sys/stat.h>
#include <math.h>

#include "scil.h"
#include "util.h"

double* read_data(const char* path){
	
	assert(path != NULL);

	FILE* file = SAFE_FOPEN(path, "rb");

	fseek(file, 0, SEEK_END);
	size_t size = (size_t)ftell(file);
	rewind(file);

	double* buf = (double*)SAFE_MALLOC(size);

	size_t result = fread(buf, sizeof(double), size, file);
	if(result != size){
		fprintf(stderr, "Reading error of file %s", path);
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

void printb_uint64(uint64_t a){

	uint8_t bits = 8 * sizeof(uint64_t);

	for(uint8_t i = bits - 1; i < bits; --i){
		printf("%d", (a >> i) % 2);
	}
	printf("\n");

}

void printb_uint8(uint8_t a){

	uint8_t bits = 8 * sizeof(uint8_t);

	for(uint8_t i = bits - 1; i < bits; --i){
		printf("%d", (a >> i) % 2);
	}
	printf("\n");

}

int main(int argc, char** argv){

	
	size_t num = 10;

	double* buf = (double*)SAFE_MALLOC(num * sizeof(double));

	printf("Before Comp-Decomp\n");
	for(size_t i = 0; i < num; i++){
		buf[i] = 1 - i % 2;
		printf("%f\n", buf[i]);
	}
	printf("\n");

	cdata* comp = compress(buf, num, 0, 1.0, 1.0);

	buf = decompress(comp, num, 0, 1.0, 1.0);

	printf("\n");

	printf("After Comp-Decomp\n");
	for(size_t i = 0; i < num; i++){
		printf("%f\n", buf[i]);
	}

	free(comp->buffer);
	free(comp);
	

	return 0;
}
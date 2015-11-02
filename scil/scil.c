#include "scil.h"

#include <stdint.h>
#include <assert.h>
#include <math.h>

static double find_min(const double* buf, const size_t size){

	assert(buf != NULL);

	double result = INFINITY;
	for(size_t i = 0; i < size; ++i)
	{
		if(buf[i] < result)
			result = buf[i];
	}
	return result;
}

static double find_max(const double* buf, const size_t size){

	assert(buf != NULL);

	double result = -INFINITY;
	for(size_t i = 0; i < size; ++i)
	{
		if(buf[i] > result)
			result = buf[i];
	}
	return result;
}

static int find_min_max(double* min, double* max, const double* buf, const size_t size){

	assert(buf != NULL);

	*min = INFINITY;
	*max = -INFINITY;

	for(size_t i = 0; i < size; ++i)
	{
		if(buf[i] < *min)
			*min = buf[i];
		if(buf[i] > *max)
			*max = buf[i];
	}

	return 0;
}

static uint8_t get_needed_bit_count(const double min_value, const double max_value, const double absolute_tolerance){

	assert(max_value > min_value);
	assert(error_step > 0);

	return (uint8_t)ceil(log2(1 + (max_value - min_value) / (2 * absolute_tolerance))); //TODO: Test this
}

static size_t round_up_byte(const size_t bits){
	
	uint8_t a = bits % 8;
	if(a == 0)
		return bits / 8;
	return 1 + (bits - a) / 8;
}

static uint64_t int_repres(const double num, const double min, const double absolute_tolerance){

	assert(num >= min);

	return (uint64_t)round((num - min) / (2 * absolute_tolerance)); //TODO: Test this
}

static double double_repres(const uint64_t num, const double min, const double absolute_tolerance){

	return min + num * 2 * absolute_tolerance; //TODO: Test this
}

int scil_create_compression_context(scil_context * out_ctx, scil_hints * hints){

	out_ctx->hints = hints;

	return 0;
}

int scil_compress(scil_context* ctx, char* compressed_buf_out, size_t* out_size, const double* data_in, const size_t in_size);

	assert(ctx != NULL);
	assert(data_in != NULL);

	//Finding minimum and maximum values in data
	double min, max;
	find_min_max(&min, &max, data_in, in_size);

	//Get needed bits per number in data
	uint8_t bits = get_needed_bit_count(min, max, ctx->hints->absolute_tolerance);

	return 0;
}

int scil_decompress(scil_context* ctx, double* data_out, size_t* out_size, const char* compressed_buf_in, const size_t in_size);
	
	assert(ctx != NULL);
	assert(compressed_buf_in != NULL);

	return 0;
}

cdata* compress(double* buf, size_t size, double min_value, double max_value, double error_step){

	cdata* result = (cdata*)SAFE_MALLOC(sizeof(cdata));

	uint8_t bits = get_needed_bit_count(min_value, max_value, error_step);
	result->bytes_num = round_up_byte(bits * size);

	//printf("\nBit Size:\t\t%d Bit\n", bits);
	//printf("Buffer Size:\t\t%d Byte\n", size * sizeof(double));
	//rintf("Compressed Buffer Size:\t%d Byte\n\n", result->size);

	result->buffer = (uint8_t*)SAFE_CALLOC(result->bytes_num, 1);

	size_t from_i = 0;
	size_t to_i = 0;

	int8_t right_shifts = 0;

	uint8_t from_filled = 1;
	uint8_t to_filled = 1;

	while(from_i < size){

		right_shifts += from_filled * bits;
		right_shifts -= to_filled * 8;

		uint64_t integ = int_repres(buf[from_i], min_value, error_step);

		result->buffer[to_i] |= right_shifts < 0 ? (integ << -right_shifts) : (integ >> right_shifts); //Hier liegt das Problem!!

		//printb_uint8(result->buffer[to_i]);

		from_filled = right_shifts <= 0;
		to_filled = right_shifts >= 0;

		from_i += from_filled;
		to_i += to_filled;

	}

	return result;
}

double* decompress(cdata* data, size_t size, double min_value, double max_value, double error_step){

	double* result = (double*)SAFE_MALLOC(size * sizeof(double));

	uint8_t bits = get_needed_bit_count(min_value, max_value, error_step);

	size_t from_i = 0;
	size_t to_i = 0;

	int8_t right_shifts = 0;

	uint8_t from_filled = 1;
	uint8_t to_filled = 1;

	uint64_t current = 0;

	while(to_i < size){
		
		right_shifts -= to_filled * bits;
		right_shifts += from_filled * 8;

		printf("%d\n", right_shifts);
		printb_uint8(data->buffer[from_i]);
		
		current *= (1 - to_filled);
		current |= right_shifts < 0 ? (data->buffer[from_i] << -right_shifts) : (data->buffer[from_i] >> right_shifts);
		
		printb_uint8(current);
		printf("\n");

		result[to_i] = double_repres(current, min_value, error_step);

		from_filled = right_shifts <= 0;
		to_filled = right_shifts >= 0;

		from_i += from_filled;
		to_i += to_filled;
	}

	return result;
}


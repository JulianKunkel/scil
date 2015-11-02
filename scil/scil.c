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

static int find_min_max(const double* min, const double* max, const double* buf, const size_t size){

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

int scil_compress(scil_context* ctx, char* compressed_buf_out, const size_t* out_size, const double* data_in, const size_t in_size);

	assert(ctx != NULL);
	assert(data_in != NULL);

	//Finding minimum and maximum values in data
	double min, max;
	find_min_max(&min, &max, data_in, in_size);

	//Locally assigning absolute tolerance
	double abs_tol = ctx->hints->absolute_tolerance;

	//Get needed bits per compressed number in data
	uint8_t bits_per_num = get_needed_bit_count(min, max, abs_tol);

	//Get number of needed bytes for the whole compressed buffer
	*out_size = round_up_byte(bits * size);

	//Initialize every bit in output buffer to 0
	compressed_buf_out = (char*)SAFE_CALLOC(*out_size, sizeof(char));

	//Input and output buffer indices
	size_t from_i = 0;
	size_t to_i = 0;

	//Needed shifts (also left as negative right) for bit perfect packing
	int8_t right_shifts = 0;

	//Helper booleans to determine if a number form input buffer is done packing
	//and if a byte in outbut buffer is full
	uint8_t from_filled = 1;
	uint8_t to_filled = 1;

	//As long as there are numbers not done packing in input buffer, do
	while(from_i < size){

		//If last number is done packing, increment right_shifts by bits per compressed number
		right_shifts += from_filled * bits;
		//If last target byte was filled, decrement right_shifts by one byte
		right_shifts -= to_filled * 8;

		//Get integer representation of a compressed number
		uint64_t integ = int_repres(data_in[from_i], min, abs_tol);

		//Set the current bytes bit to current bits of integ
		compressed_buf_out[to_i] |= right_shifts < 0 ? (integ << -right_shifts) : (integ >> right_shifts);

		//If right_shifts were smaller or equal 0, the current compressed number is done packing
		from_filled = right_shifts <= 0;
		//If it was greater or equal 0, the current byte is filled
		to_filled = right_shifts >= 0;

		//Increment the index of the data buffer if the current number is done packing
		from_i += from_filled;
		//Increment the target byte index if the current byte was filled
		to_i += to_filled;
	}

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


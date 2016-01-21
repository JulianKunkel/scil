#include <math.h>

#include <algo/algo-1.h>
#include <util.h>

static int find_min_max(double*restrict min, double*restrict max, const double* buf, const size_t size){

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
    assert(absolute_tolerance > 0);

    return (uint8_t)ceil(log2(1 + (max_value - min_value) / (2 * absolute_tolerance))); //TODO: Test this
}

static size_t round_up_byte(const size_t bits){

    uint8_t a = bits % 8;
    if(a == 0)
        return bits / 8;
    return 1 + (bits - a) / 8;
}

static uint64_t int_repres(const double num, const double min, const double absolute_tolerance){

    if(num < min){
      printf("%f\n", num);
    }
    assert(num >= min);

    return (uint64_t)round((num - min) / (2 * absolute_tolerance));
}

static double double_repres(const uint64_t num, const double min, const double absolute_tolerance){

    return min + (double)num * 2 * absolute_tolerance;
}

int scil_algo1_compress(const scil_context* ctx,
                        char* restrict compressed_buf_out,
                        size_t* restrict out_size,
                        const double*restrict data_in,
                        const size_t in_size)
{
    //Finding minimum and maximum values in data
    double min, max;
    find_min_max(&min, &max, data_in, in_size);

    //Locally assigning absolute tolerance
    double abs_tol = ctx->hints.absolute_tolerance;

    //Get needed bits per compressed number in data
    uint8_t bits_per_num = get_needed_bit_count(min, max, abs_tol);

    //Header byte size (currently sizes of magic number + min + abs_tol)
    uint8_t head_size = 0;

    //Set compression information
    //Minimum value
    *((double*)(compressed_buf_out)) = min;
    compressed_buf_out+= 8;
    head_size += 8;

    *((double*)(compressed_buf_out)) = abs_tol;
    compressed_buf_out+= 8;
    head_size += 8;

    *compressed_buf_out = bits_per_num;
    compressed_buf_out++;
    head_size += 1;


    //Get number of needed bytes for the whole compressed buffer
    *out_size = round_up_byte(bits_per_num * in_size) + head_size;

    //printf("C %d %f %f\n", bits_per_num, min, abs_tol);

    //Input and output buffer indices
    size_t from_i = 0;
    size_t to_i = 0;

    //Needed shifts (also left as negative right) for bit perfect packing
    int right_shifts = 0;

    //Helper booleans to determine if a number form input buffer is done packing
    //and if a byte in outbut buffer is full
    bool from_filled = true;
    bool to_filled = true;

    //As long as there are numbers not done packing in input buffer, do
    while(from_i < in_size){

        //If last number is done packing, increment right_shifts by bits per compressed number
        right_shifts += (int)(from_filled * bits_per_num);
        //If last target byte was filled, decrement right_shifts by one byte
        right_shifts -= to_filled * 8;

        //Get integer representation of a compressed number
        uint64_t integ = int_repres(data_in[from_i], min, abs_tol);
        printf(" %d", integ);

        //Set the current bytes bit to current bits of integ
        compressed_buf_out[to_i] = compressed_buf_out[to_i] | ((char)(right_shifts < 0 ? integ << -right_shifts : integ >> right_shifts));

        //If right_shifts were smaller or equal 0, the current compressed number is done packing
        from_filled = right_shifts <= 0;
        //If it was greater or equal 0, the current byte is filled
        to_filled = right_shifts >= 0;

        //Increment the index of the data buffer if the current number is done packing
        from_i += from_filled;
        //Increment the target byte index if the current byte was filled
        to_i += to_filled;
    }

    printf("\n");
    return 0;
}

int scil_algo1_decompress(  const scil_context* ctx,
                            double*restrict data_out,
                            size_t*restrict out_size,
                            const char*restrict compressed_buf_in,
                            const size_t min_size)
{
    assert(compressed_buf_in != NULL);

    uint8_t bits_per_num;
    double min, abs_tol;

    size_t in_size = min_size - 17;

    // parse Header

    min = *((double*)(compressed_buf_in));
    compressed_buf_in+= 8;

    abs_tol = *((double*)(compressed_buf_in));
    compressed_buf_in+= 8;

    bits_per_num = *compressed_buf_in;
    compressed_buf_in++;

    //printf("D %d %f %f\n", bits_per_num, min, abs_tol);    
    int remaining_bits = 0;

    uint8_t start_mask[8] = {255,127,63,31,15,7,3,1,0};
    uint8_t end_mask[8] = {254,255-3,255-7,255-15,255-31,255-63,255-127,0};

    printf("DECOMPRESS %d %lld\n", bits_per_num, (long long int) in_size);

    for(int i=0; i < in_size*8; i+= bits_per_num){
        int spos = i / 8;
        int epos = (i+bits_per_num) / 8;

        // start may only keep last few bits
        uint64_t value = start_mask[i % 8] & compressed_buf_in[spos];

        if (spos != epos){
            uint64_t remainder = 0;

            for (int byte = spos ; byte < epos; byte ++){
                remainder = remainder | compressed_buf_in[byte];
                remainder = remainder << 8;
            }
            uint8_t last_bits = (i+bits_per_num)%8;
            value = value & (remainder >> (8 - last_bits));

            // read last few bits
            value = compressed_buf_in[epos] & end_mask[ last_bits ];
        }else{
            uint8_t last_bits = (i+bits_per_num)%8;
            value = value >> (8 - last_bits);
        }
        printf(" %d\n", value);
    }

    return 0;
}

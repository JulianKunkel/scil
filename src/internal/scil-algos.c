#include <scil-algos.h>

#include <assert.h>
#include <ctype.h>
#include <string.h>

scil_compression_algorithm_t* algo_array[] = {
	& algo_memcopy,
	& algo_abstol,
	& algo_gzip,
	& algo_sigbits,
	& algo_fpzip,
	& algo_zfp_abstol,
	& algo_zfp_precision,
	& algo_lz4fast,
	& algo_precond_dummy,
	& algo_quantize,
	& algo_swage,
	& algo_wavelets,
	NULL
};

int scil_compressors_available(){
	static int count = -1;
	if (count > 0){
		return count;
	}

	scil_compression_algorithm_t ** cur = algo_array;
	count = 0;
	// count
	while(*cur != NULL){
		count++;
		cur++;
	}

	return count;
}

const char* scil_compressor_name(int num)
{
    assert(num < scil_compressors_available());
    return algo_array[num]->name;
}

int scil_compressor_num_by_name(const char* name)
{
    scil_compression_algorithm_t** cur = algo_array;
    int count                        = 0;

    // check if this is a number
    int num = 1;
    for (unsigned i = 0; i < strlen(name); i++) {
        if (!isdigit(name[i])) {
            num = 0;
            break;
        }
    }
    if (num) {
        return atoi(name);
    }

    // count
    while (*cur != NULL) {
        if (strcmp((*cur)->name, name) == 0) {
            return count;
        }
        count++;
        cur++;
    }

    return -1;
}

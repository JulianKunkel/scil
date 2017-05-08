#include <scil-compressors.h>

#include <assert.h>
#include <ctype.h>
#include <string.h>

scilI_algorithm_t* algo_array[] = {
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
	& algo_allquant,
	& algo_sz, // 13
	& algo_precond_delta, // 14
	& algo_precond_fp_delta, // 15
	NULL
};

int scilU_get_available_compressor_count()
{
	static int count = -1;
	if (count > 0){
		return count;
	}

	scilI_algorithm_t ** cur = algo_array;
	count = 0;
	// count
	while(*cur != NULL){
		count++;
		cur++;
	}

	return count;
}

const char* scilU_get_compressor_name(int number)
{
    assert(number < scilU_get_available_compressor_count());
    return algo_array[number]->name;
}

int scilU_get_compressor_number(const char* name)
{
    scilI_algorithm_t** cur = algo_array;
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

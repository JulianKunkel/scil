#include <scil-compressor.h>

// known algorithms:
#include <algo/algo-abstol.h>
#include <algo/algo-fpzip.h>
#include <algo/algo-gzip.h>
#include <algo/algo-memcopy.h>
#include <algo/algo-sigbits.h>
#include <algo/algo-zfp-abstol.h>
#include <algo/algo-zfp-precision.h>
#include <algo/lz4fast.h>
#include <algo/zstd.h>
#include <algo/zstd-11.h>
#include <algo/zstd-22.h>
#include <algo/precond-dummy.h>
#include <algo/algo-quantize.h>
#include <algo/algo-swage.h>
//#include <algo/algo-wavelets.h>
#include <algo/algo-allquant.h>
#include <algo/algo-sz.h>
#include <algo/precond-delta.h>
#include <algo/precond-fp-delta.h>

#include <scil-debug.h>

#include <assert.h>
#include <ctype.h>
#include <string.h>


scilU_algorithm_t algo_wavelets = {// unimplemented at the moment
    .name = "wavelets",
    11,
    SCIL_COMPRESSOR_TYPE_DATATYPES,
    1
};

static scilU_algorithm_t* algo_array[] = {
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
  	& algo_zstd,
  	& algo_zstd11,
  	& algo_zstd22,
	NULL
};

void scil_initialize_compressors(){
    // verify correctness of algo_array
    int i = 0;
    for (scilU_algorithm_t **algo = algo_array; *algo != NULL;
         algo++, i++) {
        if ((*algo)->compressor_id != i) {
					printf("id_%i i=%i",(*algo)->compressor_id,i);
            critical("compressor ID does not match!");
        }
        if ((*algo)->type == SCIL_COMPRESSOR_TYPE_INDIVIDUAL_BYTES) {
            // we expect that all byte compressors are lossless
            (*algo)->is_lossy = 0;
        }
    }
}

scilU_algorithm_t* scil_get_compressor(int number){
	return algo_array[number];
}


int scilU_get_available_compressor_count()
{
	static int count = -1;
	if (count > 0){
		return count;
	}

	scilU_algorithm_t ** cur = algo_array;
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
    scilU_algorithm_t** cur = algo_array;
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


scilU_algorithm_t* scilU_find_compressor_by_name(const char* name)
{
    int num = scilU_get_compressor_number(name);
    if (num < 0 || num >= scilU_get_available_compressor_count()) {
        return NULL;
    } else {
        return algo_array[num];
    }
}

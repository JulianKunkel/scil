/*
 * This test is based on the test nc_test4/tst_chunks3.c from NetCDF under the license in COPYRIGHT-netcdf.txt
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#ifdef HAVE_SYS_TIMES_H
#  include <sys/times.h>
#endif
#include <sys/stat.h>
#include <assert.h>
#include <sys/time.h>
#include <unistd.h>		/* for sysconf */
#ifdef HAVE_SYS_RESOURCE_H
#  include <sys/resource.h>
#endif
#include <netcdf.h>

#include <scil.h>

#define FILENAME "tst_chunks3.nc"

/* This macro prints an error message with line number and name of
 * test program. */
#define ERR1(n) do {						  \
fflush(stdout); /* Make sure our stdout is synced with stderr. */ \
fprintf(stderr, "Sorry! Unexpected result, %s, line: %d - %s\n", \
	__FILE__, __LINE__, nc_strerror(n));			 \
return n; \
} while (0)


void
parse_args(int argc, char *argv[], /* from command-line invocation */
	   int *shufflep,	   /* returned: 1 if shuffle, otherwise 0 */
	   size_t *dims,	   /* returned: dimension sizes */
	   size_t *chunks,	   /* returned: chunk sizes */
	   size_t *cache_sizep,	   /* returned: cache size (bytes) */
	   size_t *cache_nelemsp,  /* returned: cache capacity (chunks) */
	   float *cache_prep)	   /* returned: cache preemption policy (0-1) */
{

    if(argc > 1)
	dims[0] = atol(argv[1]);
    if(argc > 2)
	chunks[0] = atol(argv[2]);
    else
	chunks[0] = (dims[0]+7)/8;
    if(argc > 3)
	dims[1] = atol(argv[3]);
    else
	dims[1] = dims[0];
    if(argc > 4)
	chunks[1] = atol(argv[4]);
    else
	chunks[1] = chunks[0];
    if(argc > 5)
	dims[2] = atol(argv[5]);
    else
	dims[2] = dims[1];
    if(argc > 6)
	chunks[2] = atol(argv[6]);
    else
	chunks[2] = chunks[1];
    if(argc > 7)
	*cache_sizep = atol(argv[7]);
    if(argc > 8)
	*cache_nelemsp = atol(argv[8]);
    if(argc > 9)
	*cache_prep = atof(argv[9]);
    if(argc > 10) {
	printf("Usage: %s [dim1] [chunk1] [dim2] [chunk2] [dim3] [chunk3] [cache_size] [cache_nelems] [cache_pre]\n",
	       argv[0]);
	exit(1);
    }
    return;
}

void *
emalloc(size_t bytes) {
    size_t *memory;
    memory = malloc(bytes);
    if(memory == 0) {
	printf("malloc failed\n");
	exit(2);
    }
    return memory;
}


/* compare contiguous, chunked, and compressed performance */
int
main(int argc, char *argv[]) {

    int  stat;  /* return status */
    int  ncid;  /* netCDF id */
    int i, j, k;
    int dim1id, dim2id, dim3id;
    int varid_g;		  /* varid for contiguous */
    int varid_k;		  /* varid for chunked */
    int varid_x;		  /* varid for compressed */
		scil_user_hints_t scil_hints;
		scil_user_hints_initialize(& scil_hints);

    float *varxy, *varxz, *varyz;    /* 2D memory slabs used for I/O */
    int mm;
    size_t dims[] = {32, 32, 32}; /* default dim lengths */
    size_t chunks[] = {32, 32, 32}; /* default chunk sizes */
    size_t start[3], count[3];
    float contig_time, chunked_time, compressed_time, ratio;
    int shuffle = NC_NOSHUFFLE;
    size_t cache_size_def;
    size_t cache_hash_def;
    float cache_pre_def;
    size_t cache_size = 0;	    /* use library default */
    size_t cache_hash = 0;	    /* use library default */
    float cache_pre = -1.0f;	    /* use library default */

    /* rank (number of dimensions) for each variable */
#   define RANK_var1 3

    /* variable shapes */
    int var_dims[RANK_var1];

    /* From args, get parameters for timing, including variable and
       chunk sizes.  Negative deflate level means also use shuffle
       filter. */
    parse_args(argc, argv, &shuffle, dims,
	       chunks, &cache_size, &cache_hash, &cache_pre);

    /* get cache defaults, then set cache parameters that are not default */
    if((stat = nc_get_chunk_cache(&cache_size_def, &cache_hash_def,
				   &cache_pre_def)))
	ERR1(stat);
    if(cache_size == 0)
	cache_size = cache_size_def;
    if(cache_hash == 0)
	cache_hash = cache_hash_def;
    if(cache_pre == -1.0f)
	cache_pre = cache_pre_def;
    if((stat = nc_set_chunk_cache(cache_size, cache_hash, cache_pre)))
	ERR1(stat);
    printf("cache: %3.2f MBytes  %ld objs  %3.2f preempt, ",
	   cache_size/1.e6, cache_hash, cache_pre);

    if(shuffle == 1) {
	printf(", shuffled");
    }
    printf("\n\n");

    /* initialize 2D slabs for writing along each axis with phony data */
    varyz = (float *) emalloc(sizeof(float) * 1 * dims[1] * dims[2]);
    varxz = (float *) emalloc(sizeof(float) * dims[0] * 1 * dims[2]);
    varxy = (float *) emalloc(sizeof(float) * dims[0] * dims[1] * 1);
    mm = 0;
    for(j = 0; j < dims[1]; j++) {
	for(k = 0; k < dims[2]; k++) {
	    varyz[mm++] = k + dims[2]*j;
	}
    }
    mm = 0;
    for(i = 0; i < dims[0]; i++) {
	for(k = 0; k < dims[2]; k++) {
	    varxz[mm++] = k + dims[2]*i;
	}
    }
    mm = 0;
    for(i = 0; i < dims[0]; i++) {
	for(j = 0; j < dims[1]; j++) {
	    varxy[mm++] = j + dims[1]*i;
	}
    }

    if((stat = nc_create(FILENAME, NC_NETCDF4 | NC_CLASSIC_MODEL, &ncid)))
	ERR1(stat);

    /* define dimensions */
    if((stat = nc_def_dim(ncid, "dim1", dims[0], &dim1id)))
	ERR1(stat);
    if((stat = nc_def_dim(ncid, "dim2", dims[1], &dim2id)))
	ERR1(stat);
    if((stat = nc_def_dim(ncid, "dim3", dims[2], &dim3id)))
	ERR1(stat);

    /* define variables */
    var_dims[0] = dim1id;
    var_dims[1] = dim2id;
    var_dims[2] = dim3id;
    if((stat = nc_def_var(ncid, "var_contiguous", NC_FLOAT, RANK_var1,
			   var_dims, &varid_g)))
	ERR1(stat);
    if((stat = nc_def_var(ncid, "var_chunked", NC_FLOAT, RANK_var1,
			   var_dims, &varid_k)))
	ERR1(stat);
    if((stat = nc_def_var(ncid, "var_compressed", NC_FLOAT, RANK_var1,
			   var_dims, &varid_x)))
	ERR1(stat);

    if((stat = nc_def_var_chunking(ncid, varid_g, NC_CONTIGUOUS, 0)))
	ERR1(stat);

    if((stat = nc_def_var_chunking(ncid, varid_k, NC_CHUNKED, chunks)))
	ERR1(stat);

    if((stat = nc_def_var_chunking(ncid, varid_x, NC_CHUNKED, chunks)))
	ERR1(stat);

	if((stat = nc_def_var_fill(ncid, varid_x, 0, 0)))
		ERR1(stat);
  if((stat = nc_def_var_scil(ncid, varid_x, & scil_hints)))
  	ERR1(stat);
	if((stat = nc_def_var_fill(ncid, varid_k, 0, 0)))
		ERR1(stat);
    /* leave define mode */
  if((stat = nc_enddef (ncid)))
		ERR1(stat);

    /* write each variable one yz slab at a time */
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = 1;
    count[1] = dims[1];
    count[2] = dims[2];
    for(i = 0; i < dims[0]; i++) {
	start[0] = i;
	if((stat = nc_put_vara(ncid, varid_g, start, count, &varyz[0])))
	    ERR1(stat);
    }
    for(i = 0; i < dims[0]; i++) {
	start[0] = i;
	if((stat = nc_put_vara(ncid, varid_k, start, count, &varyz[0])))
	    ERR1(stat);
    }
    for(i = 0; i < dims[0]; i++) {
	start[0] = i;
	if((stat = nc_put_vara(ncid, varid_x, start, count, &varyz[0])))
	    ERR1(stat);
    }

    /* write each variable one xz slab at a time */
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    count[0] = dims[0];
    count[1] = 1;
    count[2] = dims[2];

    for(i = 0; i < dims[1]; i++) {
	start[1] = i;
	if((stat = nc_put_vara(ncid, varid_g, start, count, &varxz[0])))
	    ERR1(stat);
    }
    for(i = 0; i < dims[1]; i++) {
	start[1] = i;
	if((stat = nc_put_vara(ncid, varid_k, start, count, &varxz[0])))
	    ERR1(stat);
    }
    for(i = 0; i < dims[1]; i++) {
	start[1] = i;
	if((stat = nc_put_vara(ncid, varid_x, start, count, &varxz[0])))
	    ERR1(stat);
    }

    if((stat = nc_close(ncid)))
	ERR1(stat);

    return 0;
}

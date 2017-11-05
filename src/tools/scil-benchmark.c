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

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <scil.h>
#include <scil-compressor.h>
#include <scil-data-characteristics.h>
#include <scil-error.h>
#include <scil-debug.h>
#include <scil-patterns.h>
#include <scil-util.h>

#define allocate(type, name, count) type* name = (type*)malloc(count * sizeof(type))

static int error_occured = 0;
static double * buffer_uncompressed;

void benchmark(FILE * f, SCIL_Datatype_t datatype, const char * name, byte * buffer_in, scil_dims_t dims){
	size_t out_c_size;

	const size_t buff_size = scil_get_compressed_data_size_limit(&dims, datatype);
	const size_t data_size = scil_dims_get_size(&dims, datatype);

	allocate(byte, buffer_out, buff_size);
	allocate(byte, tmp_buff, buff_size);

  scil_context_t* ctx;
  scil_user_hints_t hints;

  scil_user_hints_initialize(&hints);
	hints.absolute_tolerance = SCIL_ACCURACY_DBL_FINEST;

	double r = (double) scilU_get_data_randomness(buffer_in, data_size, tmp_buff, buff_size);

	char * outputFiles = getenv("SCIL_BENCHMARK_OUTPUT");
	const size_t buffer_size = scil_get_compressed_data_size_limit(&dims, datatype);

	for(int i=0; i < scilU_get_available_compressor_count(); i++ ){
		char compression_name[1024];
		sprintf(compression_name, "%s", scilU_get_compressor_name(i));
		hints.force_compression_methods = compression_name;

		int ret = scil_context_create(&ctx, datatype, 0, NULL, &hints);
		if (ret != 0){
			printf("Invalid combination %s\n", compression_name);
			continue;
		}
		assert(ctx != NULL);
		int ret_c;
		int ret_d;

		scil_timer timer;
		scilU_start_timer(& timer);
		ret_c = scil_compress(buffer_out, buff_size, buffer_in, & dims, & out_c_size, ctx);
		double seconds_compress = scilU_stop_timer(timer);
		ret_d = -1;

		double seconds_decompress;
		if(ret_c == 0){
			// initialize memory
			memset(buffer_uncompressed, -1, buffer_size);

			scilU_start_timer(& timer);
			ret_d = scil_decompress(datatype, buffer_uncompressed, & dims, buffer_out, out_c_size, tmp_buff);
			seconds_decompress = scilU_stop_timer(timer);
		}else{
			seconds_decompress = 1;
		}

		if ( ret_c == 0 && ret_d == 0 && outputFiles != NULL ){
			// dump the file
			char filename[1024];
			sprintf(filename,"%s-%s.csv", name, compression_name);
			// TODO
			//scilU_plot(filename, dims, (double*) buffer_uncompressed);
		}


		if (ret_c != 0 || ret_d != 0 ){
			error_occured = 1;
			printf("Warning: compression %s returned an error!\n",  hints.force_compression_methods);
			continue;
		}
		double c_fac = (double)(out_c_size) / data_size;

		fprintf(f, "%.1f; %d; %s; %s; %.1lf; %.1lf; %.3lf\n",
			r, datatype, name, hints.force_compression_methods,
			data_size/seconds_compress/1024 /1024, data_size/seconds_decompress/1024 /1024, c_fac);
  }
	free(buffer_out);
	free(tmp_buff);
}

void scilU_check_std_err(char const * what, int ret){
	if (ret != 0){
		critical("%s returned the error %s\n", what, strerror(errno));
		exit(1);
	}
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
int main(int argc, char** argv){
	int ret;
	scil_dims_t dims;

	if (argc != 1){
	  switch(argc - 1){
	    case (1):{
	  	  scil_dims_initialize_1d(& dims, atol(argv[1]));
	      break;
	    }case (2):{
	      scil_dims_initialize_2d(& dims, atol(argv[1]), atol(argv[2]));
	      break;
	    }case (3):{
	      scil_dims_initialize_3d(& dims, atol(argv[1]), atol(argv[2]), atol(argv[3]));
	      break;
	    }default:{
	      printf("Error will only benchmark up to 3D\n");
	      exit(1);
	    }
	  }
	}else{
		scil_dims_initialize_1d(& dims, 1024*1024);
	}

	int bufferSize = scil_get_compressed_data_size_limit(&dims, SCIL_TYPE_DOUBLE);
	double * buffer_in = (double*) malloc(bufferSize);
	buffer_uncompressed = malloc(bufferSize);

	printf("This program creates a new scil.conf by measuring performance\n");

	FILE * f = fopen("scil.conf.bak", "w+");
	{
		char * str = "#randomness; data type; pattern name; compressor name; compr. performance MiB; decompr. performance MiB; inverse compr. ratio\n";
		ret = fwrite(str, strlen(str), 1, f);
		scilU_check_std_err("fwrite", ret != 1);
	}

	char * check_pattern = getenv("SCIL_PATTERN_TO_USE");

	for(int i=0; i < scilP_get_pattern_library_size(); i++){
		char * name = scilP_get_library_pattern_name(i);

		if( check_pattern != NULL && strcmp(name, check_pattern) != 0){
			printf("Skipping %s\n", name);
			continue;
		}

		for(int d=SCIL_DATATYPE_NUMERIC_MIN; d < SCIL_DATATYPE_NUMERIC_MAX; d++ ){
			ret = scilP_create_library_pattern(buffer_in, d, &dims, i);
			assert( ret == SCIL_NO_ERR);
			benchmark(f, d, name, (byte*) buffer_in, dims);
		}
	}
	fclose(f);
	ret = rename("scil.conf.bak", "scil.conf");
	scilU_check_std_err("rename", ret);


	free(buffer_in);
	free(buffer_uncompressed);

	return error_occured;
}

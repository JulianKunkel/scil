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

#include <scil-util.h>
#include <scil-algo-chooser.h>
#include <scil-patterns.h>
#include <scil-internal.h>

#define allocate(type, name, count) type* name = (type*)malloc(count * sizeof(type))

static int error_occured = 0;
static double * buffer_uncompressed;

void benchmark(FILE * f, enum SCIL_Datatype datatype, const char * name, double * buffer_in, scil_dims dims){
	size_t out_c_size;

	const size_t buff_size = scil_compress_buffer_size_bound(datatype, &dims);
	const size_t data_size = scil_get_data_size(datatype, &dims);

	allocate(byte, buffer_out, buff_size);
	allocate(byte, tmp_buff, buff_size);

  scil_context_p ctx;
  scil_hints hints;

  scil_init_hints(&hints);
	hints.absolute_tolerance = SCIL_ACCURACY_DBL_FINEST;

	double r = (double) scilI_determine_randomness(buffer_in, data_size, tmp_buff, buff_size);

	for(int i=0; i < scil_compressors_available(); i++ ){
		char compression_name[1024];
		sprintf(compression_name, "%s", scil_compressor_name(i));
		hints.force_compression_methods = compression_name;

		int ret = scil_create_compression_context(& ctx, SCIL_TYPE_DOUBLE, &hints);
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
			scilU_start_timer(& timer);
			ret_d = scil_decompress(SCIL_TYPE_DOUBLE, buffer_uncompressed, & dims, buffer_out, out_c_size, tmp_buff);
			seconds_decompress = scilU_stop_timer(timer);
		}else{
			seconds_decompress = 1;
		}

		if (ret_c != 0 || ret_d != 0 ){
			error_occured = 1;
			printf("Warning: compression %s returned an error!\n",  hints.force_compression_methods);
			continue;
		}
		double c_fac = (double)(out_c_size) / data_size;

		fprintf(f, "%.1f; %s; %.1lf; %.1lf; %.3lf\n",
			r, hints.force_compression_methods,
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
	int variableSize = 1024*1024*10/sizeof(double);
	if (argc != 1){
		variableSize = atoll(argv[1]);
	}
	int ret;
	scil_dims dims;

	allocate(double, buffer_in, variableSize);
	buffer_uncompressed = malloc(variableSize*4*sizeof(double));

	printf("This program creates a new scil.conf by measuring performance\n");

	FILE * f = fopen("scil.conf.bak", "w+");
	{
		char * str = "#randomness; compressor name; compr. performance MiB; decompr. performance MiB; inverse compr. ratio";
		ret = fwrite(str, strlen(str), 1, f);
		scilU_check_std_err("fwrite", ret != 1);
	}

	scil_init_dims_1d(& dims, variableSize);

	for(int i=0; i < scilP_library_size(); i++){
		char * name = scilP_library_pattern_name(i);
		ret = scilP_library_create_pattern_double(i, & dims, buffer_in);
		assert( ret == SCIL_NO_ERR);
		benchmark(f, SCIL_TYPE_DOUBLE, name, buffer_in, dims);
	}
	fclose(f);
	ret = rename("scil.conf.bak", "scil.conf");
	scilU_check_std_err("rename", ret);


	free(buffer_in);
	free(buffer_uncompressed);

	return error_occured;
}

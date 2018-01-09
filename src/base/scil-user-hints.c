#include <stdio.h>
#include <string.h>
#include <float.h>
#include <stdbool.h>
#include <stdlib.h>

#include <scil-user-hints.h>
#include <scil-error.h>

const char * performance_units[] = {
	"IGNORE",
	"MiB",
	"GiB",
	"NetworkSpeed",
	"NodeLocalStorageSpeed",
	"SingleStreamSharedStorageSpeed"
};


static const char * hint_names[] = {"relative_tolerance_percent",
	"relative_err_finest_abs_tolerance",
	"absolute_tolerance",
	"significant_digits",
	"significant_bits",
	"lossless_data_range_up_to",
	"lossless_data_range_from",
	"fill_value",
	"comp_speed",
	"decomp_speed",
	"force_compression_methods",
	NULL};

static void print_hint_dbl_values(const char * name, const double val ){
	if ((val <= SCIL_ACCURACY_DBL_IGNORE && val >= SCIL_ACCURACY_DBL_IGNORE) || (val <= DBL_MAX && val >= DBL_MAX)){
		printf("\t%s:\tIGNORE\n", name);
		return;
	}
	if (val <= SCIL_ACCURACY_DBL_FINEST && val >= SCIL_ACCURACY_DBL_FINEST){
		printf("\t%s:\tFINEST\n", name);
		return;
	}
	printf("\t%s:\t%.16f\n", name, val);
}

static void print_hint_int_values(const char * name, const int val ){
	if (val == SCIL_ACCURACY_INT_FINEST){
		printf("\t%s:\tFINEST\n", name);
		return;
	}
	printf("\t%s:\t%d\n", name, val);
}


static void print_performance_hint(const char* name, const scil_performance_hint_t p)
{
    printf("\t%s: %f * %s\n", name, (double)p.multiplier, performance_units[p.unit]);
}

void scil_user_hints_initialize(scil_user_hints_t *hints)
{
    memset(hints, 0, sizeof(scil_user_hints_t));
    hints->relative_tolerance_percent        = SCIL_ACCURACY_DBL_IGNORE;
    hints->relative_err_finest_abs_tolerance = SCIL_ACCURACY_DBL_IGNORE;
    hints->absolute_tolerance                = SCIL_ACCURACY_DBL_IGNORE;
    hints->comp_speed.unit   = SCIL_PERFORMANCE_IGNORE;
    hints->decomp_speed.unit = SCIL_PERFORMANCE_IGNORE;
		hints->lossless_data_range_from = DBL_MAX;
		hints->lossless_data_range_up_to = -DBL_MAX;
		hints->fill_value = DBL_MAX;
}

void scil_user_hints_copy(scil_user_hints_t *oh, const scil_user_hints_t *hints)
{
	memcpy(oh, hints, sizeof(scil_user_hints_t));
	if(hints->force_compression_methods != NULL){
		oh->force_compression_methods = strdup(hints->force_compression_methods);
	}
}

void scil_user_hints_print(const scil_user_hints_t *hints)
{
  // TODO: implement this
	printf("hints:\n");

	print_hint_int_values("sig digits", hints->significant_digits);
	print_hint_int_values("sig bits", hints->significant_bits);
	print_hint_dbl_values("abs tol", hints->absolute_tolerance);
	print_hint_dbl_values("rel percent", hints->relative_tolerance_percent);
	print_hint_dbl_values("rel abs tol", hints->relative_err_finest_abs_tolerance);
	print_performance_hint("Comp speed", hints->comp_speed);
	print_performance_hint("Deco speed", hints->decomp_speed);
}

static int scil_readline(FILE * fd, int maxlength, char * out){
	int pos = 0;
	maxlength = maxlength - 1;
	while(pos < maxlength && ! feof(fd)){
		char ch = getc(fd);
		if(ch == '\n'){
			out[pos] = ch;
			out[pos+1] = 0;
			return pos + 1;
		}
		out[pos] = ch;
		pos++;
	}
	out[pos] = 0;
	return pos;
}

static int scil_is_empty_line_or_comment(char * out){
	for(; *out != 0; out++){
		if( *out == '#' ){
			return true;
		}
		if(! (*out == ' ' || *out == '\t' || *out == '\n')){
			return false;
		}
	}
	return true;
}

static int scil_string_to_performance(char * value, scil_performance_hint_t * p){
	char * unitstr = strstr(value, "*");
	*unitstr = 0;
	unitstr++;
	p->multiplier = atof(value);
	int unit = 0;
	p->unit = SCIL_PERFORMANCE_IGNORE;
	for(unit=0; unit < SCIL_PERFORMANCE_LAST_UNIT; unit++){
		if(strcasecmp(unitstr, performance_units[unit]) == 0){
			p->unit = unit;
			return SCIL_NO_ERR;
		}
	}
	return SCIL_EINVAL;
}

int scil_set_user_hint_from_string(scil_user_hints_t * hints, const char * var){
	int len = strlen(var);
	int i;
	for(i=0; i < len; i++){
		if(!(var[i] == ' ' || var[i] == '\t')){
			break;
		}
	}
	char * key = strdup(& var[i]);
	char * value = strstr(key, "=");
	if(value == NULL){
		return 1;
	}
	*value = 0;
	value++;
	if(value[strlen(value)-1] == '\n'){
		value[strlen(value)-1] = 0;
	}

	for(int i=0; ; i++){
		if(hint_names[i] == NULL){
			break;
		}
		if(strcasecmp(key, hint_names[i]) == 0){
			int ret;
			// found the key
			switch(i){
				case(0):
				  hints->relative_tolerance_percent = atof(value);
				  break;
				case(1):
				  hints->relative_err_finest_abs_tolerance = atof(value);
				  break;
				case(2):
				  hints->absolute_tolerance = atof(value);
				  break;
				case(3):
				  hints->significant_digits = atoi(value);
				  break;
				case(4):
				  hints->significant_bits = atoi(value);
				  break;
				case(5):
				  hints->lossless_data_range_up_to = atof(value);
				  break;
				case(6):
				  hints->lossless_data_range_from = atof(value);
				  break;
				case(7):
				  hints->fill_value = atof(value);
				  break;
				case(8):
				  ret = scil_string_to_performance(value, & hints->comp_speed);
					if(ret != SCIL_NO_ERR){
						printf("Error could not parse performance value: %s", var);
						exit(1);
					}
				  break;
				case(9):
				  ret = scil_string_to_performance(value, & hints->decomp_speed);
					if(ret != SCIL_NO_ERR){
						printf("Error could not parse performance value: %s", var);
						exit(1);
					}
				  break;
				case(10):
				  hints->force_compression_methods = strdup(value);
				  break;
				default:
					printf("Error could not parse key,value: %s,%s \n", key, value);
					exit(1);
			}
			free(key);
			return 0;
		}
	}
	free(key);
	return 1;
}

int scil_user_hints_load(scil_user_hints_t * out_hints, const char * filename, const char * variable){
	FILE * fd = fopen(filename, "r");
	if (fd == NULL){
		return SCIL_EINVAL;
	}
	// find the right variable
	int ret = 1;
	char searchstring[1024];
	sprintf(searchstring, "%s:\n", variable);
	int read_var = false;
	int have_var = false;

	while(1){ // line by lineNetworkSpeed
		char line[1024];
		ret = scil_readline(fd, 1024, line);
		if (ret == 0){
			break;
		}
		if(scil_is_empty_line_or_comment(line)){
			continue;
		}
		if(strcmp(searchstring, line) == 0){
			// now until we reach another variable
			read_var = true;
			have_var = true;
			continue;
		}
		if(strstr(line, ":") != NULL){
			read_var = false;
			continue;
		}
		if(read_var){
			ret = scil_set_user_hint_from_string(out_hints, line);
			if(ret != 0){
				printf("Error parsing line: \"%s\"\n", line);
				exit(1);
			}
		}
	}

	fclose(fd);
	if(have_var){
		return SCIL_NO_ERR;
	}
	return SCIL_EINVAL;
}

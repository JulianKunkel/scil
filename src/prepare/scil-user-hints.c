#include <scil-user-hints.h>

#include <stdio.h>
#include <string.h>
#include <float.h>

const char * performance_units[] = {
	"IGNORE",
	"MiB",
	"GiB",
	"NetworkSpeed",
	"NodeLocalStorageSpeed",
	"SingleStreamSharedStorageSpeed"
};

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


static void print_performance_hint(const char* name, const scilPr_performance_hint_t p)
{
    printf("\t%s: %f * %s\n", name, (double)p.multiplier, performance_units[p.unit]);
}

void scilPr_initialize_user_hints(scil_user_hints_t* hints)
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

void scilPr_copy_user_hints(scil_user_hints_t * oh, const scil_user_hints_t* hints)
{
	memcpy(oh, hints, sizeof(scil_user_hints_t));
	/*if(hints->force_compression_methods != NULL){
		oh->force_compression_methods = NULL;
	}
	*/
}

void scilPr_print_user_hints(const scil_user_hints_t* hints)
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

#include <scil-user-params.h>

#include <stdio.h>
#include <string.h>

static void print_performance_hint(const char* name, const scilPr_performance_hint_t p)
{
    printf("\t%s: %f * %s\n", name, (double)p.multiplier, performance_units[p.unit]);
}

void scilPr_initialize_user_hints(scilPr_user_hints_t* hints)
{
    memset(hints, 0, sizeof(scilPr_user_hints_t));
    hints->relative_tolerance_percent        = SCIL_ACCURACY_DBL_IGNORE;
    hints->relative_err_finest_abs_tolerance = SCIL_ACCURACY_DBL_IGNORE;
    hints->absolute_tolerance                = SCIL_ACCURACY_DBL_IGNORE;

    hints->comp_speed.unit   = SCIL_PERFORMANCE_IGNORE;
    hints->decomp_speed.unit = SCIL_PERFORMANCE_IGNORE;
}

void scilPr_copy_user_hints(scilPr_user_hints_t * oh, const scilPr_user_hints_t* hints){
	memcpy(oh, hints, sizeof(scilPr_user_hints_t));
	/*if(hints->force_compression_methods != NULL){
		oh->force_compression_methods = NULL;
	}
	*/
}

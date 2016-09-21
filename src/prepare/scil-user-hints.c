#include <scil-user-hints.h>

#include <stdio.h>
#include <string.h>

static const char* const performance_units[] = {
	"IGNORE",
	"MiB",
	"GiB",
	"NetworkSpeed",
	"NodeLocalStorageSpeed",
	"SingleStreamSharedStorageSpeed"
};

void scilPr_print_performance_hint(const char* const name, const scilPr_performance_hint_t p)
{
    printf("\t%s: %f * %s\n", name, (double)p.multiplier, performance_units[p.unit]);
}

void scilPr_initialize_user_hints(scilPr_user_hints_t* const hints)
{
    memset(hints, 0, sizeof(scilPr_user_hints_t));
    hints->relative_tolerance_percent        = SCIL_ACCURACY_DBL_IGNORE;
    hints->relative_err_finest_abs_tolerance = SCIL_ACCURACY_DBL_IGNORE;
    hints->absolute_tolerance                = SCIL_ACCURACY_DBL_IGNORE;

    hints->comp_speed.unit   = SCIL_PERFORMANCE_IGNORE;
    hints->decomp_speed.unit = SCIL_PERFORMANCE_IGNORE;
}

void scilPr_copy_user_hints(scilPr_user_hints_t * oh, const scilPr_user_hints_t* const hints)
{
	memcpy(oh, hints, sizeof(scilPr_user_hints_t));
}

void scilPr_print_user_hints(const scilPr_user_hints_t* const hints)
{
    // TODO: Implement this
    return;
}

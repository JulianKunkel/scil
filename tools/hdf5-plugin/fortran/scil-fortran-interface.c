#include <stdio.h>
#include <assert.h>
#include <hdf5.h>
#include <string.h>

#include <scil-hdf5-plugin.h>

#include <scil.h>
#include <scil-util.h>

void h5pset_scil_compression_hints_f_(int32_t * prop_id_p,
  double * relative_tolerance_percent,
  double * relative_err_finest_abs_tolerance,
  double * absolute_tolerance,
  int * significant_digits,
  int * significant_bits)
{
  hid_t prop_id = (hid_t) *prop_id_p;
  printf("Property ID: %lld\n", (long long) prop_id);

  H5Pset_filter(prop_id, (H5Z_filter_t) SCIL_ID, H5Z_FLAG_MANDATORY, 0, NULL);

  scil_user_hints_t hints;
  scil_user_hints_initialize( & hints);
  hints.relative_tolerance_percent = * relative_tolerance_percent;
  hints.relative_err_finest_abs_tolerance = * relative_err_finest_abs_tolerance;
  hints.absolute_tolerance = * absolute_tolerance;
  hints.significant_digits = * significant_digits;
  hints.significant_bits = * significant_bits;
  H5Pset_scil_user_hints_t(prop_id, & hints);
}

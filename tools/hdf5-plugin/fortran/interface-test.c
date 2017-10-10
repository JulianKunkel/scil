#include <stdio.h>
#include <assert.h>
#include <hdf5.h>
#include <string.h>

#include <scil-hdf5-plugin.h>

#include <scil.h>
#include <scil-util.h>

void h5pset_scil_compression_hints_f_(hid_t dcpl, int a, int *b, double c)
{
  scil_user_hints_t hints;
  scilPr_initialize_user_hints(& hints);
  //H5Pset_scil_user_hints_t(dcpl, & hints);

  printf("%s%d %f\n", "Ergebnis = ", a * *b, c);
}

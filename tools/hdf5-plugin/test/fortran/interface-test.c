#include <stdio.h>
#include <scil.h>
#include <scil-hdf5-plugin.h>

void h5pset_scil_compression_hints_f(hid_t dcpl, int a, int *b, double c)
{
  scil_user_hints_t hints;
  scilPr_initialize_user_hints(& hints);
  //H5Pset_scil_user_hints_t(hid_t dcpl, scil_user_hints_t * hints);

  printf("%s%d %f\n", "Ergebnis = ", a * *b, c);
}

#include <stdio.h>
#include <assert.h>

#include <scil.h>

int main(){
  scil_user_hints_t hints;
  scil_user_hints_initialize(& hints);

  int ret = scil_user_hints_load(& hints, "../../../src/compression/test/user-hints.cfg", "test");
  if( ret != SCIL_NO_ERR ) {
    printf("Error reading test! %s\n", scil_error_get_message(ret));
    exit(1);
  }
  scil_user_hints_print(& hints);

  ret = scil_user_hints_load(& hints, "../../../src/compression/test/user-hints.cfg", "not-existing");
  if( ret == SCIL_NO_ERR ) {
    printf("Error not-existing exists! %s\n", scil_error_get_message(ret));
    exit(1);
  }

  return 0;
}

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

#include <stdio.h>
#include <assert.h>

#include <scil-util.h>

int main(){
  int v = 1;
  char *p = (char*) & v;

  int little_endian = *p;
  int big_endian = *(p + sizeof(int) - 1);

  assert(little_endian | big_endian);
  assert(! (little_endian && big_endian));

  printf("little %d big: %d\n", little_endian, big_endian);

  char buffer[10];
  int result = -1;

  scilU_pack4(buffer, v);
  scilU_unpack4(buffer, & result);

  printf("%d == %d\n", v, result);
  assert(v == result);

  uint64_t v64 = 3343224241llu;
  uint64_t result64 = -1;
  scilU_pack8(buffer, v64);
  scilU_unpack8(buffer, & result64);

  printf("%llu == %llu\n", (long long unsigned) v64, (long long unsigned) result64);
  assert(v64 == result64);

  scilU_reverse_copy(& result64, & v64, sizeof(v64));
  for(unsigned i=0; i < sizeof(v64); i++){
    assert( ((char*) & v64)[i] == ((char*) & result64)[sizeof(v64)-i-1]);
  }
  printf("%llu == %llu\n", (long long unsigned) v64, (long long unsigned) result64);

  double testVal = 2.95;
  int64_t packed;
  scilU_pack8((void*) & packed, testVal);

  printf("%llu == %llu\n", *(unsigned long long*) &testVal, (unsigned long long) packed );
  double unpacked = 0;
  scilU_unpack8(& packed, & unpacked);
  printf("%f\n", unpacked);
  assert(testVal == unpacked);
  return 0;
}

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

#include <scil.h>


int main(){
  int ret;

  scil_hints hints;
  scil_init_hints(& hints);

  //hints.

  scil_context_p ctx;
  ret = scil_create_compression_context(& ctx, & hints);
  assert(ret);

  scil_hints_print(& hints);

  ret = scil_destroy_compression_context(& ctx);

  return ret;
}

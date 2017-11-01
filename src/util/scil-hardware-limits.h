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

#ifndef SCIL_HARDWARE_LIMITS_H
#define SCIL_HARDWARE_LIMITS_H

enum hardware_limit_e{
  NETWORK = 0,
  STORAGE = 1,
  HARDWARE_MAX
};

void scilU_initialize_hardware_limits();

int scilU_add_hardware_limit(const char* name, const char* str);


#endif // SCIL_HARDWARE_LIMITS_H

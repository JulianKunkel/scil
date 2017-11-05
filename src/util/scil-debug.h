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

#ifndef SCIL_DEBUG_H
#define SCIL_DEBUG_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#ifdef DEBUG
  #define debug(...) fprintf(stderr, "[SCIL DEBUG] "__VA_ARGS__);
#else
  #define debug(...)
#endif

#ifdef DEBUG_INTERNALS
  #define debugI(...) fprintf(stderr, "[SCIL DEBUG I] "__VA_ARGS__);
#else
  #define debugI(...)
#endif

#define critical(...) { fprintf(stderr, "[SCIL CRITICAL] "__VA_ARGS__); exit(1); }
#define warn(...) fprintf(stderr, "[SCIL WARN] "__VA_ARGS__);

#define FUNC_START debug("CALL %s\n", __PRETTY_FUNCTION__);

#endif // SCIL_INTERNAL_H

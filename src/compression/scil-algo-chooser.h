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

#ifndef SCIL_ALGO_CHOOSER_H
#define SCIL_ALGO_CHOOSER_H

#include <scil-context.h>
#include <scil-dims.h>

void scilC_algo_chooser_initialize();

void scilC_algo_chooser_execute(const void* const restrict source, scil_dims* dims, scil_context_p ctx);

#endif // SCIL_ALGO_CHOOSER_H

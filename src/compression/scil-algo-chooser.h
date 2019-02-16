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
#include <scil-dict.h>
#include <scil-decision-tree.h>

scilU_dict_t *variable_dict;
scilU_decision_tree* decision_tree;

void scilC_algo_chooser_initialize();

void scilC_algo_chooser_execute(const void* restrict source,
                                const scil_dims_t* dims,
                                scil_context_t* ctx);

#endif // SCIL_ALGO_CHOOSER_H

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

#ifndef SCIL_HDF5_PLUGIN_H
#define SCIL_HDF5_PLUGIN_H

#define H5_HAVE_FILTER_SCIL
#define SCIL_ID 32003

#include <scil.h>

/*
 * Primitive versions for providing hints to HDF5 data sets
 * You are not allowed to modify the hints after you have set them.
 * Memory should be managed externally, you should free memory after you close the dataset that is compressed.
 */
herr_t H5Pset_scil_user_hints_t(hid_t dcpl, scil_user_hints_t * hints);

/*
 * The memory of the hints is managed internally. Do not free them while the plugin is operational.
 */
herr_t H5Pget_scil_user_hints_t(hid_t dcpl, scil_user_hints_t ** out_hints);

#endif

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

/*
 * Copyright (c) 2008, Jerome Fimes, Communications & Systemes <jerome.fimes@c-s.fr>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS `AS IS'
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <assert.h>
#include <stdio.h>


#include <scil-util.h>

#include <openjpeg.h>
#include <opj_config.h>

#define SUCCESS 0

int main(){

  const size_t count = 10;

  float* source = (float*)SAFE_MALLOC(count * sizeof(float));

  for(int i = 0; i < count; ++i){
      source[i] = i % 10;
      printf("%d ", source[i]);
  }

  scil_dims dims;
  scil_init_dims_1d(&dims, count);

  size_t dest_size = scil_compress_buffer_size_bound(SCIL_TYPE_FLOAT, &dims);
  byte* dest       = (byte*)SAFE_MALLOC(dest_size);

  scil_context_p ctx;
  scil_hints hints;

  scil_init_hints(&hints);

  hints.absolute_tolerance        = 1e-100;
  hints.force_compression_methods = "opj";
  scil_create_compression_context(&ctx, SCIL_TYPE_FLOAT, &hints);

  int ret = scil_compress(dest, dest_size, source, &dims, &dest_size, ctx);
  printf("\n\n%d %d\n", ret, SCIL_PRECISION_ERR);

  free(source);
  free(dest);
  free(ctx);

  return ret != SCIL_PRECISION_ERR;
}

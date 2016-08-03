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

  printf("Begin TEST\n");
  int ret = 0;
  const size_t count1 = 64;
  const size_t count2 = 64;
  //static float *data_check;

  const size_t source_size = count1 * count2 * sizeof(int16_t);

  int16_t* source = (int16_t *)SAFE_MALLOC(source_size);

  for(uint i = 0; i < count1; i++){
    for (uint j = 0; j < count2; j++){
      *(source + i*count2 + j) = 2;
    }
  }

  //scilU_print_buffer((char*)source, 20);
  printf("Array size: %i\n", source_size);

  scil_dims dims;
  scil_init_dims_2d(&dims, count1, count2);

  size_t dest_size = scil_compress_buffer_size_bound(SCIL_TYPE_INT16, &dims);
  byte* dest       = (byte*)SAFE_MALLOC(dest_size);

  //printf("Dest array size: %i\n", dest_size);

  printf("SOURCE: %x %x %x %x %x %x %x %x \n",source[0], source[1],source[2],source[3],source[4],source[5],source[6],source[7]);

  scil_context_p ctx;
  scil_hints hints;

  scil_init_hints(&hints);

  hints.absolute_tolerance        = 1e-100;
  hints.force_compression_methods = "opj";
  scil_create_compression_context(&ctx, SCIL_TYPE_INT16, 0, NULL, &hints);

  printf("##Compression##\n");
  //printf("DEST: %lu\n",dest_size);

  //scilU_print_buffer((char*)source, count1*count2*sizeof(float));

  ret = scil_compress(dest, dest_size, source, &dims, &dest_size, ctx);
  //printf("\n\n?error %d %d\n", ret, SCIL_PRECISION_ERR);
  //assert(ret == SCIL_NO_ERR && "ERROR COMPRESSION");
  //printf("DEST: %lu",dest_size);

  printf("ENCODED: %x %x %x %x %x %x %x %x \n",dest[0], dest[1],dest[2],dest[3],dest[4], dest[5],dest[6],dest[7]);

  int16_t* data_check        = (int16_t*)malloc(count1*count2*sizeof(SCIL_TYPE_INT16));
  byte* tmpBuff        = (byte*)malloc(count1*count2*sizeof(SCIL_TYPE_INT16));

  printf("##Decompression##\n");
  //memset(data_check, 0, sizeof(data_check));
  ret = scil_decompress(SCIL_TYPE_INT16, data_check, & dims, dest, dest_size, tmpBuff);
  assert(ret == SCIL_NO_ERR && "ERROR DECOMPRESSION");

  printf("DECODED: %x %x %x %x \n",data_check[0], data_check[1],data_check[2],data_check[3]);

  //printf("Printed buffer \n");
  //scilU_print_buffer((char*)data_check, 8);

  free(source);
  free(dest);

  printf("End   TEST\n");
  return ret;
}

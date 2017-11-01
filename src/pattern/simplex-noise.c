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

#include <simplex-noise.h>

#include <scil-error.h>
#include <scil-util.h>

#include <assert.h>

#include <open-simplex-noise.h>

static int simplex(double* buffer, const scil_dims_t* dims, double mn, double mx, double arg, double arg2, int seed){
  const int frequencyCount = (int) arg2;
  double highFrequency = (double) arg;

  if( scilU_double_equal(mn, mx) || frequencyCount <= 0 ){
    return SCIL_EINVAL;
  }

  struct osn_context *ctx;
  open_simplex_noise(seed, &ctx);

  int64_t max_potenz = 1<<frequencyCount;
  const size_t* len = dims->length;

  switch(dims->dims){
    case (1):{
      size_t count = scil_dims_get_count(dims);
      for (size_t i=0; i < count; i++){
        buffer[i] = 0;
        int64_t potenz = max_potenz;
        int64_t divisor = 1;
        for(int f = 1 ; f <= frequencyCount; f++){
          buffer[i] += potenz * open_simplex_noise2(ctx, 1.0, (double) i / count * divisor * highFrequency);
          potenz /= 2;
          divisor *= 2;
        }
      }
      break;
    }case (2):{
      for (size_t y=0; y < len[1]; y++){

        for (size_t x=0; x < len[0]; x++){
          double var = 0;
          int64_t potenz = max_potenz;
          int64_t divisor = 1;
          for(int f = 1 ; f <= frequencyCount; f++){
            var += potenz * open_simplex_noise2(ctx, (double) x / len[0] * divisor*highFrequency, (double) y / len[1] * divisor*highFrequency);
            potenz /= 2;
            divisor *= 2;
          }
          buffer[x+y*len[0]] = var;
        }

      }
      break;
    }case (3):{
      for (size_t z=0; z < len[2]; z++){
        for (size_t y=0; y < len[1]; y++){
          for (size_t x=0; x < len[0]; x++){
            double var = 0;
            int64_t potenz = max_potenz;
            int64_t divisor = 1;
            for(int f = 1 ; f <= frequencyCount; f++){
              var += potenz * open_simplex_noise3(ctx, (double) x / len[0]*divisor*highFrequency, (double) y / len[1]*divisor*highFrequency, (double) z / len[2]*divisor*highFrequency);
              potenz /= 2;
              divisor *= 2;
            }
            buffer[x+y*len[0]+z*(len[0]*len[1])] = var;
          }
        }
      }
      break;
    }case (4):{
      for (size_t w=0; w < len[3]; w++){
        for (size_t z=0; z < len[2]; z++){
          for (size_t y=0; y < len[1]; y++){
            for (size_t x=0; x < len[0]; x++){
              double var = 0;
              int64_t potenz = max_potenz;
              int64_t divisor = 1;
              for(int f = 1 ; f <= frequencyCount; f++){
                var += potenz * open_simplex_noise4(ctx, (double) x / len[0]*divisor*highFrequency, (double) y / len[1]*divisor*highFrequency, (double) z / len[2]*divisor*highFrequency, (double) w / len[3]*divisor*highFrequency);
                potenz /= 2;
                divisor *= 2;
              }
              buffer[x+len[0]*(y+len[1]*(z+len[2]*w))] = var;
            }
          }
        }
      }
      break;
    }default:
      assert(0 && "Not supported");
  }

  // fix min + max, first identify min/max
  scilP_change_data_scale(buffer, dims, mn, mx);

  open_simplex_noise_free(ctx);

  return SCIL_NO_ERR;
}

scil_pattern_t scil_pattern_simplex_noise = { &simplex, "simplexNoise" };

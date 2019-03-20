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
#include <algo/algo-wavelets.h>

#include "macros.h"
#include "alloc.h"
#include "wav_filters.h"
#include "wav_trf.h"
#include "wav_gen.h"
#include "wav_basic.h"
#include <string.h>
#include <math.h>

#include <scil-error.h>

// Repeat for each data type

// hard threshold the values in matrix.
int hard_threshold_<DATATYPE>(<DATATYPE> **matrix,int Ni,int Nj,double threshold)

{
	int i,j,cnt=0;

	for(i=0;i<Ni;i++)
		for(j=0;j<Nj;j++) {

			if(fabs(matrix[i][j])<=threshold) {

				matrix[i][j]=0;
				cnt++;
			}
		}
	return(cnt);
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
int scil_wavelets_compress_<DATATYPE>(const scil_context_t* ctx,
                        byte * restrict dest,
                        size_t* restrict dest_size,
                        <DATATYPE>*restrict source,
                        const scil_dims_t* dims)
{
    int i;
  	int shift_arr_r[MAX_ARR_SIZE],shift_arr_c[MAX_ARR_SIZE];
    double threshold = 1;

  	// 3 levels of wavelets.
  	const int levs=1;
  	int Nl,Nh;
  	float *lp,*hp;

    // Choose wavelet filter. List of filters is in wav_trf.c
  	choose_filter('H',9);

    // Select the forward bank of filters.
    lp=MFLP;Nl=Nflp;
    hp=MFHP;Nh=Nfhp;

		if(dims->dims == 1){
			return SCIL_EINVAL;
		}

    /*if (dims->dims==1){
      int Nj = dims->length[0];
      float *buffer_t;
      buffer_t=allocate_1d_float(dims->length[0],0);
      filter_n_decimate(buffer_t,source,Nj,lp,Nl,begflp,2);
      memcpy(dest,buffer_t,Nj*sizeof(float));

      // Free buffers.
    	free(buffer_t);
      return 0;
    }*/

    <DATATYPE> **buffer_t;
    int Ni = dims->length[1], Nj = dims->length[0];

  	// Main buffer for operations.
  	buffer_t=allocate_2d_<DATATYPE>(Ni,Nj,0);

	  for(i=levs-1;i>=0;i--) {
  		shift_arr_r[i]=shift_arr_c[i]=0;
  	}

  	for(i=0;i<Ni;i++) {
      memcpy(buffer_t[i], source+i*Nj, Nj*sizeof(<DATATYPE>));
  	}

  	wav2d_inpl((float**)buffer_t,Ni,Nj,levs,lp,Nl,hp,Nh,1,shift_arr_r,shift_arr_c);

    hard_threshold_<DATATYPE>(buffer_t,Ni,Nj,threshold);

    memcpy(dest, &levs, sizeof(int));
    for(i=0;i<Ni;i++) {
      memcpy(dest+i*Nj*sizeof(<DATATYPE>)+sizeof(int), buffer_t[i], Nj*sizeof(<DATATYPE>));
    }

  	// Free buffers.
  	free_2d_<DATATYPE>(buffer_t,Ni);

    return 0;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
int  scil_wavelets_decompress_<DATATYPE>( <DATATYPE>*restrict data_out,
                            scil_dims_t* dims,
                            byte*restrict compressed_buf_in,
                            const size_t in_size)
{
  int i;
  int Ni = dims->length[1], Nj = dims->length[0];
  <DATATYPE> **buffer_t;
  int shift_arr_r[MAX_ARR_SIZE],shift_arr_c[MAX_ARR_SIZE];

  //levels of wavelets.
  int levs;
  memcpy(&levs, compressed_buf_in, sizeof(int));
  int Nl,Nh;
  float *lp,*hp;

  buffer_t=allocate_2d_<DATATYPE>(Ni,Nj,0);

  for(i=0;i<Ni;i++) {
    memcpy(buffer_t[i], compressed_buf_in+i*Nj*sizeof(<DATATYPE>)+sizeof(int), Nj*sizeof(<DATATYPE>));
  }

  // Choose wavelet filter. List of filters is in wav_trf.c
  choose_filter('H',9);

  for(i=levs-1;i>=0;i--) {
    shift_arr_r[i]=shift_arr_c[i]=0;
  }

  // Select the inverse bank of filters.
  lp=MILP;Nl=Nilp;
  hp=MIHP;Nh=Nihp;

  // Inverse transform.
  wav2d_inpl((float**)buffer_t,Ni,Nj,levs,lp,Nl,hp,Nh,0,shift_arr_r,shift_arr_c);

  for(i=0;i<Ni;i++) {
    memcpy(data_out+i*Nj, buffer_t[i], Nj*sizeof(<DATATYPE>));
  }

  // Free buffers.
  free_2d_<DATATYPE>(buffer_t,Ni);

  return 0;
}
// End repeat

scilU_algorithm_t algo_wavelets = {
    .c.DNtype = {
        CREATE_INITIALIZER(scil_wavelets)
    },
    "wavelets",
    11,
    SCIL_COMPRESSOR_TYPE_DATATYPES,
		1
};

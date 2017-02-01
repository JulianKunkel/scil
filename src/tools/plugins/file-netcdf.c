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

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netcdf.h>

#include <scil-util.h>
#include <plugins/file-netcdf.h>

static option_help options [] = {
  LAST_OPTION
};

static option_help * get_options(){
  return options;
}

/*
From netCDF type in group igrp, get size in memory needed for each
 * value */
//inq_value_size(int igrp, nc_type vartype, size_t *value_sizep)
//copy_var_data(int igrp, int inkind, int varid, int ogrp)
//copy_data(int igrp, int inkind, int ogrp)
// to read all values::: nc_get_vara_double(ncid, rh_id, start, count, rh_vals);

/* reverse string */
void reverse(char s[])
{
    int i, j;
    char c;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

void itoa(int n, char s[])
 {
     int i, sign;

     if ((sign = n) < 0)
         n = -n;
     i = 0;
     do {
         s[i++] = n % 10 + '0';
     } while ((n /= 10) > 0);
     if (sign < 0)
         s[i++] = '-';
     s[i] = '\0';
     reverse(s);
 }

static int readData(const char * name, byte ** out_buf, SCIL_Datatype_t * out_datatype, scil_dims_t * out_dims, size_t * read_size){
  printf("READDATA\n");
  int ncid;
  int ndims_in, nvars_in, ngatts_in, unlimdimid_in;
  int varid;
  size_t lengthp[NC_MAX_VAR_DIMS];
  int formatp, rh_ndims, rh_dimids[NC_MAX_VAR_DIMS];
  nc_type rh_type;

  /* Error handling. */
  int retval;

  byte *input_data = NULL;

  /* Open the file. */
  if (retval = nc_open(name, NC_NOWRITE, &ncid))
  {
    NC_ISSYSERR(retval);
    return 1;
  }
  nc_inq_format(ncid, &formatp);
  printf("%s file opened for read\n", (formatp == NC_FORMAT_CLASSIC)?"NC_FORMAT_CLASSIC":(formatp == NC_FORMAT_64BIT_OFFSET)?"NC_FORMAT_64BIT_OFFSET":(formatp == NC_FORMAT_NETCDF4)?"NC_FORMAT_NETCDF4":"another");
  //NC_FORMAT_CLASSIC, NC_FORMAT_64BIT_OFFSET, NC_FORMAT_CDF5, NC_FORMAT_NETCDF4, NC_FORMAT_NETCDF4_CLASSIC.

  if (retval = nc_inq(ncid, &ndims_in, &nvars_in, &ngatts_in, &unlimdimid_in))
  {
    NC_ISSYSERR(retval);
    return 1;
  }

  printf("\n\n====OUTPUT====\nnumber of dimensions: %i\nnumber of variables: %i\nnumber of global attributes: %i\nunlimited dimentions: %i\n\n", ndims_in, nvars_in, ngatts_in, unlimdimid_in);

  if (nvars_in >1)
  {
    printf("ERROR: you can load now only file with one up to 2-dimensional variable\n");
    return 1;
  }

  /*Get variables*/
  for (int i = 0; i < nvars_in; i++)
  {
    if (retval = nc_inq_var (ncid, i, NULL, &rh_type, &rh_ndims, rh_dimids, NULL))
      NC_ISSYSERR(retval);
    else if (rh_ndims > 2)
    {
      printf("ERROR: you can load now only file with one up to 2-dimensional variable\n");
      return 1;
    }
    else
    {
      switch(rh_type){
        case(NC_DOUBLE):
          *out_datatype = SCIL_TYPE_DOUBLE;
          break;
        case(NC_FLOAT):
          *out_datatype = SCIL_TYPE_FLOAT;
          break;
        case(NC_BYTE):
          *out_datatype = SCIL_TYPE_INT8;
          break;
        case(NC_SHORT):
          *out_datatype = SCIL_TYPE_INT16;
          break;
        case(NC_INT):
          *out_datatype = SCIL_TYPE_INT32;
          break;
        case(NC_INT64):
          *out_datatype = SCIL_TYPE_INT64;
          break;
        default:
          printf("Not supported datatype in writeData\n");
        }
      for (int j = 0; j < rh_ndims; j++)
        if (retval = nc_inq_dim(ncid, rh_dimids[j], NULL, &lengthp[j]))
          NC_ISSYSERR(retval);

      if (rh_ndims == 2)
      {
        scilPr_initialize_dims_2d(out_dims, lengthp[0], lengthp[1]);

        input_data = (byte*) malloc(scilPr_get_compressed_data_size_limit(out_dims, *out_datatype));

        nc_get_var_float(ncid,i,(float*)input_data);
      }
    }
  }

  /* Close the file. */
  if ((retval = nc_close(ncid)))
  {
    NC_ISSYSERR(retval);
    return 1;
  }

  printf("*** SUCCESS reading example file %s!\n", name);

  *read_size = scilPr_get_dims_size(out_dims, *out_datatype);
  *out_buf = input_data;
  return 0;
}

static int writeData(const char * name, const byte * buf, SCIL_Datatype_t buf_datatype, size_t elements, SCIL_Datatype_t orig_datatype, scil_dims_t dims){
  printf("WRITEDATA\n");
  int ncid;
  int ndims_in, nvars_in, ngatts_in, unlimdimid_in;
  int dimids[NC_MAX_VAR_DIMS];
  int varid;
  int ncdatatype;

  /* Error handling. */
  int retval;

  switch(buf_datatype){
    case(SCIL_TYPE_DOUBLE):
      ncdatatype = NC_DOUBLE;
      break;
    case(SCIL_TYPE_FLOAT):
      ncdatatype = NC_FLOAT;printf("float\n");
      break;
    case(SCIL_TYPE_INT8):
      ncdatatype = NC_BYTE;
      break;
    case(SCIL_TYPE_INT16):
      ncdatatype = NC_SHORT;
      break;
    case(SCIL_TYPE_INT32):
      ncdatatype = NC_INT;
      break;
    case(SCIL_TYPE_INT64):
      ncdatatype = NC_INT64;
      break;
    default:
      printf("Not supported datatype in writeData\n");
    }
  /* Write new file*/
  if (retval = nc_create(name, NC_NETCDF4, &ncid))
    NC_ISSYSERR(retval);

  char cbuffer[5];//99+dim


  for (int i = 0; i < dims.dims; i++)
  {
    memset(cbuffer,0,strlen(cbuffer));
    itoa(i,cbuffer);
    strcat( cbuffer, "dim");
    if (retval = nc_def_dim(ncid, cbuffer, dims.length[i], &dimids[i]))
      NC_ISSYSERR(retval);
  }


  if (retval = nc_def_var(ncid, "data", ncdatatype, dims.dims, dimids, &varid))
    NC_ISSYSERR(retval);

  /* End define mode. This tells netCDF we are done defining
   * metadata. */
  if (retval = nc_enddef(ncid))
     NC_ISSYSERR(retval);

  switch(buf_datatype){//output_datatype
    if (retval = nc_put_var_double(ncid, varid, (const double*) buf))
      NC_ISSYSERR(retval);
         break;
       case(SCIL_TYPE_FLOAT):
       if (retval = nc_put_var_float(ncid, varid, (const float*) buf))
         NC_ISSYSERR(retval);
         break;
       case(SCIL_TYPE_INT8):
       if (retval = nc_put_var_text(ncid, varid, (const char*) buf))
         NC_ISSYSERR(retval);
         break;
       case(SCIL_TYPE_INT16):
       if (retval = nc_put_var_short(ncid, varid, (const short*) buf))
         NC_ISSYSERR(retval);
         break;
       case(SCIL_TYPE_INT32):
       if (retval = nc_put_var_int(ncid, varid, (const int*) buf))
         NC_ISSYSERR(retval);
         break;
       case(SCIL_TYPE_INT64):
       if (retval = nc_put_var_longlong(ncid, varid, (const long long*) buf))
         NC_ISSYSERR(retval);
         break;
       default:
         printf("Not supported datatype in writeData\n");
       }

  /* Close the file. */
  if ((retval = nc_close(ncid)))
  {
    NC_ISSYSERR(retval);
    return 1;
  }

  printf("*** SUCCESS writing file %s!\n", name);


  return 0;
}

scil_file_plugin_t netcdf_plugin = {
  "netcdf",
  "nc",
  get_options,
  readData,
  writeData
};
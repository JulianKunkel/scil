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

#include <algo/algo-opj.h>

#include <openjpeg.h>
#include <opj_config.h>

/* -------------------------------------------------------------------------- */

/**
sample error debug callback expecting no client object
*/
static void error_callback(const char *msg, void *client_data) {
	(void)client_data;
	fprintf(stdout, "[ERROR] %s", msg);
}
/**
sample warning debug callback expecting no client object
*/
static void warning_callback(const char *msg, void *client_data) {
	(void)client_data;
	fprintf(stdout, "[WARNING] %s", msg);
}
/**
sample debug callback expecting no client object
*/
static void info_callback(const char *msg, void *client_data) {
	(void)client_data;
	fprintf(stdout, "[INFO] %s", msg);
}

/* -------------------------------------------------------------------------- */

#define NUM_COMPS_MAX 4

#pragma GCC diagnostic ignored "-Wunused-parameter"
int scil_opj_compress(const scil_context_p ctx, byte* restrict dest, size_t* restrict dest_size, const byte*restrict source, const size_t source_size){
  opj_cparameters_t param;
  opj_image_cmptparm_t params [NUM_COMPS_MAX];
  opj_codec_t* codec;
  opj_stream_t* stream;
  opj_image_t* image;
  OPJ_UINT32 nb_tiles;
	OPJ_UINT32 data_size = source_size;

  opj_image_cmptparm_t * current_param_ptr;
	OPJ_UINT32 i;
	OPJ_BYTE *data;

  OPJ_UINT32 num_comps;
  int image_width;
  int image_height;
  int tile_width;
  int tile_height;
  int comp_prec;
  int irreversible;

	data = (OPJ_BYTE*) malloc(data_size * sizeof(OPJ_BYTE));
	for (i=0;i<data_size;++i)	{
		data[i] = (OPJ_BYTE) source[i];
	}

  opj_set_default_encoder_parameters(&param);
  param.tcp_numlayers = 1;
	param.cp_fixed_quality = 1;
	param.tcp_distoratio[0] = 20;
	/* is using others way of calculation */
	/* param.cp_disto_alloc = 1 or param.cp_fixed_alloc = 1 */
	/* param.tcp_rates[0] = ... */


	/* tile definitions parameters */
	/* position of the tile grid aligned with the image */
	param.cp_tx0 = 0;
	param.cp_ty0 = 0;
	/* tile size, we are using tile based encoding */
	param.tile_size_on = OPJ_TRUE;
	param.cp_tdx = tile_width;
	param.cp_tdy = tile_height;

	/* use irreversible encoding ?*/
	param.irreversible = irreversible;

	/* do not bother with mct, the rsiz is set when calling opj_set_MCT*/
	/*param.cp_rsiz = OPJ_STD_RSIZ;*/

	/* no cinema */
	/*param.cp_cinema = 0;*/

	/* no not bother using SOP or EPH markers, do not use custom size precinct */
	/* number of precincts to specify */
	/* param.csty = 0;*/
	/* param.res_spec = ... */
	/* param.prch_init[i] = .. */
	/* param.prcw_init[i] = .. */


	/* do not use progression order changes */
	/*param.numpocs = 0;*/
	/* param.POC[i].... */

	/* do not restrain the size for a component.*/
	/* param.max_comp_size = 0; */

	/** block encoding style for each component, do not use at the moment */
	/** J2K_CCP_CBLKSTY_TERMALL, J2K_CCP_CBLKSTY_LAZY, J2K_CCP_CBLKSTY_VSC, J2K_CCP_CBLKSTY_SEGSYM, J2K_CCP_CBLKSTY_RESET */
	/* param.mode = 0;*/

	/** number of resolutions */
	param.numresolution = 6;

	/** progression order to use*/
	/** OPJ_LRCP, OPJ_RLCP, OPJ_RPCL, PCRL, CPRL */
	param.prog_order = OPJ_LRCP;

	/** no "region" of interest, more precisally component */
	/* param.roi_compno = -1; */
	/* param.roi_shift = 0; */

	/* we are not using multiple tile parts for a tile. */
	/* param.tp_on = 0; */
	/* param.tp_flag = 0; */
  /* image definition */
	current_param_ptr = params;
	for (i=0;i<num_comps;++i) {
		/* do not bother bpp useless */
		/*l_current_param_ptr->bpp = COMP_PREC;*/
		current_param_ptr->dx = 1;
		current_param_ptr->dy = 1;

		current_param_ptr->h = (OPJ_UINT32)image_height;
		current_param_ptr->w = (OPJ_UINT32)image_width;

		current_param_ptr->sgnd = 0;
		current_param_ptr->prec = (OPJ_UINT32)comp_prec;

		current_param_ptr->x0 = 0;
		current_param_ptr->y0 = 0;

		++current_param_ptr;
	}

  codec = opj_create_compress(OPJ_CODEC_JP2);//J2K
  /* catch events using our callbacks and give a local context */
	opj_set_info_handler(codec, info_callback,00);
	opj_set_warning_handler(codec, warning_callback,00);
	opj_set_error_handler(codec, error_callback,00);

  image = opj_image_tile_create(num_comps,params,OPJ_CLRSPC_SRGB);
	if (! image) {
		opj_destroy_codec(codec);
		return SCIL_UNKNOWN_ERR;
	}

  image->x0 = 0;
	image->y0 = 0;
	image->x1 = (OPJ_UINT32)image_width;
	image->y1 = (OPJ_UINT32)image_height;
	image->color_space = OPJ_CLRSPC_SRGB;

	if (! opj_setup_encoder(codec,&param,image)) {
		fprintf(stderr, "ERROR -> test_tile_encoder: failed to setup the codec!\n");
		opj_destroy_codec(codec);
		opj_image_destroy(image);
		return SCIL_UNKNOWN_ERR;
	}

  stream = opj_stream_default_create(OPJ_FALSE);
  if (! stream) {
    fprintf(stderr, "ERROR -> test_tile_encoder: failed to create the stream !\n");
    opj_destroy_codec(codec);
    opj_image_destroy(image);
    return SCIL_UNKNOWN_ERR;
  }
  if (! opj_start_compress(codec,image,stream)) {
    fprintf(stderr, "ERROR -> test_tile_encoder: failed to start compress!\n");
    opj_stream_destroy(stream);
    opj_destroy_codec(codec);
    opj_image_destroy(image);
    return SCIL_UNKNOWN_ERR;
  }

  for (i=0;i<nb_tiles;++i) {
		if (! opj_write_tile(codec,i,data,data_size,stream)) {
			fprintf(stderr, "ERROR -> test_tile_encoder: failed to write the tile %d!\n",i);
      opj_stream_destroy(stream);
			opj_destroy_codec(codec);
			opj_image_destroy(image);
			return SCIL_UNKNOWN_ERR;
		}
	}

  if (! opj_end_compress(codec,stream)) {
		fprintf(stderr, "ERROR -> test_tile_encoder: failed to end compress!\n");
    opj_stream_destroy(stream);
		opj_destroy_codec(codec);
		opj_image_destroy(image);
		return SCIL_UNKNOWN_ERR;
	}

  opj_stream_destroy(stream);
	opj_destroy_codec(codec);
	opj_image_destroy(image);

	free(data);
  return SCIL_NO_ERR;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
int scil_opj_decompress(byte*restrict data_out, size_t buff_size,  const byte*restrict compressed_buf_in, const size_t in_size, size_t * uncomp_size_out)
{

				opj_dparameters_t param;
        opj_codec_t * codec;
        opj_image_t * image;
        opj_stream_t * stream;
        OPJ_UINT32 data_size = in_size;
        OPJ_UINT32 max_data_size = 1000;
        OPJ_UINT32 tile_index;
        OPJ_BYTE * data = (OPJ_BYTE *) malloc(1000);
        OPJ_BOOL go_on = OPJ_TRUE;
        OPJ_UINT32 nb_comps=0 ;
        OPJ_INT32 l_current_tile_x0,l_current_tile_y0,l_current_tile_x1,l_current_tile_y1;

        int da_x0=0;
        int da_y0=0;
        int da_x1=1000;
        int da_y1=1000;
				int i=0;


        if (! data) {
                return EXIT_FAILURE;
        }

				for (i=0;i<data_size;++i)	{
					data[i] = (OPJ_BYTE) compressed_buf_in[i];
				}

        stream = opj_stream_create(data_size, OPJ_TRUE);
        if (!stream){
                free(data);
                fprintf(stderr, "ERROR -> failed to create the stream\n");
                return EXIT_FAILURE;
        }

				opj_stream_set_user_data (stream, data, NULL);

        /* Set the default decoding parameters */
        opj_set_default_decoder_parameters(&param);

        /* */
        //param.decod_format = JP2_CFMT;

        /** you may here add custom decoding parameters */
        /* do not use layer decoding limitations */
        param.cp_layer = 0;

        /* do not use resolutions reductions */
        param.cp_reduce = 0;

        /* to decode only a part of the image data */
        /*opj_restrict_decoding(&param,0,0,1000,1000);*/

        /* Get a decoder handle */
        codec = opj_create_decompress(OPJ_CODEC_JP2);

        /* catch events using our callbacks and give a local context */
        opj_set_info_handler(codec, info_callback,00);
        opj_set_warning_handler(codec, warning_callback,00);
        opj_set_error_handler(codec, error_callback,00);

        /* Setup the decoder decoding parameters using user parameters */
        if (! opj_setup_decoder(codec, &param))
        {
                fprintf(stderr, "ERROR -> j2k_dump: failed to setup the decoder\n");
                free(data);
                opj_stream_destroy(stream);
                opj_destroy_codec(codec);
                return EXIT_FAILURE;
        }

        /* Read the main header of the codestream and if necessary the JP2 boxes*/
        if (! opj_read_header(stream, codec, &image))
        {
                fprintf(stderr, "ERROR -> j2k_to_image: failed to read the header\n");
                free(data);
                opj_stream_destroy(stream);
                opj_destroy_codec(codec);
                return EXIT_FAILURE;
        }

        if (!opj_set_decode_area(codec, image, da_x0, da_y0,da_x1, da_y1)){
                fprintf(stderr,	"ERROR -> j2k_to_image: failed to set the decoded area\n");
                free(data);
                opj_stream_destroy(stream);
                opj_destroy_codec(codec);
                opj_image_destroy(image);
                return EXIT_FAILURE;
        }


        while (go_on)
        {
                if (! opj_read_tile_header( codec,
                                        stream,
                                        &tile_index,
                                        &data_size,
                                        &l_current_tile_x0,
                                        &l_current_tile_y0,
                                        &l_current_tile_x1,
                                        &l_current_tile_y1,
                                        &nb_comps,
                                        &go_on))
                {
                        free(data);
                        opj_stream_destroy(stream);
                        opj_destroy_codec(codec);
                        opj_image_destroy(image);
                        return EXIT_FAILURE;
                }

                if (go_on)
                {
                        if (data_size > max_data_size)
                        {
                                OPJ_BYTE *l_new_data = (OPJ_BYTE *) realloc(data, data_size);
                                if (! l_new_data)
                                {
                                        free(l_new_data);
                                        opj_stream_destroy(stream);
                                        opj_destroy_codec(codec);
                                        opj_image_destroy(image);
                                        return EXIT_FAILURE;
                                }
                                data = l_new_data;
                                max_data_size = data_size;
                        }

                        if (! opj_decode_tile_data(codec,tile_index,data,data_size,stream))
                        {
                                free(data);
                                opj_stream_destroy(stream);
                                opj_destroy_codec(codec);
                                opj_image_destroy(image);
                                return EXIT_FAILURE;
                        }
                        /** now should inspect image to know the reduction factor and then how to behave with data */
                }
        }

        if (! opj_end_decompress(codec,stream))
        {
                free(data);
                opj_stream_destroy(stream);
                opj_destroy_codec(codec);
                opj_image_destroy(image);
                return EXIT_FAILURE;
        }

        /* Free memory */
        free(data);
        opj_stream_destroy(stream);
        opj_destroy_codec(codec);
        opj_image_destroy(image);
  return 0;
}

scil_compression_algorithm algo_opj = {
    .c.Btype = {
        scil_opj_compress,
        scil_opj_decompress
    },
    "opj",
    2,
    SCIL_COMPRESSOR_TYPE_INDIVIDUAL_BYTES
};

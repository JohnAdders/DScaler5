/*
 * Utils for libavcodec
 * Copyright (c) 2002 Fabrice Bellard.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/**
 * @file allcodecs.c
 * Utils for libavcodec.
 */

#include "avcodec.h"

/* If you do not call this function, then you can select exactly which
   formats you want to support */

/**
 * simple call to register all the codecs. 
 */
void avcodec_register_all(void)
{
    static int inited = 0;
    
    if (inited != 0)
	return;
    inited = 1;

    /* encoders */
    register_avcodec(&ac3_encoder);
#ifdef CONFIG_ENCODERS
    register_avcodec(&mpeg1video_encoder);
//    register_avcodec(&h264_encoder);
    register_avcodec(&mpeg2video_encoder);
    register_avcodec(&h263_encoder);
    register_avcodec(&h263p_encoder);
    register_avcodec(&flv_encoder);
    register_avcodec(&rv10_encoder);
    register_avcodec(&mpeg4_encoder);
    register_avcodec(&msmpeg4v1_encoder);
    register_avcodec(&msmpeg4v2_encoder);
    register_avcodec(&msmpeg4v3_encoder);
    register_avcodec(&wmv1_encoder);
    register_avcodec(&wmv2_encoder);
    register_avcodec(&mjpeg_encoder);
    register_avcodec(&ljpeg_encoder);
    register_avcodec(&huffyuv_encoder);
    register_avcodec(&asv1_encoder);
    register_avcodec(&asv2_encoder);
    register_avcodec(&ffv1_encoder);
#if __STDC_VERSION__ >= 199901L
    register_avcodec(&snow_encoder);
#endif
    register_avcodec(&dvvideo_encoder);
#endif
    /* decoders */
    register_avcodec(&h263_decoder);
    register_avcodec(&mpeg4_decoder);
    register_avcodec(&msmpeg4v1_decoder);
    register_avcodec(&msmpeg4v2_decoder);
    register_avcodec(&msmpeg4v3_decoder);
    register_avcodec(&wmv1_decoder);
    register_avcodec(&wmv2_decoder);
    register_avcodec(&h263i_decoder);
    register_avcodec(&flv_decoder);
    register_avcodec(&rv10_decoder);
    register_avcodec(&rv20_decoder);
    register_avcodec(&svq1_decoder);
    register_avcodec(&svq3_decoder);
    register_avcodec(&wmav1_decoder);
    register_avcodec(&wmav2_decoder);
    register_avcodec(&indeo3_decoder);
    register_avcodec(&mpeg1video_decoder);
    register_avcodec(&mpeg2video_decoder);
    register_avcodec(&mpegvideo_decoder);
    register_avcodec(&dvvideo_decoder);
    register_avcodec(&mjpeg_decoder);
    register_avcodec(&mjpegb_decoder);
    register_avcodec(&sp5x_decoder);
    register_avcodec(&huffyuv_decoder);
    register_avcodec(&ffv1_decoder);
#if __STDC_VERSION__ >= 199901L
    register_avcodec(&snow_decoder);
#endif
    register_avcodec(&cyuv_decoder);
    register_avcodec(&h264_decoder);
    register_avcodec(&vp3_decoder);
    register_avcodec(&theora_decoder);
    register_avcodec(&asv1_decoder);
    register_avcodec(&asv2_decoder);
    register_avcodec(&vcr1_decoder);
    register_avcodec(&cinepak_decoder);
    register_avcodec(&msrle_decoder);
    register_avcodec(&msvideo1_decoder);
    register_avcodec(&eightbps_decoder);
    register_avcodec(&truemotion1_decoder);
    register_avcodec(&mszh_decoder);
    register_avcodec(&zlib_decoder);
    register_avcodec(&corepng_decoder);
#ifdef CONFIG_AC3
    register_avcodec(&ac3_decoder);
#endif
    register_avcodec(&qtrle_decoder);
    register_avcodec(&flac_decoder);
    register_avcodec(&tscc_decoder);
    register_avcodec(&amr_nb_decoder);
    register_avcodec(&msgsm_decoder);

    /* pcm codecs */

#define PCM_CODEC(id, name) \
    register_avcodec(& name ## _decoder);

    PCM_CODEC(CODEC_ID_PCM_ALAW, pcm_alaw);
    PCM_CODEC(CODEC_ID_PCM_MULAW, pcm_mulaw);

    /* adpcm codecs */
    PCM_CODEC(CODEC_ID_ADPCM_IMA_QT, adpcm_ima_qt);
    PCM_CODEC(CODEC_ID_ADPCM_IMA_WAV, adpcm_ima_wav);
    PCM_CODEC(CODEC_ID_ADPCM_IMA_DK3, adpcm_ima_dk3);
    PCM_CODEC(CODEC_ID_ADPCM_IMA_DK4, adpcm_ima_dk4);
    PCM_CODEC(CODEC_ID_ADPCM_IMA_WS, adpcm_ima_ws);
    PCM_CODEC(CODEC_ID_ADPCM_IMA_SMJPEG, adpcm_ima_smjpeg);
    PCM_CODEC(CODEC_ID_ADPCM_MS, adpcm_ms);
    PCM_CODEC(CODEC_ID_ADPCM_4XM, adpcm_4xm);
    PCM_CODEC(CODEC_ID_ADPCM_XA, adpcm_xa);
    PCM_CODEC(CODEC_ID_ADPCM_ADX, adpcm_adx);
    PCM_CODEC(CODEC_ID_ADPCM_EA, adpcm_ea);
    //PCM_CODEC(CODEC_ID_ADPCM_G726, adpcm_g726);
    PCM_CODEC(CODEC_ID_ADPCM_CT, adpcm_ct);

#undef PCM_CODEC

    /* parsers */ 
    av_register_codec_parser(&mpegvideo_parser);
    av_register_codec_parser(&mpeg4video_parser);
    av_register_codec_parser(&h263_parser);
    av_register_codec_parser(&h264_parser);
    av_register_codec_parser(&mjpeg_parser);

#ifdef CONFIG_AC3
    av_register_codec_parser(&ac3_parser);
#endif
}


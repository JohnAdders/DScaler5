/*
 * Misc image convertion routines
 * Copyright (c) 2001, 2002, 2003 Fabrice Bellard.
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
 * @file imgconvert.c
 * Misc image convertion routines.
 */

/* TODO:
 * - write 'ffimg' program to test all the image related stuff
 * - move all api to slice based system
 * - integrate deinterlacing, postprocessing and scaling in the conversion process
 */

#include "avcodec.h"
#include "dsputil.h"

#ifdef USE_FASTMEMCPY
#include "fastmemcpy.h"
#endif

#ifdef HAVE_MMX
#include "i386/mmx.h"
#endif

#define xglue(x, y) x ## y
#define glue(x, y) xglue(x, y)

#define FF_COLOR_RGB      0 /* RGB color space */
#define FF_COLOR_GRAY     1 /* gray color space */
#define FF_COLOR_YUV      2 /* YUV color space. 16 <= Y <= 235, 16 <= U, V <= 240 */
#define FF_COLOR_YUV_JPEG 3 /* YUV color space. 0 <= Y <= 255, 0 <= U, V <= 255 */

#define FF_PIXEL_PLANAR   0 /* each channel has one component in AVPicture */
#define FF_PIXEL_PACKED   1 /* only one components containing all the channels */
#define FF_PIXEL_PALETTE  2  /* one components containing indexes for a palette */

typedef struct PixFmtInfo {
    const char *name;
    uint8_t nb_channels;     /* number of channels (including alpha) */
    uint8_t color_type;      /* color type (see FF_COLOR_xxx constants) */
    uint8_t pixel_type;      /* pixel storage type (see FF_PIXEL_xxx constants) */
    uint8_t is_alpha : 1;    /* true if alpha can be specified */
    uint8_t x_chroma_shift; /* X chroma subsampling factor is 2 ^ shift */
    uint8_t y_chroma_shift; /* Y chroma subsampling factor is 2 ^ shift */
    uint8_t depth;           /* bit depth of the color components */
} PixFmtInfo;

/* this table gives more information about formats */
static PixFmtInfo pix_fmt_info[PIX_FMT_NB] 
#if __STDC_VERSION__ >= 199901L
= {
    /* YUV formats */
    [PIX_FMT_YUV420P] = {
        .name = "yuv420p",
        .nb_channels = 3,
        .color_type = FF_COLOR_YUV,
        .pixel_type = FF_PIXEL_PLANAR,
        .depth = 8,
        .x_chroma_shift = 1, .y_chroma_shift = 1, 
    },
    [PIX_FMT_YUV422P] = {
        .name = "yuv422p",
        .nb_channels = 3,
        .color_type = FF_COLOR_YUV,
        .pixel_type = FF_PIXEL_PLANAR,
        .depth = 8,
        .x_chroma_shift = 1, .y_chroma_shift = 0, 
    },
    [PIX_FMT_YUV444P] = {
        .name = "yuv444p",
        .nb_channels = 3,
        .color_type = FF_COLOR_YUV,
        .pixel_type = FF_PIXEL_PLANAR,
        .depth = 8,
        .x_chroma_shift = 0, .y_chroma_shift = 0, 
    },
    [PIX_FMT_YUV422] = {
        .name = "yuv422",
        .nb_channels = 1,
        .color_type = FF_COLOR_YUV,
        .pixel_type = FF_PIXEL_PACKED,
        .depth = 8,
        .x_chroma_shift = 1, .y_chroma_shift = 0,
    },
    [PIX_FMT_UYVY422] = {
        .name = "uyvy422",
        .nb_channels = 1,
        .color_type = FF_COLOR_YUV,
        .pixel_type = FF_PIXEL_PACKED,
        .depth = 8,
        .x_chroma_shift = 1, .y_chroma_shift = 0,
    },
    [PIX_FMT_YUV410P] = {
        .name = "yuv410p",
        .nb_channels = 3,
        .color_type = FF_COLOR_YUV,
        .pixel_type = FF_PIXEL_PLANAR,
        .depth = 8,
        .x_chroma_shift = 2, .y_chroma_shift = 2,
    },
    [PIX_FMT_YUV411P] = {
        .name = "yuv411p",
        .nb_channels = 3,
        .color_type = FF_COLOR_YUV,
        .pixel_type = FF_PIXEL_PLANAR,
        .depth = 8,
        .x_chroma_shift = 2, .y_chroma_shift = 0,
    },

    /* JPEG YUV */
    [PIX_FMT_YUVJ420P] = {
        .name = "yuvj420p",
        .nb_channels = 3,
        .color_type = FF_COLOR_YUV_JPEG,
        .pixel_type = FF_PIXEL_PLANAR,
        .depth = 8,
        .x_chroma_shift = 1, .y_chroma_shift = 1, 
    },
    [PIX_FMT_YUVJ422P] = {
        .name = "yuvj422p",
        .nb_channels = 3,
        .color_type = FF_COLOR_YUV_JPEG,
        .pixel_type = FF_PIXEL_PLANAR,
        .depth = 8,
        .x_chroma_shift = 1, .y_chroma_shift = 0, 
    },
    [PIX_FMT_YUVJ444P] = {
        .name = "yuvj444p",
        .nb_channels = 3,
        .color_type = FF_COLOR_YUV_JPEG,
        .pixel_type = FF_PIXEL_PLANAR,
        .depth = 8,
        .x_chroma_shift = 0, .y_chroma_shift = 0, 
    },

    /* RGB formats */
    [PIX_FMT_RGB24] = {
        .name = "rgb24",
        .nb_channels = 3,
        .color_type = FF_COLOR_RGB,
        .pixel_type = FF_PIXEL_PACKED,
        .depth = 8,
        .x_chroma_shift = 0, .y_chroma_shift = 0,
    },
    [PIX_FMT_BGR24] = {
        .name = "bgr24",
        .nb_channels = 3,
        .color_type = FF_COLOR_RGB,
        .pixel_type = FF_PIXEL_PACKED,
        .depth = 8,
        .x_chroma_shift = 0, .y_chroma_shift = 0,
    },
    [PIX_FMT_RGBA32] = {
        .name = "rgba32",
        .nb_channels = 4, .is_alpha = 1,
        .color_type = FF_COLOR_RGB,
        .pixel_type = FF_PIXEL_PACKED,
        .depth = 8,
        .x_chroma_shift = 0, .y_chroma_shift = 0,
    },
    [PIX_FMT_RGB565] = {
        .name = "rgb565",
        .nb_channels = 3,
        .color_type = FF_COLOR_RGB,
        .pixel_type = FF_PIXEL_PACKED,
        .depth = 5,
        .x_chroma_shift = 0, .y_chroma_shift = 0,
    },
    [PIX_FMT_RGB555] = {
        .name = "rgb555",
        .nb_channels = 4, .is_alpha = 1,
        .color_type = FF_COLOR_RGB,
        .pixel_type = FF_PIXEL_PACKED,
        .depth = 5,
        .x_chroma_shift = 0, .y_chroma_shift = 0,
    },

    /* gray / mono formats */
    [PIX_FMT_GRAY8] = {
        .name = "gray",
        .nb_channels = 1,
        .color_type = FF_COLOR_GRAY,
        .pixel_type = FF_PIXEL_PLANAR,
        .depth = 8,
    },
    [PIX_FMT_MONOWHITE] = {
        .name = "monow",
        .nb_channels = 1,
        .color_type = FF_COLOR_GRAY,
        .pixel_type = FF_PIXEL_PLANAR,
        .depth = 1,
    },
    [PIX_FMT_MONOBLACK] = {
        .name = "monob",
        .nb_channels = 1,
        .color_type = FF_COLOR_GRAY,
        .pixel_type = FF_PIXEL_PLANAR,
        .depth = 1,
    },

    /* paletted formats */
    [PIX_FMT_PAL8] = {
        .name = "pal8",
        .nb_channels = 4, .is_alpha = 1,
        .color_type = FF_COLOR_RGB,
        .pixel_type = FF_PIXEL_PALETTE,
        .depth = 8,
    },
    [PIX_FMT_XVMC_MPEG2_MC] = {
        .name = "xvmcmc",
    },
    [PIX_FMT_XVMC_MPEG2_IDCT] = {
        .name = "xvmcidct",
    },
    [PIX_FMT_UYVY411] = {
        .name = "uyvy411",
        .nb_channels = 1,
        .color_type = FF_COLOR_YUV,
        .pixel_type = FF_PIXEL_PACKED,
        .depth = 8,
        .x_chroma_shift = 2, .y_chroma_shift = 0,
    },
};

#else
;
void avpicture_init_pixfmtinfo(void)
{
 pix_fmt_info[PIX_FMT_YUV420P].name = "yuv420p";
 pix_fmt_info[PIX_FMT_YUV420P].nb_channels = 3;
 pix_fmt_info[PIX_FMT_YUV420P].color_type = FF_COLOR_YUV;
 pix_fmt_info[PIX_FMT_YUV420P].pixel_type = FF_PIXEL_PLANAR;
 pix_fmt_info[PIX_FMT_YUV420P].depth = 8;
 pix_fmt_info[PIX_FMT_YUV420P].x_chroma_shift = 1;
 pix_fmt_info[PIX_FMT_YUV420P].y_chroma_shift = 1; 

 pix_fmt_info[PIX_FMT_YUV422P].name = "yuv422p";
 pix_fmt_info[PIX_FMT_YUV422P].nb_channels = 3;
 pix_fmt_info[PIX_FMT_YUV422P].color_type = FF_COLOR_YUV;
 pix_fmt_info[PIX_FMT_YUV422P].pixel_type = FF_PIXEL_PLANAR;
 pix_fmt_info[PIX_FMT_YUV422P].depth = 8,
 pix_fmt_info[PIX_FMT_YUV422P].x_chroma_shift = 1;
 pix_fmt_info[PIX_FMT_YUV422P].y_chroma_shift = 0;

 pix_fmt_info[PIX_FMT_YUV444P].name = "yuv444p";
 pix_fmt_info[PIX_FMT_YUV444P].nb_channels = 3;
 pix_fmt_info[PIX_FMT_YUV444P].color_type = FF_COLOR_YUV;
 pix_fmt_info[PIX_FMT_YUV444P].pixel_type = FF_PIXEL_PLANAR;
 pix_fmt_info[PIX_FMT_YUV444P].depth = 8;
 pix_fmt_info[PIX_FMT_YUV444P].x_chroma_shift = 0;
 pix_fmt_info[PIX_FMT_YUV444P].y_chroma_shift = 0;

 pix_fmt_info[PIX_FMT_YUV422].name = "yuv422";
 pix_fmt_info[PIX_FMT_YUV422].nb_channels = 1;
 pix_fmt_info[PIX_FMT_YUV422].color_type = FF_COLOR_YUV;
 pix_fmt_info[PIX_FMT_YUV422].pixel_type = FF_PIXEL_PACKED;
 pix_fmt_info[PIX_FMT_YUV422].depth = 8;
 pix_fmt_info[PIX_FMT_YUV422].x_chroma_shift = 1;
 pix_fmt_info[PIX_FMT_YUV422].y_chroma_shift = 0;

 pix_fmt_info[PIX_FMT_UYVY422].name = "uyvy422";
 pix_fmt_info[PIX_FMT_UYVY422].nb_channels = 1;
 pix_fmt_info[PIX_FMT_UYVY422].color_type = FF_COLOR_YUV;
 pix_fmt_info[PIX_FMT_UYVY422].pixel_type = FF_PIXEL_PACKED;
 pix_fmt_info[PIX_FMT_UYVY422].depth = 8;
 pix_fmt_info[PIX_FMT_UYVY422].x_chroma_shift = 1;
 pix_fmt_info[PIX_FMT_UYVY422].y_chroma_shift = 0;

 pix_fmt_info[PIX_FMT_YUV410P].name = "yuv410p";
 pix_fmt_info[PIX_FMT_YUV410P].nb_channels = 3;
 pix_fmt_info[PIX_FMT_YUV410P].color_type = FF_COLOR_YUV;
 pix_fmt_info[PIX_FMT_YUV410P].pixel_type = FF_PIXEL_PLANAR;
 pix_fmt_info[PIX_FMT_YUV410P].depth = 8;
 pix_fmt_info[PIX_FMT_YUV410P].x_chroma_shift = 2;
 pix_fmt_info[PIX_FMT_YUV410P].y_chroma_shift = 2;

 pix_fmt_info[PIX_FMT_YUV411P].name = "yuv411p";
 pix_fmt_info[PIX_FMT_YUV411P].nb_channels = 3;
 pix_fmt_info[PIX_FMT_YUV411P].color_type = FF_COLOR_YUV;
 pix_fmt_info[PIX_FMT_YUV411P].pixel_type = FF_PIXEL_PLANAR;
 pix_fmt_info[PIX_FMT_YUV411P].depth = 8;
 pix_fmt_info[PIX_FMT_YUV411P].x_chroma_shift = 2;
 pix_fmt_info[PIX_FMT_YUV411P].y_chroma_shift = 0;

 /* JPEG YUV */
 pix_fmt_info[PIX_FMT_YUVJ420P].name = "yuvj420p";
 pix_fmt_info[PIX_FMT_YUVJ420P].nb_channels = 3;
 pix_fmt_info[PIX_FMT_YUVJ420P].color_type = FF_COLOR_YUV_JPEG;
 pix_fmt_info[PIX_FMT_YUVJ420P].pixel_type = FF_PIXEL_PLANAR;
 pix_fmt_info[PIX_FMT_YUVJ420P].depth = 8;
 pix_fmt_info[PIX_FMT_YUVJ420P].x_chroma_shift = 1;  
 pix_fmt_info[PIX_FMT_YUVJ420P].y_chroma_shift = 1;

 pix_fmt_info[PIX_FMT_YUVJ422P].name = "yuvj422p";
 pix_fmt_info[PIX_FMT_YUVJ422P].nb_channels = 3;
 pix_fmt_info[PIX_FMT_YUVJ422P].color_type = FF_COLOR_YUV_JPEG;
 pix_fmt_info[PIX_FMT_YUVJ422P].pixel_type = FF_PIXEL_PLANAR;
 pix_fmt_info[PIX_FMT_YUVJ422P].depth = 8;
 pix_fmt_info[PIX_FMT_YUVJ422P].x_chroma_shift = 1;
 pix_fmt_info[PIX_FMT_YUVJ422P].y_chroma_shift = 0; 

 pix_fmt_info[PIX_FMT_YUVJ444P].name = "yuvj444p";
 pix_fmt_info[PIX_FMT_YUVJ444P].nb_channels = 3;
 pix_fmt_info[PIX_FMT_YUVJ444P].color_type = FF_COLOR_YUV_JPEG;
 pix_fmt_info[PIX_FMT_YUVJ444P].pixel_type = FF_PIXEL_PLANAR;
 pix_fmt_info[PIX_FMT_YUVJ444P].depth = 8;
 pix_fmt_info[PIX_FMT_YUVJ444P].x_chroma_shift = 0;
 pix_fmt_info[PIX_FMT_YUVJ444P].y_chroma_shift = 0;

 /* RGB formats */
 pix_fmt_info[PIX_FMT_RGB24].name = "rgb24";
 pix_fmt_info[PIX_FMT_RGB24].nb_channels = 3;
 pix_fmt_info[PIX_FMT_RGB24].color_type = FF_COLOR_RGB;
 pix_fmt_info[PIX_FMT_RGB24].pixel_type = FF_PIXEL_PACKED;
 pix_fmt_info[PIX_FMT_RGB24].depth = 8;
 pix_fmt_info[PIX_FMT_RGB24].x_chroma_shift = 0;
 pix_fmt_info[PIX_FMT_RGB24].y_chroma_shift = 0;

 pix_fmt_info[PIX_FMT_BGR24].name = "bgr24";
 pix_fmt_info[PIX_FMT_BGR24].nb_channels = 3;
 pix_fmt_info[PIX_FMT_BGR24].color_type = FF_COLOR_RGB;
 pix_fmt_info[PIX_FMT_BGR24].pixel_type = FF_PIXEL_PACKED;
 pix_fmt_info[PIX_FMT_BGR24].depth = 8;
 pix_fmt_info[PIX_FMT_BGR24].x_chroma_shift = 0;
 pix_fmt_info[PIX_FMT_BGR24].y_chroma_shift = 0;

 pix_fmt_info[PIX_FMT_RGBA32].name = "rgba32";
 pix_fmt_info[PIX_FMT_RGBA32].nb_channels = 4;
 pix_fmt_info[PIX_FMT_RGBA32].is_alpha = 1;
 pix_fmt_info[PIX_FMT_RGBA32].color_type = FF_COLOR_RGB;
 pix_fmt_info[PIX_FMT_RGBA32].pixel_type = FF_PIXEL_PACKED;
 pix_fmt_info[PIX_FMT_RGBA32].depth = 8;
 pix_fmt_info[PIX_FMT_RGBA32].x_chroma_shift = 0;
 pix_fmt_info[PIX_FMT_RGBA32].y_chroma_shift = 0;

 pix_fmt_info[PIX_FMT_RGB565].name = "rgb565";
 pix_fmt_info[PIX_FMT_RGB565].nb_channels = 3;
 pix_fmt_info[PIX_FMT_RGB565].color_type = FF_COLOR_RGB;
 pix_fmt_info[PIX_FMT_RGB565].pixel_type = FF_PIXEL_PACKED;
 pix_fmt_info[PIX_FMT_RGB565].depth = 5;
 pix_fmt_info[PIX_FMT_RGB565].x_chroma_shift = 0;
 pix_fmt_info[PIX_FMT_RGB565].y_chroma_shift = 0;

 pix_fmt_info[PIX_FMT_RGB555].name = "rgb555";
 pix_fmt_info[PIX_FMT_RGB555].nb_channels = 4;
 pix_fmt_info[PIX_FMT_RGB555].is_alpha = 1;
 pix_fmt_info[PIX_FMT_RGB555].color_type = FF_COLOR_RGB;
 pix_fmt_info[PIX_FMT_RGB555].pixel_type = FF_PIXEL_PACKED;
 pix_fmt_info[PIX_FMT_RGB555].depth = 5;
 pix_fmt_info[PIX_FMT_RGB555].x_chroma_shift = 0;
 pix_fmt_info[PIX_FMT_RGB555].y_chroma_shift = 0;
 
 /* gray / mono formats */
 pix_fmt_info[PIX_FMT_GRAY8].name = "gray";
 pix_fmt_info[PIX_FMT_GRAY8].nb_channels = 1;
 pix_fmt_info[PIX_FMT_GRAY8].color_type = FF_COLOR_GRAY;
 pix_fmt_info[PIX_FMT_GRAY8].pixel_type = FF_PIXEL_PLANAR;
 pix_fmt_info[PIX_FMT_GRAY8].depth = 8;

 pix_fmt_info[PIX_FMT_MONOWHITE].name = "monow";
 pix_fmt_info[PIX_FMT_MONOWHITE].nb_channels = 1;
 pix_fmt_info[PIX_FMT_MONOWHITE].color_type = FF_COLOR_GRAY;
 pix_fmt_info[PIX_FMT_MONOWHITE].pixel_type = FF_PIXEL_PLANAR;
 pix_fmt_info[PIX_FMT_MONOWHITE].depth = 1;

 pix_fmt_info[PIX_FMT_MONOBLACK].name = "monob";
 pix_fmt_info[PIX_FMT_MONOBLACK].nb_channels = 1;
 pix_fmt_info[PIX_FMT_MONOBLACK].color_type = FF_COLOR_GRAY;
 pix_fmt_info[PIX_FMT_MONOBLACK].pixel_type = FF_PIXEL_PLANAR;
 pix_fmt_info[PIX_FMT_MONOBLACK].depth = 1;

 /* paletted formats */
 pix_fmt_info[PIX_FMT_PAL8].name = "pal8";
 pix_fmt_info[PIX_FMT_PAL8].nb_channels = 4; 
 pix_fmt_info[PIX_FMT_PAL8].is_alpha = 1;
 pix_fmt_info[PIX_FMT_PAL8].color_type = FF_COLOR_RGB;
 pix_fmt_info[PIX_FMT_PAL8].pixel_type = FF_PIXEL_PALETTE;
 pix_fmt_info[PIX_FMT_PAL8].depth = 8;

 pix_fmt_info[PIX_FMT_UYVY411].name = "uyvy411";
 pix_fmt_info[PIX_FMT_UYVY411].nb_channels = 1;
 pix_fmt_info[PIX_FMT_UYVY411].color_type = FF_COLOR_YUV;
 pix_fmt_info[PIX_FMT_UYVY411].pixel_type = FF_PIXEL_PACKED;
 pix_fmt_info[PIX_FMT_UYVY411].depth = 8;
 pix_fmt_info[PIX_FMT_UYVY411].x_chroma_shift = 2;
 pix_fmt_info[PIX_FMT_UYVY411].y_chroma_shift = 0;
}
#endif

void avcodec_get_chroma_sub_sample(int pix_fmt, int *h_shift, int *v_shift)
{
        *h_shift = pix_fmt_info[pix_fmt].x_chroma_shift;
        *v_shift = pix_fmt_info[pix_fmt].y_chroma_shift;
}    

const char *avcodec_get_pix_fmt_name(int pix_fmt)
{
    if (pix_fmt < 0 || pix_fmt >= PIX_FMT_NB)
        return "???";
    else
        return pix_fmt_info[pix_fmt].name;
}

enum PixelFormat avcodec_get_pix_fmt(const char* name)
{
    int i; 
    
    for (i=0; i < PIX_FMT_NB; i++)
         if (!strcmp(pix_fmt_info[i].name, name))
	     break;
    return i;
}

/* Picture field are filled with 'ptr' addresses. Also return size */
int avpicture_fill(AVPicture *picture, uint8_t *ptr,
		   int pix_fmt, int width, int height)
{
    int size, w2, h2, size2;
    PixFmtInfo *pinfo;
    
    pinfo = &pix_fmt_info[pix_fmt];
    size = width * height;
    switch(pix_fmt) {
    case PIX_FMT_YUV420P:
    case PIX_FMT_YUV422P:
    case PIX_FMT_YUV444P:
    case PIX_FMT_YUV410P:
    case PIX_FMT_YUV411P:
    case PIX_FMT_YUVJ420P:
    case PIX_FMT_YUVJ422P:
    case PIX_FMT_YUVJ444P:
        w2 = (width + (1 << pinfo->x_chroma_shift) - 1) >> pinfo->x_chroma_shift;
        h2 = (height + (1 << pinfo->y_chroma_shift) - 1) >> pinfo->y_chroma_shift;
        size2 = w2 * h2;
        picture->data[0] = ptr;
        picture->data[1] = picture->data[0] + size;
        picture->data[2] = picture->data[1] + size2;
        picture->linesize[0] = width;
        picture->linesize[1] = w2;
        picture->linesize[2] = w2;
        return size + 2 * size2;
    case PIX_FMT_RGB24:
    case PIX_FMT_BGR24:
        picture->data[0] = ptr;
        picture->data[1] = NULL;
        picture->data[2] = NULL;
        picture->linesize[0] = width * 3;
        return size * 3;
    case PIX_FMT_RGBA32:
        picture->data[0] = ptr;
        picture->data[1] = NULL;
        picture->data[2] = NULL;
        picture->linesize[0] = width * 4;
        return size * 4;
    case PIX_FMT_RGB555:
    case PIX_FMT_RGB565:
    case PIX_FMT_YUV422:
        picture->data[0] = ptr;
        picture->data[1] = NULL;
        picture->data[2] = NULL;
        picture->linesize[0] = width * 2;
        return size * 2;
    case PIX_FMT_UYVY422:
        picture->data[0] = ptr;
        picture->data[1] = NULL;
        picture->data[2] = NULL;
        picture->linesize[0] = width * 2;
        return size * 2;
    case PIX_FMT_UYVY411:
        picture->data[0] = ptr;
        picture->data[1] = NULL;
        picture->data[2] = NULL;
        picture->linesize[0] = width + width/2;
        return size + size/2;
    case PIX_FMT_GRAY8:
        picture->data[0] = ptr;
        picture->data[1] = NULL;
        picture->data[2] = NULL;
        picture->linesize[0] = width;
        return size;
    case PIX_FMT_MONOWHITE:
    case PIX_FMT_MONOBLACK:
        picture->data[0] = ptr;
        picture->data[1] = NULL;
        picture->data[2] = NULL;
        picture->linesize[0] = (width + 7) >> 3;
        return picture->linesize[0] * height;
    case PIX_FMT_PAL8:
        size2 = (size + 3) & ~3;
        picture->data[0] = ptr;
        picture->data[1] = ptr + size2; /* palette is stored here as 256 32 bit words */
        picture->data[2] = NULL;
        picture->linesize[0] = width;
        picture->linesize[1] = 4;
        return size2 + 256 * 4;
    default:
        picture->data[0] = NULL;
        picture->data[1] = NULL;
        picture->data[2] = NULL;
        picture->data[3] = NULL;
        return -1;
    }
}

int avpicture_layout(const AVPicture* src, int pix_fmt, int width, int height,
                     unsigned char *dest, int dest_size)
{
    PixFmtInfo* pf = &pix_fmt_info[pix_fmt];
    int i, j, w, h, data_planes;
    const unsigned char* s; 
    int size = avpicture_get_size(pix_fmt, width, height);

    if (size > dest_size)
        return -1;

    if (pf->pixel_type == FF_PIXEL_PACKED || pf->pixel_type == FF_PIXEL_PALETTE) {
        if (pix_fmt == PIX_FMT_YUV422 || 
            pix_fmt == PIX_FMT_UYVY422 || 
            pix_fmt == PIX_FMT_RGB565 ||
	    pix_fmt == PIX_FMT_RGB555)
	  w = width * 2;
	else if (pix_fmt == PIX_FMT_UYVY411)
	  w = width + width/2;
	else if (pix_fmt == PIX_FMT_PAL8)
	  w = width;
	else
	  w = width * (pf->depth * pf->nb_channels / 8);
	  
	data_planes = 1;
	h = height;
    } else {
        data_planes = pf->nb_channels;
	w = (width*pf->depth + 7)/8;
	h = height;
    }
    
    for (i=0; i<data_planes; i++) {
         if (i == 1) {
	     w = width >> pf->x_chroma_shift;
	     h = height >> pf->y_chroma_shift;
	 }
         s = src->data[i];
	 for(j=0; j<h; j++) {
	     memcpy(dest, s, w);
	     dest += w;
	     s += src->linesize[i];
	 }
    }
    
    if (pf->pixel_type == FF_PIXEL_PALETTE)
	memcpy((unsigned char *)(((size_t)dest + 3) & ~3), src->data[1], 256 * 4);
    
    return size;
}

int avpicture_get_size(int pix_fmt, int width, int height)
{
    AVPicture dummy_pict;
    return avpicture_fill(&dummy_pict, NULL, pix_fmt, width, height);
}

/**
 * compute the loss when converting from a pixel format to another 
 */
int avcodec_get_pix_fmt_loss(int dst_pix_fmt, int src_pix_fmt,
                             int has_alpha)
{
    const PixFmtInfo *pf, *ps;
    int loss;

    ps = &pix_fmt_info[src_pix_fmt];
    pf = &pix_fmt_info[dst_pix_fmt];

    /* compute loss */
    loss = 0;
    pf = &pix_fmt_info[dst_pix_fmt];
    if (pf->depth < ps->depth ||
        (dst_pix_fmt == PIX_FMT_RGB555 && src_pix_fmt == PIX_FMT_RGB565))
        loss |= FF_LOSS_DEPTH;
    if (pf->x_chroma_shift > ps->x_chroma_shift ||
        pf->y_chroma_shift > ps->y_chroma_shift)
        loss |= FF_LOSS_RESOLUTION;
    switch(pf->color_type) {
    case FF_COLOR_RGB:
        if (ps->color_type != FF_COLOR_RGB &&
            ps->color_type != FF_COLOR_GRAY)
            loss |= FF_LOSS_COLORSPACE;
        break;
    case FF_COLOR_GRAY:
        if (ps->color_type != FF_COLOR_GRAY)
            loss |= FF_LOSS_COLORSPACE;
        break;
    case FF_COLOR_YUV:
        if (ps->color_type != FF_COLOR_YUV)
            loss |= FF_LOSS_COLORSPACE;
        break;
    case FF_COLOR_YUV_JPEG:
        if (ps->color_type != FF_COLOR_YUV_JPEG &&
            ps->color_type != FF_COLOR_YUV && 
            ps->color_type != FF_COLOR_GRAY)
            loss |= FF_LOSS_COLORSPACE;
        break;
    default:
        /* fail safe test */
        if (ps->color_type != pf->color_type)
            loss |= FF_LOSS_COLORSPACE;
        break;
    }
    if (pf->color_type == FF_COLOR_GRAY &&
        ps->color_type != FF_COLOR_GRAY)
        loss |= FF_LOSS_CHROMA;
    if (!pf->is_alpha && (ps->is_alpha && has_alpha))
        loss |= FF_LOSS_ALPHA;
    if (pf->pixel_type == FF_PIXEL_PALETTE && 
        (ps->pixel_type != FF_PIXEL_PALETTE && ps->color_type != FF_COLOR_GRAY))
        loss |= FF_LOSS_COLORQUANT;
    return loss;
}

static int avg_bits_per_pixel(int pix_fmt)
{
    int bits;
    const PixFmtInfo *pf;

    pf = &pix_fmt_info[pix_fmt];
    switch(pf->pixel_type) {
    case FF_PIXEL_PACKED:
        switch(pix_fmt) {
        case PIX_FMT_YUV422:
        case PIX_FMT_UYVY422:
        case PIX_FMT_RGB565:
        case PIX_FMT_RGB555:
            bits = 16;
            break;
	case PIX_FMT_UYVY411:
	    bits = 12;
	    break;
        default:
            bits = pf->depth * pf->nb_channels;
            break;
        }
        break;
    case FF_PIXEL_PLANAR:
        if (pf->x_chroma_shift == 0 && pf->y_chroma_shift == 0) {
            bits = pf->depth * pf->nb_channels;
        } else {
            bits = pf->depth + ((2 * pf->depth) >> 
                                (pf->x_chroma_shift + pf->y_chroma_shift));
        }
        break;
    case FF_PIXEL_PALETTE:
        bits = 8;
        break;
    default:
        bits = -1;
        break;
    }
    return bits;
}

static int avcodec_find_best_pix_fmt1(int pix_fmt_mask, 
                                      int src_pix_fmt,
                                      int has_alpha,
                                      int loss_mask)
{
    int dist, i, loss, min_dist, dst_pix_fmt;

    /* find exact color match with smallest size */
    dst_pix_fmt = -1;
    min_dist = 0x7fffffff;
    for(i = 0;i < PIX_FMT_NB; i++) {
        if (pix_fmt_mask & (1 << i)) {
            loss = avcodec_get_pix_fmt_loss(i, src_pix_fmt, has_alpha) & loss_mask;
            if (loss == 0) {
                dist = avg_bits_per_pixel(i);
                if (dist < min_dist) {
                    min_dist = dist;
                    dst_pix_fmt = i;
                }
            }
        }
    }
    return dst_pix_fmt;
}

/** 
 * find best pixel format to convert to. Return -1 if none found 
 */
int avcodec_find_best_pix_fmt(int pix_fmt_mask, int src_pix_fmt,
                              int has_alpha, int *loss_ptr)
{
    int dst_pix_fmt, loss_mask, i;
    static const int loss_mask_order[] = {
        ~0, /* no loss first */
        ~FF_LOSS_ALPHA,
        ~FF_LOSS_RESOLUTION,
        ~(FF_LOSS_COLORSPACE | FF_LOSS_RESOLUTION),
        ~FF_LOSS_COLORQUANT,
        ~FF_LOSS_DEPTH,
        0,
    };

    /* try with successive loss */
    i = 0;
    for(;;) {
        loss_mask = loss_mask_order[i++];
        dst_pix_fmt = avcodec_find_best_pix_fmt1(pix_fmt_mask, src_pix_fmt, 
                                                 has_alpha, loss_mask);
        if (dst_pix_fmt >= 0)
            goto found;
        if (loss_mask == 0)
            break;
    }
    return -1;
 found:
    if (loss_ptr)
        *loss_ptr = avcodec_get_pix_fmt_loss(dst_pix_fmt, src_pix_fmt, has_alpha);
    return dst_pix_fmt;
}

static void img_copy_plane(uint8_t *dst, int dst_wrap, 
                           const uint8_t *src, int src_wrap,
                           int width, int height)
{
    for(;height > 0; height--) {
        memcpy(dst, src, width);
        dst += dst_wrap;
        src += src_wrap;
    }
}

/**
 * Copy image 'src' to 'dst'.
 */
void img_copy(AVPicture *dst, const AVPicture *src,
              int pix_fmt, int width, int height)
{
    int bwidth, bits, i;
    PixFmtInfo *pf = &pix_fmt_info[pix_fmt];
    
    pf = &pix_fmt_info[pix_fmt];
    switch(pf->pixel_type) {
    case FF_PIXEL_PACKED:
        switch(pix_fmt) {
        case PIX_FMT_YUV422:
        case PIX_FMT_UYVY422:
        case PIX_FMT_RGB565:
        case PIX_FMT_RGB555:
            bits = 16;
            break;
	case PIX_FMT_UYVY411:
	    bits = 12;
	    break;
        default:
            bits = pf->depth * pf->nb_channels;
            break;
        }
        bwidth = (width * bits + 7) >> 3;
        img_copy_plane(dst->data[0], dst->linesize[0],
                       src->data[0], src->linesize[0],
                       bwidth, height);
        break;
    case FF_PIXEL_PLANAR:
        for(i = 0; i < pf->nb_channels; i++) {
            int w, h;
            w = width;
            h = height;
            if (i == 1 || i == 2) {
                w >>= pf->x_chroma_shift;
                h >>= pf->y_chroma_shift;
            }
            bwidth = (w * pf->depth + 7) >> 3;
            img_copy_plane(dst->data[i], dst->linesize[i],
                           src->data[i], src->linesize[i],
                           bwidth, h);
        }
        break;
    case FF_PIXEL_PALETTE:
        img_copy_plane(dst->data[0], dst->linesize[0],
                       src->data[0], src->linesize[0],
                       width, height);
        /* copy the palette */
        img_copy_plane(dst->data[1], dst->linesize[1],
                       src->data[1], src->linesize[1],
                       4, 256);
        break;
    }
}

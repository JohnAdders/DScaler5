/*
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
 
 
#include "avcodec.h"
#include "common.h"
#include "dsputil.h"
#include "../png/png.h"

#define PNGFrameType_RGB24 0x01
#define PNGFrameType_YUY2  0x02
#define PNGFrameType_YV12  0x03

typedef struct CorePNGCodecPrivate {
    int16_t wSize;
    int8_t bType;
} CorePNGCodecPrivate;

typedef struct CorePNGcontext{
    AVCodecContext *avctx;
    DSPContext dsp;
    AVFrame picture,prev_picture;
    CorePNGCodecPrivate private;
    png_struct png_struct1, *png_ptr;
    png_info png_info1, *info_ptr;
    uint8_t *buf;int buf_size;
    int shiftX,shiftY;
} CorePNGcontext;

png_voidp PNGAPI
png_memset_check (png_structp png_ptr, png_voidp s1, int value,
   png_uint_32 length)
{
   png_size_t size;

   size = (png_size_t)length;
   if ((png_uint_32)size != length)
      png_error(png_ptr,"Overflow in png_memset_check.");

   return (png_memset (s1, value, size));

}

void PNGAPI
png_chunk_error(png_structp png_ptr, png_const_charp error_message)
{
   char msg[18+64];
   //   png_format_buffer(png_ptr, msg, error_message);
   png_error(png_ptr, msg);
}

void PNGAPI
png_chunk_warning(png_structp png_ptr, png_const_charp warning_message)
{
   char msg[18+64];
   //   png_format_buffer(png_ptr, msg, warning_message);
   png_warning(png_ptr, msg);
}

void png_read_data (png_struct *png_ptr, png_byte *data, png_uint_32 length)
{
    int ret;
    CorePNGcontext *ctx=(CorePNGcontext*)png_ptr->io_ptr;

    ret=(ctx->buf_size<length)?ctx->buf_size:length;

    memcpy(data,ctx->buf,ret);
    ctx->buf+=ret;ctx->buf_size-=ret;
    if (ret != length)
	png_error (png_ptr, "Read Error");
}

static int read_image(CorePNGcontext * const a, uint8_t *ptr, int linesize,int flip)
{
    int y;

    png_read_init(a->png_ptr);
    a->png_ptr->io_ptr=a;
    if (setjmp(png_jmpbuf(a->png_ptr))) {
        png_read_destroy (a->png_ptr, a->info_ptr, NULL);
        return -1;
    }
    png_info_init(a->info_ptr);
    png_read_info (a->png_ptr, a->info_ptr);
    png_start_read_image(a->png_ptr);
    if (flip) {
        ptr+=linesize*(a->info_ptr->height-1);
        linesize*=-1;
    }
    for (y = 0; y < a->info_ptr->height; y++) {
        png_read_row(a->png_ptr, NULL, NULL);
        memcpy(ptr, a->png_ptr->row_buf + 1, a->info_ptr->rowbytes);
        ptr += linesize;
    }
    png_read_end(a->png_ptr, a->info_ptr);
    png_read_destroy (a->png_ptr, a->info_ptr, NULL);
    return 0;
}

static int decode_frame(AVCodecContext *avctx, 
                        void *data, int *data_size,
                        uint8_t *buf, int buf_size)
{
    CorePNGcontext * const a = avctx->priv_data;
    int Bpp,num_planes,swapUV=0;
    AVFrame temp,*p;

    temp= a->picture;
    a->picture= a->prev_picture;
    a->prev_picture= temp;

    p= &a->picture;
    avctx->coded_frame= p;

    avctx->flags |= CODEC_FLAG_EMU_EDGE; // alternatively we would have to use our own buffer management

    if(p->data[0])
        avctx->release_buffer(avctx, p);

    p->reference= 1;
    if(avctx->get_buffer(avctx, p) < 0){
        av_log(avctx, AV_LOG_ERROR, "get_buffer() failed\n");
        return -1;
    }

    /* special case for last picture */
    if (buf_size == 0) {
        return 0;
    }

    a->buf=buf;a->buf_size=buf_size;

    switch(a->private.bType){
    case PNGFrameType_RGB24:
        read_image(a,a->picture.data[0],a->picture.linesize[0],0);
        Bpp=3;num_planes=1;
        break;
    case PNGFrameType_YUY2:
        swapUV=1;
    case PNGFrameType_YV12:
        read_image(a,a->picture.data[0],a->picture.linesize[0],1);
        read_image(a,a->picture.data[swapUV?1:2],a->picture.linesize[swapUV?1:2],1);
        read_image(a,a->picture.data[swapUV?2:1],a->picture.linesize[swapUV?2:1],1);
        Bpp=1;num_planes=3;
        break;
    default:
        return 0;
    }

    if(avctx->sample_fmt==1){ //indicates that this is a keyframe, CorePNG doesn't store this info in the stream ifself
        a->picture.key_frame=1;
        a->picture.pict_type=FF_I_TYPE;
    }else{
        int i,shiftX=0,shiftY=0;
        a->picture.key_frame=0;
        a->picture.pict_type=FF_P_TYPE;
        for(i=0;i<num_planes;i++,shiftX=a->shiftX,shiftY=a->shiftY){
            uint8_t *cur=a->picture.data[i],*prev=a->prev_picture.data[i];
            int y;
            for(y=0;y<avctx->height>>shiftY;y++,cur+=a->picture.linesize[i],prev+=a->prev_picture.linesize[i])
                a->dsp.add_bytes(cur,prev,avctx->width*Bpp>>shiftX);
        }
    }

    *data_size = sizeof(AVFrame);
    *(AVFrame*)data = a->picture;

    return buf_size;
}

static int decode_init(AVCodecContext *avctx){
 
    CorePNGcontext * const a = avctx->priv_data;

    dsputil_init(&a->dsp, avctx);

    a->avctx= avctx;
    a->picture.data[0]=a->prev_picture.data[0]=NULL;
 
    if(avctx->extradata_size == sizeof(CorePNGCodecPrivate))
        memcpy(&a->private,avctx->extradata,sizeof(CorePNGCodecPrivate));
    else{
        a->private.wSize = sizeof(CorePNGCodecPrivate);
        a->private.bType = PNGFrameType_RGB24;
    }

    switch(a->private.bType){
    case PNGFrameType_RGB24:
        avctx->pix_fmt = PIX_FMT_BGR24;
        break;
    case PNGFrameType_YUY2:
        avctx->pix_fmt = PIX_FMT_YUV422P;
        break;
    case PNGFrameType_YV12:
        avctx->pix_fmt = PIX_FMT_YUV420P;
        break;
    }
    avctx->has_b_frames=0;
 
    avcodec_get_chroma_sub_sample(avctx->pix_fmt, &a->shiftX, &a->shiftY);

    a->png_ptr = &a->png_struct1;
    png_read_init(a->png_ptr);

    a->info_ptr = &a->png_info1;
    png_info_init(a->info_ptr);

    return 0;
}

AVCodec corepng_decoder = {
    "corepng",
    CODEC_TYPE_VIDEO,
    CODEC_ID_COREPNG,
    sizeof(CorePNGcontext),
    decode_init,
    NULL,
    NULL,
    decode_frame,
    0
};

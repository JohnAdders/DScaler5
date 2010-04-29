/*
 * wtw finder
 * Copyright (c) 2010 John Adcock
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "libavutil/avstring.h"
#include "avformat.h"
#include <strings.h>

typedef struct wtwfind_context {
    uint64_t frame_count;
    uint64_t max_histogram[3][256];
    uint64_t min_histogram[3][256];
    char path[1024];
    uint8_t peak_so_far[3];
    uint32_t peak_num_frames[3];
} WtwFindContext;


static void dumpImage(struct AVFormatContext *s, AVPacket *pkt)
{
    WtwFindContext *img = s->priv_data;
    ByteIOContext *pb = 0;
    char filename[1024];
    uint32_t pkt_size;
    AVCodecContext *codec= s->streams[ pkt->stream_index ]->codec;
    AVPicture *picture;
    AVFrame *frame = 0;
    uint8_t* buf = 0;
    AVCodec* tiffCodec = 0;
    AVCodecContext* context = 0;
    int sizeUsed;


    picture = (AVPicture *)pkt->data;
    pkt_size = codec->height * codec->width * 3;

    buf = av_mallocz(pkt_size * 2);
    if(!buf)
    {
        av_log(s, AV_LOG_ERROR, "Out of memory\n");
        return;
    }

    if (av_get_frame_filename(filename, sizeof(filename),
                              img->path, img->frame_count) < 0) 
    {
        av_log(s, AV_LOG_ERROR, "Could not get frame filename from pattern\n");
        goto tidyUp1;
        return;
    }
    if (url_fopen(&pb, filename, URL_WRONLY) < 0)
    {
        av_log(s, AV_LOG_ERROR, "Could not open file : %s\n",filename);
        goto tidyUp1;
    }

    tiffCodec = avcodec_find_encoder(CODEC_ID_TIFF);
    if (!tiffCodec) 
    {
        av_log(s, AV_LOG_ERROR, "Could not find tiff encoder\n");
        goto tidyUp2;
    }

    context = avcodec_alloc_context();
    if (!context) 
    {
        av_log(s, AV_LOG_ERROR, "Out of memory\n");
        goto tidyUp2;
    }

    context->width = codec->width;
    context->height = codec->height;
    context->pix_fmt = codec->pix_fmt;

    if (avcodec_open(context, tiffCodec) < 0)
    {
        av_log(s, AV_LOG_ERROR, "Could not open tiff encoder\n");
        goto tidyUp3;
        return;
    }

    frame = avcodec_alloc_frame();
    if(!frame)
    {
        av_log(s, AV_LOG_ERROR, "Out of memory\n");
        goto tidyUp4;
        return;
    }

    frame->data[0] = picture->data[0];
    frame->linesize[0] = codec->width * 3;

    sizeUsed = avcodec_encode_video(context, buf, pkt_size * 2, frame);
    if(sizeUsed > 0)
    {
        put_buffer(pb, buf, sizeUsed);
        put_flush_packet(pb);
    }
    else
    {
        av_log(s, AV_LOG_ERROR, "Tiff encode failed\n");
    }

    av_free(frame);
tidyUp4:
    avcodec_close(context);
tidyUp3:
    av_free(context);
tidyUp2:
    url_fclose(pb);
tidyUp1:
    av_free(buf);
}


static int wtwfind_write_header(AVFormatContext *s)
{
    WtwFindContext *wtw = s->priv_data;
    char buf[256];

    if (s->streams[0]->codec->pix_fmt == PIX_FMT_RGB24)
    {
        snprintf(buf, sizeof(buf), "Counter,NumRPixelsOverRange,NumGPixelsOverRange,NumBPixelsOverRange,PeakR,PeakG,PeakB,NumRPixelsUnderRange,NumGPixelsUnderRange,NumBPixelsUnderRange,MinR,MinG,MinB\r\n");
    }
    else if(s->streams[0]->codec->pix_fmt == PIX_FMT_YUV420P)
    {
        snprintf(buf, sizeof(buf), "Counter,NumYPixelsOverRange,NumCbPixelsOverRange,NumCrPixelsOverRange,PeakY,PeakCb,PeakCr,NumYPixelsUnderRange,NumCbPixelsUnderRange,NumCrPixelsUnderRange,MinY,MinCb,MinCr\r\n");
    }
    else
    {
        av_log(s, AV_LOG_ERROR, "Only PIX_FMT_RGBA and PIX_FMT_YUV420 are supported\n");
        return AVERROR_INVALIDDATA;
    }

    put_buffer(s->pb, buf, strlen(buf));
    put_flush_packet(s->pb);

    wtw->frame_count = 0;
    for(int i = 0; i < 3; ++i)
    {
        for(int j =0; j < 256; ++j)
        {
            wtw->max_histogram[i][j] = 0;
            wtw->min_histogram[i][j] = 0;
        }
        wtw->peak_so_far[i] = 0;
        wtw->peak_num_frames[i] = 0;
    }

    if(strncmp(s->filename, "pipe:", 5) != 0)
    {
        av_strlcpy(wtw->path, s->filename, sizeof(wtw->path));
    }
    else
    {
        av_strlcpy(wtw->path, "frame", sizeof(wtw->path));
    }
    av_strlcat(wtw->path, ".%06d.tif", sizeof(wtw->path));
    return 0;
}

static int wtwfind_write_packet_rgb24(struct AVFormatContext *s, AVPacket *pkt)
{
    AVCodecContext* codecContext = s->streams[0]->codec;
    WtwFindContext *wtw = s->priv_data;
    uint32_t counthi[3] = {0,0,0};
    uint32_t countlo[3] = {0,0,0};
    uint8_t peak[3] = {0,0,0};
    uint8_t valley[3] = {255,255,255};
    char buf[256];
    int needToDumpImage = 0;

    uint32_t offsety = codecContext->height / 20;
    uint32_t offsetx = codecContext->width / 20;

    AVPicture *picture;
    picture = (AVPicture *)pkt->data;

    for(uint32_t i = offsety; i < codecContext->height - offsety; ++i)
    {
        uint8_t* pBuff = picture->data[0] + i * codecContext->width * 3;
        pBuff += offsetx * 3;
        for(uint32_t j = offsetx; j < codecContext->width -offsetx; ++j)
        {
            for(int k = 0; k < 3; ++k)
            {
                uint8_t colour = *pBuff++;
                if(colour > 235)
                {
                    ++counthi[k];
                }
                if(colour < 16)
                {
                    ++countlo[k];
                }
                if(colour > peak[k])
                {
                    peak[k] = colour;
                }
                if(colour < valley[k])
                {
                    valley[k] = colour;
                }
            }
        }
    }

    for(int i = 0; i < 3; ++i)
    {
        ++(wtw->max_histogram[i][peak[i]]);
        ++(wtw->min_histogram[i][valley[i]]);
    }

    snprintf(buf, sizeof(buf), "%"PRId64",%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n", ++wtw->frame_count, counthi[0], counthi[1], counthi[2], peak[0], peak[1], peak[2], countlo[0], countlo[1], countlo[2], valley[0], valley[1], valley[2]);
    put_buffer(s->pb, buf, strlen(buf));
    put_flush_packet(s->pb);

    // look for frames at peaks so far in each colour
    for(int i = 0; i < 3; ++i)
    {
        if(peak[i] > wtw->peak_so_far[i])
        {
            wtw->peak_so_far[i] = peak[i];
            if(peak[i] > 235)
            {
                needToDumpImage = 1;
            }
        }
        if(counthi[i] > wtw->peak_num_frames[i])
        {
            wtw->peak_num_frames[i] = counthi[i];
            needToDumpImage = 1;
        }
    }

    if(needToDumpImage)
    {
        dumpImage(s, pkt);
    }

    return 0;
}

static int wtwfind_write_packet_yuv(struct AVFormatContext *s, AVPacket *pkt)
{
    AVCodecContext* codecContext = s->streams[0]->codec;
    WtwFindContext *wtw = s->priv_data;
    uint32_t counthi[3] = {0,0,0};
    uint32_t countlo[3] = {0,0,0};
    uint8_t peak[3] = {0,0,0};
    uint8_t valley[3] = {255,255,255};
    char buf[256];
    static uint64_t counter = 0;
    uint32_t offsety = codecContext->height / 20;
    uint32_t offsetx = codecContext->width / 20;
    uint8_t* pBuff;

    AVPicture *picture;
    picture = (AVPicture *)pkt->data;

    pBuff = picture->data[0];
    pBuff += offsety * codecContext->width;
    for(uint32_t i = offsety; i < codecContext->height - offsety; ++i)
    {
        pBuff += offsetx;
        for(uint32_t j = offsetx; j < codecContext->width - offsetx; ++j)
        {
            uint8_t colour = *pBuff++;
            if(colour > 235)
            {
                ++counthi[0];
            }
            if(colour < 16)
            {
                ++countlo[0];
            }
            if(colour > peak[0])
            {
                peak[0] = colour;
            }
            if(colour < valley[0])
            {
                valley[0] = colour;
            }
        }
        pBuff += offsetx;
    }
    pBuff += offsety * codecContext->width;
    pBuff = picture->data[1];
    pBuff += offsety * codecContext->width / 4;
    for(uint32_t i = offsety / 2; i < (codecContext->height - offsety) / 2; ++i)
    {
        pBuff += offsetx / 2;
        for(uint32_t j = offsetx / 2; j < (codecContext->width - offsetx)/ 2; ++j)
        {
            uint8_t colour = *pBuff++;
            if(colour > 240)
            {
                ++counthi[1];
            }
            if(colour < 16)
            {
                ++countlo[1];
            }
            if(colour > peak[1])
            {
                peak[1] = colour;
            }
            if(colour < valley[1])
            {
                valley[1] = colour;
            }
        }
        pBuff += offsetx / 2;
    }
    pBuff += offsety * codecContext->width / 4;
    pBuff = picture->data[2];
    pBuff += offsety * codecContext->width / 4;
    for(uint32_t i = offsety / 2; i < (codecContext->height - offsety) / 2; ++i)
    {
        pBuff += offsetx / 2;
        for(uint32_t j = offsetx / 2; j < (codecContext->width - offsetx) / 2; ++j)
        {
            uint8_t colour = *pBuff++;
            if(colour > 240)
            {
                ++counthi[2];
            }
            if(colour < 16)
            {
                ++countlo[2];
            }
            if(colour > peak[2])
            {
                peak[2] = colour;
            }
            if(colour < valley[2])
            {
                valley[2] = colour;
            }
        }
        pBuff += offsetx / 2;
    }
    pBuff += offsety * codecContext->width / 4;

    for(int i = 0; i < 3; ++i)
    {
        ++(wtw->max_histogram[i][peak[i]]);
        ++(wtw->min_histogram[i][valley[i]]);
    }

    snprintf(buf, sizeof(buf), "%"PRId64",%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n", ++counter, counthi[0], counthi[1], counthi[2], peak[0], peak[1], peak[2], countlo[0], countlo[1], countlo[2], valley[0], valley[1], valley[2]);
    put_buffer(s->pb, buf, strlen(buf));
    put_flush_packet(s->pb);
    return 0;
}

static int wtwfind_write_packet(struct AVFormatContext *s, AVPacket *pkt)
{
    if (s->streams[0]->codec->pix_fmt == PIX_FMT_RGB24)
    {
        return wtwfind_write_packet_rgb24(s, pkt);
    }
    else if(s->streams[0]->codec->pix_fmt == PIX_FMT_YUV420P)
    {
        return wtwfind_write_packet_yuv(s, pkt);
    }
    return -1;
}

static int wtwfind_write_trailer(struct AVFormatContext *s)
{
    WtwFindContext *wtw = s->priv_data;
    char buf[256];
    if (s->streams[0]->codec->pix_fmt == PIX_FMT_RGB24)
    {
        snprintf(buf, sizeof(buf), "Value,RedFramesPeak,GreenFramesPeak,BlueFramesPeak,RedFramesMin,GreenFramesMin,BlueFramesMin\r\n");
    }
    else if(s->streams[0]->codec->pix_fmt == PIX_FMT_YUV420P)
    {
        snprintf(buf, sizeof(buf), "Value,YFramesPeak,CbFramesPeak,CrFramesPeak,YFramesMin,CbFramesMin,CrFramesMin\r\n");
    }
    put_buffer(s->pb, buf, strlen(buf));
    put_flush_packet(s->pb);
    for(int j =0; j < 256; ++j)
    {
        snprintf(buf, sizeof(buf), "%d,%"PRId64",%"PRId64",%"PRId64",%"PRId64",%"PRId64",%"PRId64"\r\n", j, wtw->max_histogram[0][j], wtw->max_histogram[1][j], wtw->max_histogram[2][j], wtw->min_histogram[0][j], wtw->min_histogram[1][j], wtw->min_histogram[2][j]);
        put_buffer(s->pb, buf, strlen(buf));
        put_flush_packet(s->pb);
    }
    return 0;
}

AVOutputFormat wtwfind_muxer = {
    "wtwfind",
    NULL_IF_CONFIG_SMALL("wtwfind testing format"),
    NULL,
    "",
    sizeof(WtwFindContext),
    CODEC_ID_NONE,
    CODEC_ID_RAWVIDEO,
    wtwfind_write_header,
    wtwfind_write_packet,
    wtwfind_write_trailer,
    .flags = AVFMT_RAWPICTURE,
};

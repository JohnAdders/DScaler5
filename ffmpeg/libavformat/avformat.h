#ifndef AVFORMAT_H
#define AVFORMAT_H

#define LIBAVFORMAT_VERSION_INT 0x000406  
#define LIBAVFORMAT_VERSION     "0.4.6"
#define LIBAVFORMAT_BUILD       4603

#include "../libavcodec/avcodec.h"

#include "avio.h"

/* packet functions */

#undef AV_NOPTS_VALUE
#define AV_NOPTS_VALUE 0

typedef struct AVPacket {
    int64_t pts; /* presentation time stamp in stream units (set av_set_pts_info) */
    uint8_t *data;
    int size;
    int stream_index;
    int flags;
    int duration;       
#define PKT_FLAG_KEY   0x0001
} AVPacket; 

int av_new_packet(AVPacket *pkt, int size);
void av_free_packet(AVPacket *pkt);

/*************************************************/
/* fractional numbers for exact pts handling */

/* the exact value of the fractional number is: 'val + num / den'. num
   is assumed to be such as 0 <= num < den */
typedef struct AVFrac {
    int64_t val, num, den; 
} AVFrac;

void av_frac_init(AVFrac *f, int64_t val, int64_t num, int64_t den);
void av_frac_add(AVFrac *f, int64_t incr);
void av_frac_set(AVFrac *f, int64_t val);

/*************************************************/
/* input/output formats */

struct AVFormatContext;

/* this structure contains the data a format has to probe a file */
typedef struct AVProbeData {
    char *filename;
    unsigned char *buf;
    int buf_size;
} AVProbeData;

#define AVPROBE_SCORE_MAX 100

typedef struct AVFormatParameters {
    int frame_rate;
    int sample_rate;
    int channels;
    int width;
    int height;
    enum PixelFormat pix_fmt;
    struct AVImageFormat *image_format;
} AVFormatParameters;

#define AVFMT_NOFILE        0x0001 /* no file should be opened */
#define AVFMT_NEEDNUMBER    0x0002 /* needs '%d' in filename */ 
#define AVFMT_NOHEADER      0x0004 /* signal that no header is present
                                      (streams are added dynamically) */
#define AVFMT_SHOW_IDS      0x0008 /* show format stream IDs numbers */
#define AVFMT_RAWPICTURE    0x0020 /* format wants AVPicture structure for
                                      raw picture data */

typedef struct AVOutputFormat {
    const char *name;
    const char *long_name;
    const char *mime_type;
    const char *extensions; /* comma separated extensions */
    /* size of private data so that it can be allocated in the wrapper */
    int priv_data_size;
    /* output support */
    enum CodecID audio_codec; /* default audio codec */
    enum CodecID video_codec; /* default video codec */
    int (*write_header)(struct AVFormatContext *);
    /* XXX: change prototype for 64 bit pts */
    int (*write_packet)(struct AVFormatContext *, 
                        int stream_index,
                        unsigned char *buf, int size, int force_pts);
    int (*write_trailer)(struct AVFormatContext *);
    /* can use flags: AVFMT_NOFILE, AVFMT_NEEDNUMBER */
    int flags;
    /* currently only used to set pixel format if not YUV420P */
    int (*set_parameters)(struct AVFormatContext *, AVFormatParameters *);
    /* private fields */
    struct AVOutputFormat *next;
} AVOutputFormat;

typedef struct AVStream {
    int index;    /* stream index in AVFormatContext */
    int id;       /* format specific stream id */
    AVCodecContext codec; /* codec context */
    int r_frame_rate;     /* real frame rate of the stream */
    uint64_t time_length; /* real length of the stream in miliseconds */
    void *priv_data;
    /* internal data used in av_find_stream_info() */
    int codec_info_state;     
    int codec_info_nb_repeat_frames;
    int codec_info_nb_real_frames;
    /* PTS generation when outputing stream */
    AVFrac pts;
    /* ffmpeg.c private use */
    int stream_copy; /* if TRUE, just copy stream */
    /* quality, as it has been removed from AVCodecContext and put in AVVideoFrame
     * MN:dunno if thats the right place, for it */
    float quality; 
} AVStream;

#define MAX_STREAMS 20

/* format I/O context */
typedef struct AVFormatContext {
    /* can only be iformat or oformat, not both at the same time */
    struct AVOutputFormat *oformat;
    void *priv_data;
    ByteIOContext pb;
    int nb_streams;
    AVStream *streams[MAX_STREAMS];
    char filename[1024]; /* input or output filename */
    /* stream info */
    char title[512];
    char author[512];
    char copyright[512];
    char comment[512];
    int flags; /* format specific flags */
    /* private data for pts handling (do not modify directly) */
    int pts_wrap_bits; /* number of bits in pts (used for wrapping control) */
    int pts_num, pts_den; /* value to convert to seconds */
    /* This buffer is only needed when packets were already buffered but
       not decoded, for example to get the codec parameters in mpeg
       streams */
   struct AVPacketList *packet_buffer;
} AVFormatContext;

typedef struct AVPacketList {
    AVPacket pkt;
    struct AVPacketList *next;
} AVPacketList;

extern AVOutputFormat *first_oformat;

/* XXX: use automatic init with either ELF sections or C file parser */
/* modules */

/* mpeg.c */
int mpegps_init(void);

/* mpegts.c */
int mpegts_init(void);

/* rm.c */
int rm_init(void);

/* crc.c */
int crc_init(void);

/* img.c */
int img_init(void);

/* asf.c */
int asf_init(void);

/* avienc.c */
int avienc_init(void);

/* avidec.c */
int avidec_init(void);

/* swf.c */
int swf_init(void);

/* mov.c */
int mov_init(void);

/* jpeg.c */
int jpeg_init(void);

/* gif.c */
int gif_init(void);

/* au.c */
int au_init(void);

/* wav.c */
int wav_init(void);

/* raw.c */
int raw_init(void);

/* ogg.c */
int ogg_init(void);

/* dv.c */
int dv_init(void);

/* ffm.c */
int ffm_init(void);

/* yuv4mpeg.c */
extern AVOutputFormat yuv4mpegpipe_oformat;

/* utils.c */
#define MKTAG(a,b,c,d) (a | (b << 8) | (c << 16) | (d << 24))
#define MKBETAG(a,b,c,d) (d | (c << 8) | (b << 16) | (a << 24))

void av_register_output_format(AVOutputFormat *format);
AVOutputFormat *guess_stream_format(const char *short_name, 
                                    const char *filename, const char *mime_type);
AVOutputFormat *guess_format(const char *short_name, 
                             const char *filename, const char *mime_type);

void av_register_all(void);

#define AVERROR_UNKNOWN     (-1)  /* unknown error */
#define AVERROR_IO          (-2)  /* i/o error */
#define AVERROR_NUMEXPECTED (-3)  /* number syntax expected in filename */
#define AVERROR_INVALIDDATA (-4)  /* invalid data found */
#define AVERROR_NOMEM       (-5)  /* not enough memory */
#define AVERROR_NOFMT       (-6)  /* unknown format */

int get_frame_filename(char *buf, int buf_size,
                       const char *path, int number);

int av_find_stream_info(AVFormatContext *ic);
AVStream *av_new_stream(AVFormatContext *s, int id);
void av_set_pts_info(AVFormatContext *s, int pts_wrap_bits,
                     int pts_num, int pts_den);

/* media file output */
int av_set_parameters(AVFormatContext *s, AVFormatParameters *ap);
int av_write_header(AVFormatContext *s);
int av_write_frame(AVFormatContext *s, int stream_index, const uint8_t *buf, 
                   int size);
int av_write_trailer(AVFormatContext *s);

int strstart(const char *str, const char *val, const char **ptr);
int stristart(const char *str, const char *val, const char **ptr);
void pstrcpy(char *buf, int buf_size, const char *str);
char *pstrcat(char *buf, int buf_size, const char *s);

#endif /* AVFORMAT_H */

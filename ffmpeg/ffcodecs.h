#ifndef _FFCODECS_H_
#define _FFCODECS_H_

enum CodecID
{
 CODEC_ID_UNSUPPORTED   = -1,
 CODEC_ID_NONE          =  0,

 CODEC_ID_MPEG1VIDEO       =  1,
 CODEC_ID_MPEG2VIDEO       = 49,
 CODEC_ID_H263             =  2,
 CODEC_ID_MJPEG            =  8,
 CODEC_ID_MPEG4            = 10,
 CODEC_ID_MSMPEG4V1        = 12,
 CODEC_ID_MSMPEG4V2        = 13,
 CODEC_ID_MSMPEG4V3        = 14,
 CODEC_ID_WMV1             = 15,
 CODEC_ID_WMV2             = 16,
 CODEC_ID_H263P            = 17,
 CODEC_ID_HUFFYUV          = 26,
 CODEC_ID_FFV1             = 43,
 CODEC_ID_VP3              = 48,
 CODEC_ID_LJPEG            = 40,
 CODEC_ID_RV10             =  3, 
 CODEC_ID_RV20             = 55, 
 CODEC_ID_MP2              =  4, 
 CODEC_ID_MP3              =  5, 
 CODEC_ID_VORBIS           =  6, 
 CODEC_ID_AC3              =  7, 
 CODEC_ID_MJPEGB           =  9, 
 CODEC_ID_RAWVIDEO         = 11, 
 CODEC_ID_H263I            = 18, 
 CODEC_ID_SVQ1             = 19, 
 CODEC_ID_DVVIDEO          = 20, 
 CODEC_ID_DVAUDIO          = 21, 
 CODEC_ID_WMAV1            = 22, 
 CODEC_ID_WMAV2            = 23, 
 CODEC_ID_MACE3            = 24, 
 CODEC_ID_MACE6            = 25, 
 CODEC_ID_PCM_S16LE        = 27, 
 CODEC_ID_PCM_S16BE        = 28, 
 CODEC_ID_PCM_U16LE        = 29, 
 CODEC_ID_PCM_U16BE        = 30, 
 CODEC_ID_PCM_S8           = 31, 
 CODEC_ID_PCM_U8           = 32, 
 CODEC_ID_PCM_MULAW        = 33, 
 CODEC_ID_PCM_ALAW         = 34, 
 CODEC_ID_ADPCM_IMA_QT     = 35, 
 CODEC_ID_ADPCM_IMA_WAV    = 36, 
 CODEC_ID_ADPCM_MS         = 37, 
 CODEC_ID_ADPCM_IMA_DK3    = 68,
 CODEC_ID_ADPCM_IMA_DK4    = 69, 
 CODEC_ID_ADPCM_IMA_WS     = 70,
 CODEC_ID_ADPCM_IMA_SMJPEG = 71,
 CODEC_ID_ADPCM_4XM        = 72,
 CODEC_ID_ADPCM_XA         = 73,
 CODEC_ID_ADPCM_ADX        = 74,
 CODEC_ID_ADPCM_EA         = 75,
 CODEC_ID_ADPCM_G726       = 67,
 CODEC_ID_ADPCM_CT         = 80,
 CODEC_ID_H264             = 38,
 CODEC_ID_SVQ3             = 39,
 CODEC_ID_FLV1             = 41,
 CODEC_ID_ASV1             = 42,
 CODEC_ID_ASV2             = 50,
 CODEC_ID_CYUV             = 44,
 CODEC_ID_INDEO3           = 45,
 CODEC_ID_MSVIDEO1         = 46,
 CODEC_ID_CINEPAK          = 47,
 CODEC_ID_VCR1             = 51,
 CODEC_ID_MSRLE            = 52,
 CODEC_ID_SP5X             = 53,
 CODEC_ID_THEORA           = 54,
 CODEC_ID_MPEG2TS          = 56,
 CODEC_ID_MSZH             = 57,
 CODEC_ID_ZLIB             = 58,
 CODEC_ID_AMR_NB           = 59,
 CODEC_ID_8BPS             = 60,
 CODEC_ID_COREPNG          = 61,
 CODEC_ID_H261             = 62,
 CODEC_ID_QTRLE            = 63,
 CODEC_ID_RPZA             = 64,
 CODEC_ID_SMC              = 65,
 CODEC_ID_TRUEMOTION1      = 66,
 CODEC_ID_FLAC             = 76,
 CODEC_ID_TSCC             = 77,
 CODEC_ID_SNOW             = 78,
 CODEC_ID_GSM_MS           = 79,
 
 CODEC_ID_RAW           =100,
 CODEC_ID_YUY2          =101,
 CODEC_ID_RGB2          =102,
 CODEC_ID_RGB3          =103,
 CODEC_ID_RGB5          =104,
 CODEC_ID_RGB6          =105,
 CODEC_ID_YV12          =106,
 CODEC_ID_YVYU          =107,
 CODEC_ID_UYVY          =108,
 CODEC_ID_I420          =109,
 CODEC_ID_CLJR          =110,
 CODEC_ID_Y800          =111,
 CODEC_ID_LPCM          =198,
 CODEC_ID_PCM           =199,

 CODEC_ID_XVID          =200,
 CODEC_ID_XVID4         =201,

 CODEC_ID_LIBMPEG2      =300,

 CODEC_ID_THEORA_LIB    =400,
 
 CODEC_ID_MP3LIB        =500,
 
 CODEC_ID_LIBMAD        =600,

 CODEC_ID_LIBFAAD       =700,

 CODEC_ID_MPEG2ENC_1    =800,
 CODEC_ID_MPEG2ENC_2    =801, 
 
 CODEC_ID_WMV9          =900,
 
 CODEC_ID_AVISYNTH      =1000,
 
 CODEC_ID_SKAL          =1100,

 CODEC_ID_X264          =1200,
 
 CODEC_ID_LIBA52        =1300,
 
 CODEC_ID_SPDIF_AC3     =1400,
 CODEC_ID_SPDIF_DTS     =1401,

 CODEC_ID_LIBDTS        =1500,

 CODEC_ID_TREMOR        =1600
};

#ifdef __cplusplus

const FOURCC* getCodecFOURCCs(CodecID codecId);

static __inline bool lavc_codec(int x)     {return x>    0 && x <100;}
static __inline bool raw_codec(int x)      {return x>= 100 && x <200;}
static __inline bool xvid_codec(int x)     {return x>= 200 && x <300;}
static __inline bool theora_codec(int x)   {return x==CODEC_ID_THEORA_LIB;}
static __inline bool mplayer_codec(int x)  {return x>= 500 && x <600;}
static __inline bool mp2e_codec(int x)     {return x>= 800 && x <900;}
static __inline bool wmv9_codec(int x)     {return x>= 900 && x<1000;}
static __inline bool mpeg12_codec(int x)   {return x==CODEC_ID_MPEG1VIDEO || x==CODEC_ID_MPEG2VIDEO || x==CODEC_ID_LIBMPEG2 || mp2e_codec(x);}
static __inline bool mpeg2_codec(int x)    {return x==CODEC_ID_MPEG2VIDEO || x==CODEC_ID_LIBMPEG2 || x==CODEC_ID_MPEG2ENC_2;}
static __inline bool mpeg4_codec(int x)    {return x==CODEC_ID_MPEG4 || xvid_codec(x) || x==CODEC_ID_SKAL;}
static __inline bool spdif_codec(int x)    {return x>=1400 && x<1500;}

static __inline bool lossless_codec(int x) {return x==CODEC_ID_HUFFYUV || x==CODEC_ID_LJPEG || x==CODEC_ID_FFV1 || x==CODEC_ID_DVVIDEO;}

//I'm not sure of all these
static __inline bool sup_CBR(int x)           {return !lossless_codec(x) && !raw_codec(x) && x!=CODEC_ID_SNOW;}
static __inline bool sup_VBR_QUAL(int x)      {return !lossless_codec(x) && !raw_codec(x) && x!=CODEC_ID_SKAL && x!=CODEC_ID_X264;}
static __inline bool sup_VBR_QUANT(int x)     {return (lavc_codec(x) || xvid_codec(x) || mp2e_codec(x) || theora_codec(x) || x==CODEC_ID_SKAL || x==CODEC_ID_X264) && !lossless_codec(x) && x!=CODEC_ID_SNOW;}
static __inline bool sup_XVID2PASS(int x)     {return sup_VBR_QUANT(x) && x!=CODEC_ID_X264 && x!=CODEC_ID_SNOW;}
static __inline bool sup_LAVC2PASS(int x)     {return (lavc_codec(x) && !lossless_codec(x) && x!=CODEC_ID_MJPEG && x!=CODEC_ID_SNOW && !raw_codec(x)) || x==CODEC_ID_X264;}

static __inline bool sup_interlace(int x)         {return x==CODEC_ID_MPEG4 || x==CODEC_ID_MPEG2VIDEO || xvid_codec(x) || x==CODEC_ID_SKAL;}
static __inline bool sup_gray(int x)              {return x!=CODEC_ID_LJPEG && x!=CODEC_ID_FFV1 && x!=CODEC_ID_SNOW && !theora_codec(x) && !wmv9_codec(x) && !mp2e_codec(x) && !raw_codec(x) && x!=CODEC_ID_SKAL && x!=CODEC_ID_DVVIDEO && x!=CODEC_ID_X264;}
static __inline bool sup_globalheader(int x)      {return x==CODEC_ID_MPEG4;}
static __inline bool sup_part(int x)              {return x==CODEC_ID_MPEG4;}
static __inline bool sup_packedBitstream(int x)   {return xvid_codec(x);}
static __inline bool sup_closedGop(int x)         {return /*sup_bframes(x)*/ xvid_codec(x);}
static __inline bool sup_minKeySet(int x)         {return x!=CODEC_ID_MJPEG && x!=CODEC_ID_SNOW && !lossless_codec(x) && !wmv9_codec(x) && !raw_codec(x);}
static __inline bool sup_maxKeySet(int x)         {return x!=CODEC_ID_MJPEG && !lossless_codec(x) && !raw_codec(x);}
static __inline bool sup_bframes(int x)           {return x==CODEC_ID_MPEG4 || x==CODEC_ID_MPEG1VIDEO || x==CODEC_ID_MPEG2VIDEO || xvid_codec(x)/* || (x)==CODEC_ID_X264*/;}
static __inline bool sup_lavcme(int x)            {return lavc_codec(x) && x!=CODEC_ID_MJPEG && !lossless_codec(x);}
static __inline bool sup_quantProps(int x)        {return !lossless_codec(x) && !theora_codec(x) && !wmv9_codec(x) && !mp2e_codec(x) && !raw_codec(x) && x!=CODEC_ID_SNOW;}
static __inline bool sup_trellisQuant(int x)      {return x==CODEC_ID_MPEG4 || x==CODEC_ID_MPEG1VIDEO || x==CODEC_ID_MPEG2VIDEO || x==CODEC_ID_XVID4 || x==CODEC_ID_H263 || x==CODEC_ID_H263P || x==CODEC_ID_SKAL;}
static __inline bool sup_masking(int x)           {return x==CODEC_ID_MPEG4 || x==CODEC_ID_H263 || x==CODEC_ID_H263P || x==CODEC_ID_MPEG1VIDEO || x==CODEC_ID_MPEG2VIDEO || xvid_codec(x) || x==CODEC_ID_SKAL;}
static __inline bool sup_lavcOnePass(int x)       {return (lavc_codec(x) && !lossless_codec(x)) || x==CODEC_ID_X264;}
static __inline bool sup_perFrameQuant(int x)     {return !lossless_codec(x) && !wmv9_codec(x) && !raw_codec(x) && x!=CODEC_ID_X264 && x!=CODEC_ID_SNOW;}
static __inline bool sup_4mv(int x)               {return x==CODEC_ID_MPEG4 || x==CODEC_ID_H263 || x==CODEC_ID_H263P || x==CODEC_ID_SNOW || x==CODEC_ID_SKAL;}
static __inline bool sup_aspect(int x)            {return x==CODEC_ID_MPEG4 || x==CODEC_ID_MPEG1VIDEO || x==CODEC_ID_MPEG2VIDEO || mp2e_codec(x) || x==CODEC_ID_XVID4;}
static __inline bool sup_PSNR(int x)              {return (lavc_codec(x) && !lossless_codec(x)) || xvid_codec(x) || x==CODEC_ID_SKAL || x==CODEC_ID_X264;}
static __inline bool sup_quantBias(int x)         {return lavc_codec(x) && !lossless_codec(x);}
static __inline bool sup_MPEGquant(int x)         {return x==CODEC_ID_MPEG4 || x==CODEC_ID_MSMPEG4V3 || x==CODEC_ID_MPEG2VIDEO || xvid_codec(x) || x==CODEC_ID_SKAL;}
static __inline bool sup_lavcQuant(int x)         {return lavc_codec(x) && sup_quantProps(x);}
static __inline bool sup_customQuantTables(int x) {return x==CODEC_ID_MPEG4 || xvid_codec(x) || x==CODEC_ID_MPEG1VIDEO || x==CODEC_ID_MPEG2VIDEO || x==CODEC_ID_SKAL;}
static __inline bool sup_qpel(int x)              {return x==CODEC_ID_MPEG4 || x==CODEC_ID_SNOW || xvid_codec(x) || x==CODEC_ID_SKAL;}
static __inline bool sup_gmc(int x)               {return xvid_codec(x) || x==CODEC_ID_SKAL;}
static __inline bool sup_me_mv0(int x)            {return sup_lavcme(x) && x!=CODEC_ID_SNOW;}
static __inline bool sup_cbp_rd(int x)            {return x==CODEC_ID_MPEG4;}
static __inline bool sup_qns(int x)               {return lavc_codec(x) && sup_quantProps(x) && x!=CODEC_ID_MSMPEG4V3 && x!=CODEC_ID_MSMPEG4V2 && x!=CODEC_ID_MSMPEG4V1 && x!=CODEC_ID_WMV1 && x!=CODEC_ID_WMV2 && x!=CODEC_ID_MJPEG && x!=CODEC_ID_SNOW;}
static __inline bool sup_threads(int x)           {return x==CODEC_ID_DVVIDEO || x==CODEC_ID_MPEG1VIDEO || x==CODEC_ID_MPEG2VIDEO || x==CODEC_ID_H263 || x==CODEC_ID_H263P || x==CODEC_ID_FLV1 || x==CODEC_ID_RV10 || x==CODEC_ID_MPEG4 || x==CODEC_ID_MSMPEG4V1 || x==CODEC_ID_MSMPEG4V2 || x==CODEC_ID_MSMPEG4V3 || x==CODEC_ID_WMV1 || x==CODEC_ID_WMV2 || x==CODEC_ID_MJPEG;}

#endif

#endif

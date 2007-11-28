/*
 * E-AC3 parser
 * Copyright (c) 2007 Bartlomiej Wolowiec <bartek.wolowiec@gmail.com>
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
#ifndef EAC3_H
#define EAC3_H

#include "dsputil.h"
#include "avcodec.h"
#include "ac3.h"
#include "random.h"
#include "bitstream.h"

/* override ac3.h to include coupling channel */
#undef AC3_MAX_CHANNELS
#define AC3_MAX_CHANNELS 7

#define AC3_MAX_COEFS   256

#define AC3_BLOCK_SIZE  256
#define MAX_BLOCKS 6
#define MAX_SPX_CODES 18

typedef struct EAC3Context{
    AVCodecContext *avctx;           ///< Parent context
    int syncword;
///@name Bit stream information
///@{
    int strmtyp;                     ///< Stream type
    int substreamid;                 ///< Substream identification
    int frmsiz;                      ///< Frame size
    int fscod;                       ///< Sample rate code
    int fscod2;                      ///< Sample rate code 2
    int numblkscod;                  ///< Number of audio blocks
    int acmod;                       ///< Audio coding mode
    int lfeon;                       ///< Low frequency effect channel on
    int bsid;                        ///< Bit stream identification
    float dialnorm[2];               ///< Dialogue normalization
    int chanmap;                     ///< Custom channel map
    int mixmdate;                    ///< Mixing meta-data exists
    int dmixmod;                     ///< Preferred stereo downmix mode
    int lfemixlevcode;               ///< lfe mix level code exists
    int lfemixlevcod;                ///< lfe mix level code
    int pgmscl[2];                   ///< Program scale factor
    int extpgmscl;                   ///< External program scale factor
    int mixdef;                      ///< Mix control type
    int mixdeflen;                   ///< Length of mixing parameter data field
    int paninfo[2];                  ///< Pan information
    int frmmixcfginfoe;              ///< Frame mixing configuration information exists
    int blkmixcfginfo[6];            ///< Block mixing configuration information
    int infomdate;                   ///< Informational meta-data exists
///@}
///@name Audio Frame
///@{
    int expstre;                     ///< Exponent strategy syntax enabled
    int ahte;                        ///< Adaptive hybrid transform enabled
    int snroffststr;                 ///< SNR offset strategy
    int snroffst[AC3_MAX_CHANNELS];  ///< SNR offset
    int transproce;                  ///< Transient pre-noise processing enabled
    int blkswe;                      ///< Block switch syntax enabled
    int dithflage;                   ///< Dither flag syntax enabled
    int bamode;                      ///< Bit allocation model syntax enabled
    int frmfgaincode;                ///< Fast gain codes enabled
    int dbaflde;                     ///< Delta bit allocation syntax enabled
    int skipflde;                    ///< Skip Filed syntax enabled
    int spxattene;                   ///< Spectral extension attenuation enabled
    int cplinu[MAX_BLOCKS];          ///< Coupling in use
    int cplstre[MAX_BLOCKS];         ///< Coupling strategy exists
    int chexpstr[MAX_BLOCKS][AC3_MAX_CHANNELS];  ///< Channel exponent strategy
    int chahtinu[AC3_MAX_CHANNELS];      ///< Channel AHT in use
    int chintransproc[AC3_MAX_CHANNELS]; ///< Channel in transient pre-noise processing
    int transprocloc[AC3_MAX_CHANNELS];  ///< Transient location relative to start of frame
    int transproclen[AC3_MAX_CHANNELS];  ///< Transient processing length
    int chinspxatten[AC3_MAX_CHANNELS];  ///< Channel in spectral extension attenuation process
    int spxattencod[AC3_MAX_CHANNELS];   ///< spectral extension attenuation code
    uint32_t blkstrtinfo;            ///< Block start information
    int ncplblks;
///@}
///@name Audio block
///@{
    int blksw[AC3_MAX_CHANNELS];     ///< Block switch flag
    int dithflag[AC3_MAX_CHANNELS];  ///< Dither flag
    float dynrng[2];                 ///< Dynamic range gain word
    float downmix_coeffs[AC3_MAX_CHANNELS][2];  ///< stereo downmix coefficients
    int spxinu;                      ///< spectral extension in use
    int chinspx[AC3_MAX_CHANNELS];   ///< Channel in spectral extension
    int spxstrtf;                    ///< Spectral extension start copy frequency code
    int spxbegf;                     ///< Spectral extension begin frequency code
    int spxendf;                     ///< Spectral extension end frequency code
    int spxbndstrc[MAX_SPX_CODES];   ///< Spectral extension band structure
    int spxcoe[AC3_MAX_CHANNELS];    ///< Spectral extension coordinates exists
    int spxblnd[AC3_MAX_CHANNELS];   ///< Spectral extension blend
    int ecplinu;                     ///< Enhanced coupling in use
    int chincpl[AC3_MAX_CHANNELS];   ///< Channel in coupling
    int phsflginu;                   ///< Phase flag in use
    int cplbegf;                     ///< Coupling begin frequency code
    int cplendf;                     ///< Coupling end frequency code
    int cplbndstrc[19];              ///< Coupling band structure
    int ecplbegf;                    ///< Enhanced coupling begin frequency code
    int ecplendf;                    ///< Enhanced coupling end frequency code
    int ecplbndstrc[23];             ///< Enhanced coupling band structure
    int cplcoe[AC3_MAX_CHANNELS];    ///< Coupling coordinates exists
    int phsflg[18];                  ///< Phase flag
    int ecplangleintrp;              ///< Enhanced coupling angle interpolation flag
    int ecplparam1e[AC3_MAX_CHANNELS];   ///< Enhanced coupling parameters 1 exists
    int ecplparam2e[AC3_MAX_CHANNELS];   ///< Enhanced coupling parameters 2 exists
    int ecplamp[AC3_MAX_CHANNELS][23];   ///< Enhanced coupling amplitude scaling
    int ecplangle[AC3_MAX_CHANNELS][23]; ///< Enhanced coupling angle
    int ecplchaos[AC3_MAX_CHANNELS][23]; ///< Enhanced coupling chaos
    int ecpltrans[AC3_MAX_CHANNELS];     ///< Enhanced coupling transient present
    int rematflg[4];                 ///< Rematrixing flag
    int cplabsexp;                   ///< Coupling absolute exponent

    int fgain[AC3_MAX_CHANNELS];     ///< Channel fast gain
    int cplleake;                    ///< Coupling leak initialization exists
    uint8_t deltbae[AC3_MAX_CHANNELS];   ///< Delta bit allocation exists
    uint8_t deltnseg[AC3_MAX_CHANNELS];  ///< Channel delta bit allocation number of segments
    uint8_t deltoffst[AC3_MAX_CHANNELS][9]; ///< Channel delta bit allocation offset
    uint8_t deltlen[AC3_MAX_CHANNELS][9];   ///< Channel delta bit allocation length
    uint8_t deltba[AC3_MAX_CHANNELS][9];    ///< Channel delta bit allocation

    int got_cplchan;
    int chgaqgain[256];                             ///< Channel gain adaptive quantization gain
    float pre_chmant[6][AC3_MAX_CHANNELS][256];     ///< Pre channel mantissas

    int firstspxcos[AC3_MAX_CHANNELS];              ///< First spectral extension coordinates states
    int firstcplcos[AC3_MAX_CHANNELS];              ///< First coupling coordinates states
    int firstcplleak;                               ///< First coupling leak state
///@}

    int nrematbnds;                    ///< Number of rematrixing bands
    int ncplsubnd;                     ///< Number of coupling sub-bands
    int ncplbnd;                       ///< Number of structured coupled bands

    int nchgrps[AC3_MAX_CHANNELS];                  ///< Number of fbw channel exponent groups
    uint8_t dexps[AC3_MAX_CHANNELS][AC3_MAX_COEFS]; ///< Differential exponents

    int strtmant[AC3_MAX_CHANNELS];    ///< Start frequency bin
    int endmant[AC3_MAX_CHANNELS];     ///< End frequency bin
    int firstchincpl;
    int ecpl_start_subbnd;             ///< Enhanced coupling begin frequency
    int ecpl_end_subbnd;               ///< Enhanced coupling end frequency

    int necplbnd;                      ///< Number of structured enhanced coupling bands
    int nspxbnds;                      ///< Number of structured spectral extension bands
    int spxbndsztab[MAX_SPX_CODES];    ///< Sizes of spectral extension bands
    int nfchans;                       ///< Number of fbw channels

    uint8_t bap[AC3_MAX_CHANNELS][AC3_MAX_COEFS];   ///< bit allocation pointers
    uint8_t hebap[AC3_MAX_CHANNELS][AC3_MAX_COEFS]; ///< high-efficiency bit allocation pointers for AHT
    int16_t psd[AC3_MAX_CHANNELS][AC3_MAX_COEFS];   ///< scaled exponents
    int16_t bndpsd[AC3_MAX_CHANNELS][50];           ///< interpolated exponents
    int16_t mask[AC3_MAX_CHANNELS][50];             ///< masking values

    float   cplco[AC3_MAX_CHANNELS][18];            ///< coupling coordinates
    float   spxco[AC3_MAX_CHANNELS][18];            ///< Spectral extension coordinates

    AC3BitAllocParameters bit_alloc_params;         ///< Bit allocation parameters

    AVRandomState dith_state;        ///< for dither generation

    int num_channels;                ///< Total of all channels
    int lfe_channel;                 ///< Index of LFE channel

    GetBitContext *gbc;              ///< Bitstream reader

    MDCTContext imdct_512;           ///< for 512 sample imdct transform
    MDCTContext imdct_256;           ///< for 256 sample imdct transform
    DSPContext  dsp;                 ///< for optimization

    DECLARE_ALIGNED_16(float, transform_coeffs[AC3_MAX_CHANNELS][AC3_MAX_COEFS]);
    DECLARE_ALIGNED_16(float, delay[AC3_MAX_CHANNELS][AC3_BLOCK_SIZE]);  ///< delay - added to the next block
    DECLARE_ALIGNED_16(float, window[AC3_BLOCK_SIZE]);               ///< window coefficients
    DECLARE_ALIGNED_16(float, tmp_output[AC3_BLOCK_SIZE * 24]);      ///< temp storage for output before windowing
    DECLARE_ALIGNED_16(float, tmp_imdct[AC3_BLOCK_SIZE * 24]);       ///< temp storage for imdct transform
    DECLARE_ALIGNED_16(float, output[AC3_MAX_CHANNELS][AC3_BLOCK_SIZE]); ///< output after imdct transform and windowing
    DECLARE_ALIGNED_16(int16_t, int_output[AC3_MAX_CHANNELS][AC3_BLOCK_SIZE]);///< final 16-bit integer output


    float add_bias;                  ///< offset for float_to_int16 conversion
    float mul_bias;                  ///< scaling for float_to_int16 conversion
}EAC3Context;

/** Channel gain adaptive quantization mode */
typedef enum {
    EAC3_GAQ_NO =0,
    EAC3_GAQ_12,
    EAC3_GAQ_14,
    EAC3_GAQ_124
} EAC3GaqMode;

#endif

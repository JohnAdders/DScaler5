/*
 * Common code between AC3 encoder and decoder
 * Copyright (c) 2000, 2001, 2002 Fabrice Bellard.
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

/**
 * @file ac3.h
 * Common code between AC3 encoder and decoder.
 */

#ifndef AC3DEC_H
#define AC3DEC_H

#include "ac3tab.h"
#include "bitstream.h"
#include "dsputil.h"
#include "random.h"

/* override ac3.h to include coupling channel */
#undef AC3_MAX_CHANNELS
#define AC3_MAX_CHANNELS 7

#define CPL_CH 0

#define AC3_OUTPUT_LFEON  8

#define AC3_MAX_COEFS   256
#define AC3_BLOCK_SIZE  256
#define MAX_BLOCKS 6
#define MAX_SPX_CODES 18

void ff_ac3_window_init(float *window);
void ff_ac3_tables_init(void);

/** dynamic range table. converts codes to scale factors. */
extern float ff_ac3_dynamic_range_tab[256];

/** dialog normalization table */
extern float ff_ac3_dialog_norm_tab[32];

/**
 * table for exponent to scale_factor mapping
 * ff_ac3_scale_factors[i] = 2 ^ -i
 */
extern float ff_ac3_scale_factors[25];

/** channel mix levels */
extern const float ff_ac3_mix_levels[9];

/** default stereo downmixing coefficients */
extern const uint8_t ff_ac3_default_coeffs[8][5][2];

typedef struct AC3DecodeContext {
    AVCodecContext *avctx;  ///< Parent context
    GetBitContext gbc;      ///< Bitstream reader

///@defgroup bsi Bit Stream Information
///@{
    int stream_type;    ///< Stream type (strmtyp)
    int substreamid;    ///< Substream identification
    int frame_size;     ///< Frame size, in bytes
    int bit_rate;       ///< Bitrate, in bits-per-second
    int sr_code;        ///< Sample rate code (fscod)
    int sr_code2;       ///< Sample rate code 2 (fscod2)
    int sample_rate;    ///< Sample rate, in Hz
    int num_blocks;     ///< Number of audio blocks
    int channel_mode;   ///< Channel mode (acmod)
    int lfe_on;         ///< Low frequency effect channel on (lfeon)
    int bitstream_id;   ///< Bit stream identification (bsid)
    int center_mix_level;   ///< Center mix level index
    int surround_mix_level; ///< Surround mix level index
    int eac3;               ///< indicates if current frame is E-AC3
///@}

///@defgroup audfrm Frame Syntax Parameters
    int snr_offset_strategy;    ///< SNR offset strategy (snroffststr)
    int block_switch_syntax;    ///< Block switch syntax enabled (blkswe)
    int dither_flag_syntax;     ///< Dither flag syntax enabled (dithflage)
    int bit_allocation_syntax;  ///< Bit allocation model syntax enabled (bamode)
    int fast_gain_syntax;       ///< Fast gain codes enabled (frmfgaincode)
    int dba_syntax;             ///< Delta bit allocation syntax enabled (dbaflde)
    int skip_syntax;            ///< Skip Filed syntax enabled (skipflde)
///@}

///@defgroup cpl Standard Coupling
    int cpl_in_use[MAX_BLOCKS];                 ///< Coupling in use (cplinu)
    int cpl_strategy_exists[MAX_BLOCKS];        ///< Coupling strategy exists (cplstre)
    int channel_in_cpl[AC3_MAX_CHANNELS];       ///< Channel in coupling (chincpl)
    int phase_flags_in_use;                     ///< Phase flag in use (phsflginu)
    int phase_flags[18];                        ///< Phase flag
    int num_cpl_subbands;                       ///< Number of coupling sub bands (ncplsubnd)
    int num_cpl_bands;                          ///< Number of coupling bands (ncplbnd)
    int cpl_band_struct[18];                    ///< Coupling band structure (cplbndstrc)
    int firstchincpl;                           ///< First channel in coupling
    int first_cpl_coords[AC3_MAX_CHANNELS];     ///< First coupling coordinates states (firstcplcos)
    float cpl_coords[AC3_MAX_CHANNELS][18];     ///< coupling coordinates (cplco)
///@}

///@defgroup aht Adaptive Hybrid Transform
    int channel_uses_aht[AC3_MAX_CHANNELS];     ///< Channel AHT in use (chahtinu)
    int gaq_gain[256];                              ///< Gain adaptive quantization gain
    float pre_mantissa[6][AC3_MAX_CHANNELS][256];   ///< Pre-IDCT mantissas
///@}

///@defgroup spx Spectral Extension
    int channel_uses_spx[AC3_MAX_CHANNELS]; ///< Channel in spectral extension attenuation process (chinspxatten)
    int spx_atten_code[AC3_MAX_CHANNELS];   ///< spectral extension attenuation code (spxattencod)
    int spxinu;                             ///< spectral extension in use
    int chinspx[AC3_MAX_CHANNELS];          ///< Channel in spectral extension
    int spxstrtf;                           ///< Spectral extension start copy frequency code
    int spxbegf;                            ///< Spectral extension begin frequency code
    int spxendf;                            ///< Spectral extension end frequency code
    int nspxbnds;                           ///< Number of structured spectral extension bands
    int spxbndsztab[MAX_SPX_CODES];         ///< Sizes of spectral extension bands
    int spxbndstrc[MAX_SPX_CODES];          ///< Spectral extension band structure
    int spxcoe[AC3_MAX_CHANNELS];           ///< Spectral extension coordinates exists
    int spxblnd[AC3_MAX_CHANNELS];          ///< Spectral extension blend
    int firstspxcos[AC3_MAX_CHANNELS];      ///< First spectral extension coordinates states
    float spxco[AC3_MAX_CHANNELS][18];      ///< Spectral extension coordinates
///@}

///@defgroup ecpl Enhanced Coupling
    int ecpl_in_use;                        ///< Enhanced coupling in use
    int ecplbegf;                           ///< Enhanced coupling begin frequency code
    int ecplendf;                           ///< Enhanced coupling end frequency code
    int ecpl_start_subbnd;                  ///< Enhanced coupling begin frequency
    int ecpl_end_subbnd;                    ///< Enhanced coupling end frequency
    int necplbnd;                           ///< Number of structured enhanced coupling bands
    int ecplbndstrc[23];                    ///< Enhanced coupling band structure
    int ecplangleintrp;                     ///< Enhanced coupling angle interpolation flag
    int ecplparam1e[AC3_MAX_CHANNELS];      ///< Enhanced coupling parameters 1 exists
    int ecplparam2e[AC3_MAX_CHANNELS];      ///< Enhanced coupling parameters 2 exists
    int ecplamp[AC3_MAX_CHANNELS][23];      ///< Enhanced coupling amplitude scaling
    int ecplangle[AC3_MAX_CHANNELS][23];    ///< Enhanced coupling angle
    int ecplchaos[AC3_MAX_CHANNELS][23];    ///< Enhanced coupling chaos
    int ecpltrans[AC3_MAX_CHANNELS];        ///< Enhanced coupling transient present
///@}

///@defgroup channel Channel
    int fbw_channels;                           ///< Number of fbw channels
    int channels;                               ///< Total of all channels
    int lfe_ch;                                 ///< Index of LFE channel
    float downmix_coeffs[AC3_MAX_CHANNELS][2];  ///< stereo downmix coefficients
    int output_mode;                            ///< output channel configuration
    int out_channels;                           ///< number of output channels
///@}

///@defgroup dynrng Dynamic Range
    float dynamic_range[2]; ///< Dynamic range gain (dynrng)
///@}

///@defgroup bandwidth Bandwidth
    int start_freq[AC3_MAX_CHANNELS];   ///< Start frequency bin (strtmant)
    int end_freq[AC3_MAX_CHANNELS];     ///< End frequency bin (endmant)
///@}

///@defgroup rematrixing Rematrixing
    int num_rematrixing_bands;  ///< Number of rematrixing bands (nrematbnds)
    int rematrixing_flags[4];   ///< Rematrixing flags (rematflg)
///@}

///@defgroup exponents Exponents
    int nchgrps[AC3_MAX_CHANNELS];                  ///< Number of fbw channel exponent groups
    uint8_t dexps[AC3_MAX_CHANNELS][AC3_MAX_COEFS]; ///< Differential exponents
    int exp_strategy[MAX_BLOCKS][AC3_MAX_CHANNELS]; ///< Channel exponent strategy (chexpstr)
///@}

///@defgroup bitalloc Bit Allocation
    AC3BitAllocParameters bit_alloc_params;         ///< Bit allocation parameters
    int first_cpl_leak;                             ///< First coupling leak state (firstcplleak)
    int snr_offset[AC3_MAX_CHANNELS];               ///< SNR offset (snroffst)
    int fast_gain[AC3_MAX_CHANNELS];                ///< Channel fast gain (fgain)
    uint8_t bap[AC3_MAX_CHANNELS][AC3_MAX_COEFS];   ///< bit allocation pointers
    uint8_t hebap[AC3_MAX_CHANNELS][AC3_MAX_COEFS]; ///< high-efficiency bit allocation pointers for AHT
    int16_t psd[AC3_MAX_CHANNELS][AC3_MAX_COEFS];   ///< scaled exponents
    int16_t band_psd[AC3_MAX_CHANNELS][50];         ///< interpolated exponents (bndpsd)
    int16_t mask[AC3_MAX_CHANNELS][50];             ///< masking values
    uint8_t dba_mode[AC3_MAX_CHANNELS];             ///< Delta bit allocation mode (deltbae)
    uint8_t dba_nsegs[AC3_MAX_CHANNELS];            ///< Number of delta segments (deltnseg)
    uint8_t dba_offsets[AC3_MAX_CHANNELS][9];       ///< Delta segment offsets (deltoffst)
    uint8_t dba_lengths[AC3_MAX_CHANNELS][9];       ///< Delta segment lengths (deltlen)
    uint8_t dba_values[AC3_MAX_CHANNELS][9];        ///< Delta values for each segment (deltba)
///@}

///@defgroup dithering Zero-Mantissa Dithering
    int dither_all;                     ///< Indicates whether all channels use dithering
    int dither_flag[AC3_MAX_CHANNELS];  ///< Dither flag (dithflag)
    AVRandomState dith_state;           ///< for dither generation
///@}

///@defgroup imdct IMDCT
    int block_switch[AC3_MAX_CHANNELS]; ///< Block switch flag (blksw)
    MDCTContext imdct_512;              ///< for 512 sample imdct transform
    MDCTContext imdct_256;              ///< for 256 sample imdct transform
///@}

///@defgroup opt Optimization
    DSPContext  dsp;    ///< for optimization
    float add_bias;     ///< offset for float_to_int16 conversion
    float mul_bias;     ///< scaling for float_to_int16 conversion
///@}

///@defgroup arrays Aligned Arrays
    DECLARE_ALIGNED_16(float, transform_coeffs[AC3_MAX_CHANNELS][AC3_MAX_COEFS]);   ///< Frequency Coefficients
    DECLARE_ALIGNED_16(float, delay[AC3_MAX_CHANNELS][AC3_BLOCK_SIZE]);             ///< delay - added to the next block
    DECLARE_ALIGNED_16(float, window[AC3_BLOCK_SIZE]);                              ///< window coefficients
    DECLARE_ALIGNED_16(float, tmp_output[AC3_BLOCK_SIZE * 24]);                     ///< temp storage for output before windowing
    DECLARE_ALIGNED_16(float, tmp_imdct[AC3_BLOCK_SIZE * 24]);                      ///< temp storage for imdct transform
    DECLARE_ALIGNED_16(float, output[AC3_MAX_CHANNELS][AC3_BLOCK_SIZE]);            ///< output after imdct transform and windowing
    DECLARE_ALIGNED_16(int16_t, int_output[AC3_MAX_CHANNELS][AC3_BLOCK_SIZE]);      ///< final 16-bit integer output
///@}
} AC3DecodeContext;

/**
 * Decode the grouped exponents according to exponent strategy.
 * reference: Section 7.1.3 Exponent Decoding
 */
void ff_ac3_decode_exponents(GetBitContext *gb, int exp_strategy, int ngrps,
                             uint8_t absexp, int8_t *dexps);

/**
 * Grouped mantissas for 3-level 5-level and 11-level quantization
 */
typedef struct {
    float b1_mant[3];
    float b2_mant[3];
    float b4_mant[2];
    int b1ptr;
    int b2ptr;
    int b4ptr;
} mant_groups;

int ff_ac3_parse_frame_header(AC3DecodeContext *s);

/* TEMPORARY SOLUTION */
int ff_eac3_parse_header(AC3DecodeContext *s);
int ff_eac3_parse_audio_block(AC3DecodeContext *s, int blk);
void ff_eac3_tables_init(void);

int ff_ac3_get_transform_coeffs_ch(AC3DecodeContext *s, int ch, mant_groups *m);

void ff_ac3_uncouple_channels(AC3DecodeContext *s);

void ff_ac3_remove_dithering(AC3DecodeContext *s);

void ff_ac3_do_rematrixing(AC3DecodeContext *s);

void ff_ac3_do_imdct(AC3DecodeContext *s);

void ff_ac3_downmix(AC3DecodeContext *s);

void ff_ac3_set_downmix_coeffs(AC3DecodeContext *s);

/** Adjustments in dB gain */
#define LEVEL_PLUS_3DB          1.4142135623730950
#define LEVEL_PLUS_1POINT5DB    1.1892071150027209
#define LEVEL_MINUS_1POINT5DB   0.8408964152537145
#define LEVEL_MINUS_3DB         0.7071067811865476
#define LEVEL_MINUS_4POINT5DB   0.5946035575013605
#define LEVEL_MINUS_6DB         0.5000000000000000
#define LEVEL_MINUS_9DB         0.3535533905932738
#define LEVEL_ZERO              0.0000000000000000
#define LEVEL_ONE               1.0000000000000000


#endif /* AC3DEC_H */

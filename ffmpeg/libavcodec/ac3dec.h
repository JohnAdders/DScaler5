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
#include "random.h"

#define CPL_CH 0

void ff_ac3_window_init(float *window);
void ff_ac3_tables_init(void);

/** tables for ungrouping mantissas */
extern float ff_ac3_b1_mantissas[32][3];
extern float ff_ac3_b2_mantissas[128][3];
extern float ff_ac3_b3_mantissas[8];
extern float ff_ac3_b4_mantissas[128][2];
extern float ff_ac3_b5_mantissas[16];

/** dynamic range table. converts codes to scale factors. */
extern float ff_ac3_dynrng_tbl[256];

/** dialogue normalization table */
extern float ff_ac3_dialnorm_tbl[32];

/**
 * table for exponent to scale_factor mapping
 * ff_ac3_scale_factors[i] = 2 ^ -i
 */
extern float ff_ac3_scale_factors[25];

/** table for grouping exponents */
extern uint8_t ff_ac3_exp_ungroup_tbl[128][3];

/**
 * Decode the grouped exponents according to exponent strategy.
 * reference: Section 7.1.3 Exponent Decoding
 */
void ff_ac3_decode_exponents(GetBitContext *gb, int expstr, int ngrps,
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

int ff_ac3_get_transform_coeffs_ch(mant_groups *m, GetBitContext *gb, uint8_t *exps,
        uint8_t *bap, float *coeffs, int start, int end, AVRandomState *dith_state);

void ff_ac3_do_rematrixing(float (*transform_coeffs)[256], int end, int nrematbnd, int *rematflg);

void ff_ac3_do_imdct_256(float *tmp_output, float *transform_coeffs,
        MDCTContext *imdct_256, float *tmp_imdct);

void ff_ac3_downmix(float samples[AC3_MAX_CHANNELS][256], int nfchans,
                        int output_mode, float coef[AC3_MAX_CHANNELS][2]);

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

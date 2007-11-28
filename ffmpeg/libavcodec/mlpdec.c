/*
 * MLP decoder
 * Copyright (c) 2007 Ian Caulfield
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
 * @file mlpdec.c
 * MLP decoder
 */

#include "avcodec.h"
#include "intreadwrite.h"
#include "bitstream.h"
#include "crc.h"
#include "parser.h"
#include "mlp_parser.h"

/** Maximum number of channels that can be decoded. It should be safe to
 *  increase this value, but I haven't got any test streams with more than
 *  5.1 channels */
#define MAX_CHANNELS        6

/** Maximum number of substreams that can be decoded. This could also be set
 *  higher, but again I haven't seen any examples with more than two */
#define MAX_SUBSTREAMS      2

/** Maximum sample frequency supported */
#define MAX_SAMPLERATE      192000

/** The maximum number of audio samples within one access unit */
#define MAX_BLOCKSIZE       (40 * (MAX_SAMPLERATE / 48000))
/** The next power of two greater than MAX_BLOCKSIZE */
#define MAX_BLOCKSIZE_POW2  (64 * (MAX_SAMPLERATE / 48000))

/** The maximum number of taps in either the IIR or FIR filter.
 *  I believe MLP actually specifies the maximum order for IIR filters is four,
 *  and that the sum of the orders of both filters must be <= 8 */
#define MAX_FILTER_ORDER    8

/** Number of bits used for VLC lookup - longest huffman code is 9 */
#define VLC_BITS            9

typedef struct MLPDecodeContext {
    AVCodecContext *avctx;

    uint8_t     *out_buf;
    int         bytes_output;
    int         out_buf_remaining;

    uint8_t     in_sync;
    uint8_t     max_decoded_substream;
    uint8_t     stream_type;
    int         access_unit_size;
    int         access_unit_size_pow2;
    uint8_t     restart_seen[MAX_SUBSTREAMS];

    uint8_t     num_substreams;

    //@{
    /** Substream header info */
    uint8_t     restart_header_present[MAX_SUBSTREAMS];
    uint8_t     substream_parity_present[MAX_SUBSTREAMS];
    uint16_t    substream_data_len[MAX_SUBSTREAMS];
    //@}

    //@{
    /** Restart header data */
    uint16_t    restart_sync[MAX_SUBSTREAMS];
    uint8_t     min_channel[MAX_SUBSTREAMS];
    uint8_t     max_channel[MAX_SUBSTREAMS];
    uint8_t     max_matrix_channel[MAX_SUBSTREAMS];
    uint8_t     noise_shift[MAX_SUBSTREAMS];
    uint32_t    noisegen_seed[MAX_SUBSTREAMS];
    uint8_t     max_lsbs[MAX_SUBSTREAMS];
    uint8_t     max_bits1[MAX_SUBSTREAMS];
    uint8_t     max_bits2[MAX_SUBSTREAMS];
    uint8_t     data_check_present[MAX_SUBSTREAMS];
    uint8_t     ch_assign[MAX_SUBSTREAMS][MAX_CHANNELS];
    uint8_t     param_presence_flags[MAX_SUBSTREAMS];
    //@}

    //@{
    /** Matrix data */
    uint8_t     num_primitive_matrices[MAX_SUBSTREAMS];
    uint8_t     matrix_ch[MAX_SUBSTREAMS][MAX_CHANNELS];
    uint8_t     lsb_bypass[MAX_SUBSTREAMS][MAX_CHANNELS];
    int32_t     matrix_coeff[MAX_SUBSTREAMS][MAX_CHANNELS][MAX_CHANNELS+2];
    uint8_t     matrix_noise_shift[MAX_SUBSTREAMS][MAX_CHANNELS];
    //@}

    uint16_t    blocksize[MAX_SUBSTREAMS];
    uint16_t    blockpos[MAX_SUBSTREAMS];
    int8_t      output_shift[MAX_SUBSTREAMS][MAX_CHANNELS+2];
    uint8_t     quant_step_size[MAX_SUBSTREAMS][MAX_CHANNELS];

    //@{
    /* Filter data. Filter 0 is an FIR filter, filter 1 IIR */
    uint8_t     filter_order[MAX_CHANNELS][2];
    uint8_t     filter_coeff_q[MAX_CHANNELS][2];
    int32_t     filter_coeff[MAX_CHANNELS][2][MAX_FILTER_ORDER];
    int32_t     filter_state[MAX_CHANNELS][2][MAX_FILTER_ORDER];
    //@}

    //@{
    /** Sample data coding infomation */
    int16_t     huff_offset[MAX_CHANNELS];
    uint8_t     codebook[MAX_CHANNELS];
    uint8_t     huff_lsbs[MAX_CHANNELS];
    //@}

    int32_t     lossless_check_data[MAX_SUBSTREAMS];

    int8_t      noise_buffer[MAX_BLOCKSIZE_POW2];
    int8_t      bypassed_lsbs[MAX_BLOCKSIZE][MAX_CHANNELS];
    int32_t     sample_buffer[MAX_BLOCKSIZE][MAX_CHANNELS+2];
} MLPDecodeContext;

/** Tables defining the huffman codes.
 *  There are three entropy coding methods used in MLP (four if you count "none"
 *  as a method). These use the same sequences for codes starting 00... or 01...
 *  but have different codes starting 1...
 */

static const uint8_t huffman_tables[3][18][2] = {
    { /* Huffman table 0, -7 - +10 */
        {0x01, 9}, {0x01, 8}, {0x01, 7}, {0x01, 6}, {0x01, 5}, {0x01, 4}, {0x01, 3},
        {0x04, 3}, {0x05, 3}, {0x06, 3}, {0x07, 3},
        {0x03, 3}, {0x05, 4}, {0x09, 5}, {0x11, 6}, {0x21, 7}, {0x41, 8}, {0x81, 9},
    }, { /* Huffman table 1, -7 - +8 */
        {0x01, 9}, {0x01, 8}, {0x01, 7}, {0x01, 6}, {0x01, 5}, {0x01, 4}, {0x01, 3},
        {0x02, 2}, {0x03, 2},
        {0x03, 3}, {0x05, 4}, {0x09, 5}, {0x11, 6}, {0x21, 7}, {0x41, 8}, {0x81, 9},
    }, { /* Huffman table 1, -7 - +7 */
        {0x01, 9}, {0x01, 8}, {0x01, 7}, {0x01, 6}, {0x01, 5}, {0x01, 4}, {0x01, 3},
        {0x01, 1},
        {0x03, 3}, {0x05, 4}, {0x09, 5}, {0x11, 6}, {0x21, 7}, {0x41, 8}, {0x81, 9},
    }
};

static VLC huff_vlc[3];

static AVCRC crc_63[1024];
static AVCRC crc_1D[1024];


/** Initialize static data, constant between all invocations of the codec */

static void init_static()
{
    static int done = 0;

    if (!done) {
        init_vlc(&huff_vlc[0], VLC_BITS, 18,
                 &huffman_tables[0][0][1], 2, 1,
                 &huffman_tables[0][0][0], 2, 1, 1);
        init_vlc(&huff_vlc[1], VLC_BITS, 16,
                 &huffman_tables[1][0][1], 2, 1,
                 &huffman_tables[1][0][0], 2, 1, 1);
        init_vlc(&huff_vlc[2], VLC_BITS, 15,
                 &huffman_tables[2][0][1], 2, 1,
                 &huffman_tables[2][0][0], 2, 1, 1);

        av_crc_init(crc_63, 0,  8,   0x63, sizeof(crc_63));
        av_crc_init(crc_1D, 0,  8,   0x1D, sizeof(crc_1D));

        done = 1;
    }
}


/** MLP uses checksums that seem to be based on the standard CRC algorithm,
 *  but not (in implementation terms, the table lookup and XOR are reversed).
 *  We can implement this behaviour using a standard av_crc on all but the
 *  last element, then XOR that with the last element.
 */

static uint8_t mlp_checksum8(const uint8_t *buf, unsigned int buf_size)
{
    uint8_t checksum = av_crc(crc_63, 0x3c, buf, buf_size - 1); // crc_63[0xa2] == 0x3c
    checksum ^= buf[buf_size-1];
    return checksum;
}

/** Calculate an 8-bit checksum over a restart header -- a non-multiple-of-8
 *  number of bits, starting two bits into the first byte of buf.
 */

static uint8_t mlp_restart_checksum(const uint8_t *buf, unsigned int bit_size)
{
    int i;
    int num_bytes = (bit_size + 2) / 8;

    uint8_t crc = crc_1D[buf[0] & 0x3f];
    crc = av_crc(crc_1D, crc, buf + 1, num_bytes - 2);
    crc ^= buf[num_bytes - 1];

    for (i = 0; i < (bit_size + 2) % 8; i++) {
        if (crc & 0x80)
            crc = (crc << 1) ^ 0x1D;
        else
            crc = crc << 1;
        crc ^= (buf[num_bytes] >> (7 - i)) & 1;
    }

    return crc;
}

/** Read a sample, consisting of either, both or neither of entropy-coded MSBs
 *  and plain LSBs
 */

static inline int read_huff(MLPDecodeContext *m, GetBitContext *gbp,
                            unsigned int substr, unsigned int channel)
{
    int codebook = m->codebook[channel];
    int quant_step_size = m->quant_step_size[substr][channel];
    int lsb_bits = m->huff_lsbs[channel] - quant_step_size;
    int sign_shift;
    int msbs = 0, lsbs = 0;
    int result;

    if (codebook > 0)
        msbs = get_vlc2(gbp, huff_vlc[codebook-1].table,
                        VLC_BITS, (9 + VLC_BITS - 1) / VLC_BITS) - 7;

    if (lsb_bits > 0)
        lsbs = get_bits(gbp, lsb_bits);

    result = (msbs << lsb_bits) + lsbs;
    sign_shift = lsb_bits + (codebook ? 2 - codebook : -1);

    if (sign_shift >= 0)
        result += (-1) << sign_shift;

    result += m->huff_offset[channel];

    return result << quant_step_size;
}

/** Initialize the decoder */

static int mlp_decode_init(AVCodecContext *avctx)
{
    MLPDecodeContext *m = avctx->priv_data;

    init_static();
    m->avctx = avctx;
    return 0;
}

/** Read a major sync info header - contains high level information about
 *  the stream - sample rate, channel arrangement etc. Most of this
 *  information is not actually necessary for decoding, only for playback.
 */

static int read_major_sync(MLPDecodeContext *m, const uint8_t *buf,
                           unsigned int buf_size)
{
    MLPHeaderInfo mh;
    int substr;

    if (ff_mlp_read_major_sync(m->avctx, &mh, buf, buf_size) != 0)
        return -1;

    m->stream_type = mh.stream_type;

    if (m->stream_type == 0xbb) {
        if (mh.group1_bits == 0) {
            av_log(m->avctx, AV_LOG_ERROR, "Invalid/unknown bits per sample\n");
            return -1;
        }
        if (mh.group2_bits > mh.group1_bits) {
            av_log(m->avctx, AV_LOG_ERROR,
                   "Channel group 2 cannot have more bits per sample than group 1\n");
            return -1;
        }

        if (mh.group2_samplerate != 0 && mh.group2_samplerate != mh.group1_samplerate) {
            av_log(m->avctx, AV_LOG_ERROR,
                   "Channel groups with differing sample rates not currently supported\n");
            return -1;
        }
    }

    if (mh.group1_samplerate == 0) {
        av_log(m->avctx, AV_LOG_ERROR, "Invalid/unknown sampling rate\n");
        return -1;
    }
    if (mh.group1_samplerate > MAX_SAMPLERATE) {
        av_log(m->avctx, AV_LOG_ERROR,
               "Sampling rate %d is greater than maximum supported (%d)\n",
               mh.group1_samplerate, MAX_SAMPLERATE);
        return -1;
    }
    m->access_unit_size = mh.access_unit_size;
    m->access_unit_size_pow2 = mh.access_unit_size_pow2;

    m->num_substreams = mh.num_substreams;
    if (m->num_substreams > MAX_SUBSTREAMS)
        av_log(m->avctx, AV_LOG_INFO,
               "Number of substreams %d is more than maximum supported by "
               "decoder. Only the first %d will be decoded. Please provide a "
               "sample of this file to the FFmpeg development list.\n",
               m->num_substreams, MAX_SUBSTREAMS);

    m->max_decoded_substream = FFMIN(m->num_substreams, MAX_SUBSTREAMS) - 1;

    m->avctx->sample_rate = mh.group1_samplerate;

#ifdef CONFIG_AUDIO_NONSHORT
    m->avctx->bits_per_sample = mh.group1_bits;
	if (mh.group1_bits > 16) {
		m->avctx->sample_fmt = SAMPLE_FMT_S32;
    }
#endif

    m->in_sync = 1;
    for (substr = 0; substr < m->num_substreams; substr++)
        m->restart_seen[substr] = 0;

    return 0;
}

/** Read a restart header from a block in a substream. This contains parameters
 *  required to decode the audio that do not change very often. Generally
 *  (always) present only in blocks following a major sync.
 */

static int read_restart_header(MLPDecodeContext *m, GetBitContext *gbp,
                               const uint8_t *buf, unsigned int substr)
{
    unsigned int ch;
    int sync_word, tmp;
    uint8_t checksum;
    uint8_t lossless_check;
    int start_count = get_bits_count(gbp);

    sync_word = get_bits(gbp, 14);

    if ((sync_word & 0x3ffe) != 0x31ea) {
        av_log(m->avctx, AV_LOG_ERROR,
               "Restart header sync incorrect (got 0x%04x)\n", sync_word);
        return -1;
    }
    m->restart_sync[substr] = sync_word;

    skip_bits(gbp, 16); /* Output timestamp */

    m->min_channel[substr]        = get_bits(gbp, 4);
    m->max_channel[substr]        = get_bits(gbp, 4);
    m->max_matrix_channel[substr] = get_bits(gbp, 4);

    if (m->min_channel[substr] > m->max_channel[substr]) {
        av_log(m->avctx, AV_LOG_ERROR,
               "Substream min channel cannot be greater than max channel.\n");
        return -1;
    }

    if (m->max_channel[substr] >= MAX_CHANNELS
        || m->max_matrix_channel[substr] >= MAX_CHANNELS)
    {
        if (substr > 0) {
            av_log(m->avctx, AV_LOG_INFO,
                   "Substream %d contains more channels than the maximum "
                   "supported by this decoder (%d). Only substreams up to %d "
                   "will be decoded. Please provide a sample of this file "
                   "to the FFmpeg development list.\n",
                   substr, MAX_CHANNELS, substr - 1);
            m->max_decoded_substream = substr - 1;
            m->avctx->channels = m->max_channel[substr - 1] + 1;
        } else {
            av_log(m->avctx, AV_LOG_INFO,
                   "This stream contains more channels than the maximum "
                   "supported by this decoder (%d). Please provide a sample "
                   "of this file to the FFmpeg development list.\n",
                   MAX_CHANNELS);
            return -1;
        }
    }

    if (m->avctx->request_channels > 0
        && m->max_channel[substr] + 1 >= m->avctx->request_channels
        && substr < m->max_decoded_substream) {
        av_log(m->avctx, AV_LOG_INFO,
               "Extracting %d channel downmix from substream %d. "
               "Further substreams will be skipped.\n",
               m->max_channel[substr] + 1, substr);
        m->max_decoded_substream = substr;
        m->avctx->channels = m->max_channel[substr] + 1;
    }

    m->noise_shift[substr] = get_bits(gbp, 4);
    m->noisegen_seed[substr] = get_bits(gbp, 23);
    av_log(m->avctx, AV_LOG_DEBUG, "noise shift %d, seed 0x%x\n",
           m->noise_shift[substr], m->noisegen_seed[substr]);

    skip_bits(gbp, 4);
    m->max_lsbs[substr] = get_bits(gbp, 5);
    m->max_bits1[substr] = get_bits(gbp, 5);
    m->max_bits2[substr] = get_bits(gbp, 5);
    av_log(m->avctx, AV_LOG_DEBUG, "max_lsbs %d max_bits1 %d max_bits2 %d\n",
           m->max_lsbs[substr], m->max_bits1[substr], m->max_bits2[substr]);

    m->data_check_present[substr] = get_bits1(gbp);
    lossless_check = get_bits(gbp, 8);
    if (substr == m->max_decoded_substream) {
        tmp = m->lossless_check_data[substr];
        tmp ^= tmp >> 16;
        tmp ^= tmp >> 8;
        tmp &= 0xff;
        if (tmp != lossless_check)
            av_log(m->avctx, AV_LOG_INFO,
                   "Lossless check failed - expected %x, calculated %x\n",
                   lossless_check, tmp);
        else
            av_log(m->avctx, AV_LOG_DEBUG,
                   "Lossless check passed for substream %d (%x)\n",
                   substr, tmp);
    }

    skip_bits(gbp, 16);

    for (ch = 0; ch <= m->max_matrix_channel[substr]; ch++) {
        m->ch_assign[substr][ch] = get_bits(gbp, 6);
        av_log(m->avctx, AV_LOG_DEBUG, "ch_assign[%d][%d] = %d\n",
               substr, ch, m->ch_assign[substr][ch]);
    }

    checksum = mlp_restart_checksum(buf, get_bits_count(gbp) - start_count);

    if (checksum != get_bits(gbp, 8))
        av_log(m->avctx, AV_LOG_ERROR, "Restart header checksum error\n");

    /* Set default decoding parameters */
    m->param_presence_flags[substr] = 0xff;
    m->num_primitive_matrices[substr] = 0;
    m->blocksize[substr] = 8;
    m->lossless_check_data[substr] = 0;

    memset(m->output_shift[substr],    0, sizeof(m->output_shift[substr]));
    memset(m->quant_step_size[substr], 0, sizeof(m->quant_step_size[substr]));

    for (ch = m->min_channel[substr]; ch <= m->max_channel[substr]; ch++) {
        m->filter_order[ch][0] = 0;
        m->filter_order[ch][1] = 0;
        m->filter_coeff_q[ch][0] = 0;
        m->filter_coeff_q[ch][1] = 0;

        memset(m->filter_coeff[ch], 0, sizeof(m->filter_coeff[ch]));
        memset(m->filter_state[ch], 0, sizeof(m->filter_state[ch]));

        /* Default audio coding is 24-bit raw PCM */
        m->huff_offset[ch] = 0;
        m->codebook[ch] = 0;
        m->huff_lsbs[ch] = 24;
    }

    return 0;
}

/** Read parameters for one of the prediction filters
 */

static int read_filter_params(MLPDecodeContext *m, GetBitContext *gbp,
                              unsigned int channel, unsigned int filter)
{
    int i, order;

    // filter is 0 for FIR, 1 for IIR
    assert(filter < 2);

    order = get_bits(gbp, 4);
    if (order > MAX_FILTER_ORDER) {
        av_log(m->avctx, AV_LOG_ERROR,
               "%s filter order %d is greater than maximum %d\n",
               filter ? "IIR" : "FIR", order, MAX_FILTER_ORDER);
        return -1;
    }
    m->filter_order[channel][filter] = order;
    av_log(m->avctx, AV_LOG_DEBUG, "Filter %d, order %d\n", filter, order);

    if (order > 0) {
        int coeff_bits, coeff_shift;

        m->filter_coeff_q[channel][filter] = get_bits(gbp, 4);

        coeff_bits = get_bits(gbp, 5);
        coeff_shift = get_bits(gbp, 3);
        if (coeff_bits < 1 || coeff_bits > 16) {
            av_log(m->avctx, AV_LOG_ERROR,
                   "%s filter coeff_bits must be between 1 and 16\n",
                   filter ? "IIR" : "FIR");
            return -1;
        }
        if (coeff_bits + coeff_shift > 16) {
            av_log(m->avctx, AV_LOG_ERROR,
                   "Sum of coeff_bits and coeff_shift for %s filter must be 16 or less\n",
                   filter ? "IIR" : "FIR");
            return -1;
        }

        av_log(m->avctx, AV_LOG_DEBUG, "   coeff_q %d, (%d)<<%d\n",
               m->filter_coeff_q[channel][filter], coeff_bits, coeff_shift);
        for (i = 0; i < order; i++) {
            m->filter_coeff[channel][filter][i] =
                    get_sbits(gbp, coeff_bits) << coeff_shift;
            av_log(m->avctx, AV_LOG_DEBUG, "   coefficient %f\n",
                    ((float)m->filter_coeff[channel][filter][i])
                     / (1 << m->filter_coeff_q[channel][filter]));
        }

        if (get_bits1(gbp)) {
            int state_bits, state_shift;

            if (filter == 0) {
                av_log(m->avctx, AV_LOG_ERROR,
                       "FIR filter has state data specified\n");
                return -1;
            }

            state_bits = get_bits(gbp, 4);
            state_shift = get_bits(gbp, 4);

            /* TODO: check validity of state data */

            for (i = 0; i < order; i++) {
                m->filter_state[channel][filter][i] =
                    get_sbits(gbp, state_bits) << state_shift;
                av_log(m->avctx, AV_LOG_DEBUG, "   state 0x%x\n",
                       m->filter_state[channel][filter][i]);
            }
        }
    }

    return 0;
}

/** Read decoding parameters that change more often than those in the restart
 *  header
 */

static int read_decoding_params(MLPDecodeContext *m, GetBitContext *gbp,
                                unsigned int substr)
{
    unsigned int mat, ch;

    if (get_bits1(gbp))
        m->param_presence_flags[substr] = get_bits(gbp, 8);

    if (m->param_presence_flags[substr] & 0x80)
        if (get_bits1(gbp)) {
            m->blocksize[substr] = get_bits(gbp, 9);
            if (m->blocksize[substr] > MAX_BLOCKSIZE) {
                av_log(m->avctx, AV_LOG_ERROR, "Block size too large\n");
                return -1;
            }
        }

    if (m->param_presence_flags[substr] & 0x40)
        if (get_bits1(gbp)) {
            m->num_primitive_matrices[substr] = get_bits(gbp, 4);

            if (m->num_primitive_matrices[substr] > m->max_channel[substr] + 1) {
                av_log(m->avctx, AV_LOG_ERROR,
                       "More matrices specified than channels\n");
                return -1;
            }

            for (mat = 0; mat < m->num_primitive_matrices[substr]; mat++) {
                int frac_bits, max_chan;
                m->matrix_ch[substr][mat] = get_bits(gbp, 4);
                frac_bits = get_bits(gbp, 4);
                m->lsb_bypass[substr][mat] = get_bits1(gbp);

                if (m->matrix_ch[substr][mat] > m->max_channel[substr]) {
                    av_log(m->avctx, AV_LOG_ERROR,
                           "Invalid channel %d specified as output from matrix\n",
                           m->matrix_ch[substr][mat]);
                    return -1;
                }
                if (frac_bits > 14) {
                    av_log(m->avctx, AV_LOG_ERROR,
                           "Too many fractional bits specified\n");
                    return -1;
                }

                av_log(m->avctx, AV_LOG_DEBUG, "Matrix %d: ch %d (bypass %d)\n",
                       mat, m->matrix_ch[substr][mat],
                       m->lsb_bypass[substr][mat]);

                max_chan = m->max_matrix_channel[substr];
                if (m->restart_sync[substr] == 0x31ea)
                    max_chan+=2;

                for (ch = 0; ch <= max_chan; ch++) {
                    int coeff_val = 0;
                    if (get_bits1(gbp))
                        coeff_val = get_sbits(gbp, frac_bits + 2);

                    m->matrix_coeff[substr][mat][ch] = coeff_val << (14 - frac_bits);
                    av_log(m->avctx, AV_LOG_DEBUG,
                           "  Matrix coefficient (%d %x) %f\n",
                           frac_bits+2, coeff_val,
                           ((float)coeff_val) / (1 << frac_bits));
                }

                if (m->restart_sync[substr] == 0x31eb) {
                    m->matrix_noise_shift[substr][mat] = get_bits(gbp, 4);
                    av_log(m->avctx, AV_LOG_DEBUG,
                           "  mat noise shift %x\n",
                           m->matrix_noise_shift[substr][mat]);
                } else
                    m->matrix_noise_shift[substr][mat] = 0;
            }
        }

    if (m->param_presence_flags[substr] & 0x20)
        if (get_bits1(gbp)) {
            for (ch = 0; ch <= m->max_matrix_channel[substr]; ch++) {
                m->output_shift[substr][ch] = get_bits(gbp, 4);
                av_log(m->avctx, AV_LOG_DEBUG, "output shift[%d] = %d\n",
                       ch, m->output_shift[substr][ch]);
                /* TODO: validate */
            }
        }

    if (m->param_presence_flags[substr] & 0x10)
        if (get_bits1(gbp))
            for (ch = 0; ch <= m->max_channel[substr]; ch++) {
                m->quant_step_size[substr][ch] = get_bits(gbp, 4);
                av_log(m->avctx, AV_LOG_DEBUG, "quant_step_size[%d] = %d\n",
                       ch, m->quant_step_size[substr][ch]);
                /* TODO: validate */
            }

    for (ch = m->min_channel[substr]; ch <= m->max_channel[substr]; ch++)
        if (get_bits1(gbp)) {
            if (m->param_presence_flags[substr] & 0x08)
                if (get_bits1(gbp))
                    if (read_filter_params(m, gbp, ch, 0) < 0)
                        return -1;

            if (m->param_presence_flags[substr] & 0x04)
                if (get_bits1(gbp))
                    if (read_filter_params(m, gbp, ch, 1) < 0)
                        return -1;

            if (m->filter_order[ch][0] > 0 && m->filter_order[ch][1] > 0
                && m->filter_coeff_q[ch][0] != m->filter_coeff_q[ch][1]) {
                av_log(m->avctx, AV_LOG_ERROR,
                       "FIR and IIR filters must use same precision\n");
                return -1;
            }

            if (m->param_presence_flags[substr] & 0x02)
                if (get_bits1(gbp)) {
                    m->huff_offset[ch] = get_sbits(gbp, 15);
                    av_log(m->avctx, AV_LOG_DEBUG, "huff offset[%d] = 0x%x\n",
                           ch, m->huff_offset[ch]);
                }

            m->codebook[ch] = get_bits(gbp, 2);
            m->huff_lsbs[ch] = get_bits(gbp, 5);

            /* TODO: validate */

            av_log(m->avctx, AV_LOG_DEBUG, "codebook %d, lsbs %d\n",
                   m->codebook[ch], m->huff_lsbs[ch]);
        }

    return 0;
}

/** Generate a PCM sample using the prediction filters and a residual value
 *  read from the data stream, and update the filter state.
 */

static int filter_sample(MLPDecodeContext *m, unsigned int substr,
                         unsigned int channel, int32_t residual)
{
    unsigned int i;
    int64_t accum = 0;
    int32_t result;

    /* TODO: Move this code to DSPContext? */

    for (i = 0; i < m->filter_order[channel][0]; i++)
        accum += (int64_t)m->filter_state[channel][0][i] *
                 m->filter_coeff[channel][0][i];

    for (i = 0; i < m->filter_order[channel][1]; i++)
        accum += (int64_t) m->filter_state[channel][1][i] *
                 m->filter_coeff[channel][1][i];

    accum = accum >> m->filter_coeff_q[channel][0];
    result = (accum + residual)
                & ~((1 << m->quant_step_size[substr][channel]) - 1);

    memmove(&m->filter_state[channel][0][1], &m->filter_state[channel][0][0],
            sizeof(m->filter_state[channel][0][0]) * (MAX_FILTER_ORDER * 2 - 1));

    m->filter_state[channel][0][0] = result;
    m->filter_state[channel][1][0] = result - accum;

    return result;
}

/** Read a block of PCM residual (or actual if no filtering active) data
 */

static int read_block_data(MLPDecodeContext *m, GetBitContext *gbp,
                           unsigned int substr)
{
    unsigned int i, mat, ch, expected_stream_pos = 0;

    if (m->data_check_present[substr])
        expected_stream_pos = get_bits_count(gbp) + get_bits(gbp, 16);
        /* UNTESTED - find an example stream */

    av_log(m->avctx, AV_LOG_DEBUG, "Read data block\n");

    if (m->blockpos[substr] + m->blocksize[substr] > m->access_unit_size) {
        av_log(m->avctx, AV_LOG_ERROR, "Too many audio samples in frame\n");
        return -1;
    }

    memset(&m->bypassed_lsbs[m->blockpos[substr]][0], 0,
           m->blocksize[substr] * MAX_CHANNELS);

    for (i = 0; i < m->blocksize[substr]; i++) {
        for (mat = 0; mat < m->num_primitive_matrices[substr]; mat++)
            if (m->lsb_bypass[substr][mat])
                m->bypassed_lsbs[i + m->blockpos[substr]][mat] = get_bits1(gbp);

        for (ch = m->min_channel[substr]; ch <= m->max_channel[substr]; ch++) {
            int32_t sample = read_huff(m, gbp, substr, ch);
            int32_t filtered = filter_sample(m, substr, ch, sample);

            m->sample_buffer[i + m->blockpos[substr]][ch] = filtered;
        }
    }

    m->blockpos[substr] += m->blocksize[substr];

    if (m->data_check_present[substr]) {
        if (get_bits_count(gbp) != expected_stream_pos)
            av_log(m->avctx, AV_LOG_ERROR, "Block data length mismatch\n");
        skip_bits(gbp, 8);
    }

    return 0;
}

/** Data table used for TrueHD noise generation function */

static int8_t noise_table[256] = {
     30,  51,  22,  54,   3,   7,  -4,  38,  14,  55,  46,  81,  22,  58,  -3,   2,
     52,  31,  -7,  51,  15,  44,  74,  30,  85, -17,  10,  33,  18,  80,  28,  62,
     10,  32,  23,  69,  72,  26,  35,  17,  73,  60,   8,  56,   2,   6,  -2,  -5,
     51,   4,  11,  50,  66,  76,  21,  44,  33,  47,   1,  26,  64,  48,  57,  40,
     38,  16, -10, -28,  92,  22, -18,  29, -10,   5, -13,  49,  19,  24,  70,  34,
     61,  48,  30,  14,  -6,  25,  58,  33,  42,  60,  67,  17,  54,  17,  22,  30,
     67,  44,  -9,  50, -11,  43,  40,  32,  59,  82,  13,  49, -14,  55,  60,  36,
     48,  49,  31,  47,  15,  12,   4,  65,   1,  23,  29,  39,  45,  -2,  84,  69,
      0,  72,  37,  57,  27,  41, -15, -16,  35,  31,  14,  61,  24,   0,  27,  24,
     16,  41,  55,  34,  53,   9,  56,  12,  25,  29,  53,   5,  20, -20,  -8,  20,
     13,  28,  -3,  78,  38,  16,  11,  62,  46,  29,  21,  24,  46,  65,  43, -23,
     89,  18,  74,  21,  38, -12,  19,  12, -19,   8,  15,  33,   4,  57,   9,  -8,
     36,  35,  26,  28,   7,  83,  63,  79,  75,  11,   3,  87,  37,  47,  34,  40,
     39,  19,  20,  42,  27,  34,  39,  77,  13,  42,  59,  64,  45,  -1,  32,  37,
     45,  -5,  53,  -6,   7,  36,  50,  23,   6,  32,   9, -21,  18,  71,  27,  52,
    -25,  31,  35,  42,  -1,  68,  63,  52,  26,  43,  66,  37,  41,  25,  40,  70,
};

/** Noise generation functions.
 *  I'm not sure what these are for - they seem to be some kind of pseudorandom
 *  sequence generators, used to generate noise data which is used when the
 *  channels are rematrixed. I'm not sure if they provide a practical benefit
 *  to compression, or just obfuscate the decoder. Are they for some kind of
 *  dithering?
 */

/** Generate two channels of noise, used in the matrix when restart_sync == 0x31ea */

static void generate_noise_1(MLPDecodeContext *m, unsigned int substr)
{
    unsigned int i;
    uint32_t tmp;
    uint32_t seed = m->noisegen_seed[substr];
    unsigned int maxchan = m->max_matrix_channel[substr];

    for (i = 0; i < m->blockpos[substr]; i++) {
        m->sample_buffer[i][maxchan+1] = ((int8_t)(seed >> 15)) << m->noise_shift[substr];
        m->sample_buffer[i][maxchan+2] = ((int8_t)(seed >> 7)) << m->noise_shift[substr];

        tmp = seed >> 7;
        seed = (seed << 16) ^ tmp ^ (tmp << 5);
        seed &= 0x7fffff;
    }

    m->noisegen_seed[substr] = seed;
}

/** Generate a block of noise, used when restart_sync == 0x31eb */

static void generate_noise_2(MLPDecodeContext *m, unsigned int substr)
{
    unsigned int i;
    uint32_t tmp;
    uint32_t seed = m->noisegen_seed[substr];

    for (i = 0; i < m->access_unit_size_pow2; i++) {
        tmp = seed >> 15;
        m->noise_buffer[i] = noise_table[tmp];
        seed = (seed << 8) ^ tmp ^ (tmp << 5);
        seed &= 0x7fffff;
    }

    m->noisegen_seed[substr] = seed;
}


/** Apply the channel matrices in turn to reconstruct the original audio samples.
 */

static void rematrix_channels(MLPDecodeContext *m, unsigned int substr)
{
    unsigned int mat, dest_ch, src_ch, i;
    unsigned int maxchan;

    maxchan = m->max_matrix_channel[substr];
    if (m->restart_sync[substr] == 0x31ea) {
        generate_noise_1(m, substr);
        maxchan += 2;
    } else {
        generate_noise_2(m, substr);
    }

    for (mat = 0; mat < m->num_primitive_matrices[substr]; mat++) {
        dest_ch = m->matrix_ch[substr][mat];

        /* TODO: DSPContext? */

        for (i = 0; i < m->blockpos[substr]; i++) {
            int64_t accum = 0;
            for (src_ch = 0; src_ch <= maxchan; src_ch++) {
                accum += (int64_t)m->sample_buffer[i][src_ch]
                                  * m->matrix_coeff[substr][mat][src_ch];
            }
            if (m->matrix_noise_shift[substr][mat]) {
                uint32_t index = m->num_primitive_matrices[substr] - mat;
                index = (i * (index * 2 + 1) + index) & (m->access_unit_size_pow2 - 1);
                accum += m->noise_buffer[index] << (m->matrix_noise_shift[substr][mat] + 7);
            }
            m->sample_buffer[i][dest_ch] = ((accum >> 14) & ~((1 << m->quant_step_size[substr][dest_ch]) - 1))
                                             + m->bypassed_lsbs[i][mat];
        }
    }
}

/** Write the audio data into the output buffer.
 */

static int output_data(MLPDecodeContext *m, unsigned int substr)
{
    unsigned int i, ch;
    int32_t sample;
    int16_t *sample_buf16 = (int16_t*)(m->out_buf + m->bytes_output);

#ifdef CONFIG_AUDIO_NONSHORT
    int32_t *sample_buf32 = (int32_t*)(m->out_buf + m->bytes_output);

    if (m->avctx->sample_fmt == SAMPLE_FMT_S32) {
        for (i = 0; i < m->blockpos[substr]; i++) {
            for (ch = 0; ch <= m->max_channel[substr]; ch++) {
                sample = m->sample_buffer[i][ch] << m->output_shift[substr][ch];
                m->lossless_check_data[substr] ^= (sample & 0xffffff) << ch;
                *sample_buf32++ = sample << 8;
                m->bytes_output += 4;
            }
        }
    } else
#endif
    {
        for (i = 0; i < m->blockpos[substr]; i++) {
            for (ch = 0; ch <= m->max_channel[substr]; ch++) {
                sample = m->sample_buffer[i][ch] << m->output_shift[substr][ch];
                m->lossless_check_data[substr] ^= (sample & 0xffffff) << ch;
                *sample_buf16++ = (sample) >> 8;
                m->bytes_output += 2;
            }
        }
    }

    return 0;
}


/** XOR together all the bytes of a buffer.
 *  Does this belong in dspcontext?
 */

static uint8_t calculate_parity(const uint8_t *buf, unsigned int buf_size)
{
    uint32_t scratch = 0;

    while (buf_size >= 4) {
        scratch ^= *((const uint32_t*)buf);
        buf += 4;
        buf_size -= 4;
    }
    scratch ^= scratch >> 16;
    scratch ^= scratch >> 8;
    scratch &= 0xff;

    while (buf_size > 0) {
        scratch ^= *buf;
        buf++;
        buf_size--;
    }

    return scratch;
}

/**
 * Read an access unit from the stream.
 * Returns -1 on error, 0 if not enough data is present in the input stream
 * otherwise returns the number of bytes consumed.
 */

static int read_access_unit(MLPDecodeContext *m, const uint8_t *buf,
                            unsigned int buf_size)
{
    GetBitContext gb;
    unsigned int length, substr;
    unsigned int substream_start;

    if (buf_size < 2)
        return 0;

    if (m->out_buf_remaining < m->avctx->channels * m->access_unit_size
                               * (m->avctx->sample_fmt == SAMPLE_FMT_S32 ? 4 : 2))
        return -1;

    length = AV_RB16(buf) & 0xfff;

    if (length * 2 > buf_size)
        return 0;

    init_get_bits(&gb, buf, length * 16);
    skip_bits_long(&gb, 32);

    av_log(m->avctx, AV_LOG_DEBUG, "Read access unit, length %d words.\n",
           length);

    if ((show_bits_long(&gb, 31) << 1) == 0xf8726fba) {
        av_log(m->avctx, AV_LOG_DEBUG, "Found major sync\n");
        if (read_major_sync(m, buf + 4, buf_size - 4) < 0)
            goto error;
        skip_bits_long(&gb, 28 * 8);
    }

    if (!m->in_sync)
        return length * 2;

    substream_start = 0;

    for (substr = 0; substr < m->num_substreams; substr++) {
        int extraword_present, restart_absent, checkdata_present, end;

        extraword_present = get_bits1(&gb);
        restart_absent = get_bits1(&gb);
        checkdata_present = get_bits1(&gb);
        skip_bits1(&gb);

        end = get_bits(&gb, 12);

        if (extraword_present)
            skip_bits(&gb, 16);

        av_log(m->avctx, AV_LOG_DEBUG, "Substream %d, end at %d\n", substr, end);

        if (substr > m->max_decoded_substream)
            continue;

        m->restart_header_present[substr] = !restart_absent;
        m->substream_parity_present[substr] = checkdata_present;
        m->substream_data_len[substr] = end - substream_start;
        substream_start = end;

        if (restart_absent && !m->restart_seen[substr]) {
            av_log(m->avctx, AV_LOG_ERROR,
                   "No restart header indicated for substream %d", substr);
            goto error;
        }
    }

    buf += get_bits_count(&gb) >> 3;
    buf_size -= get_bits_count(&gb) >> 3;

    for (substr = 0; substr <= m->max_decoded_substream; substr++) {
        init_get_bits(&gb, buf, m->substream_data_len[substr] * 16);

        m->blockpos[substr] = 0;
        do {
            if (get_bits1(&gb)) {
                if (get_bits1(&gb)) {
                    /* A restart header should be present */
                    if (read_restart_header(m, &gb, buf, substr) < 0)
                        goto error;
                    if (!m->restart_header_present[substr])
                        av_log(m->avctx, AV_LOG_INFO, "Restart header present but not indicated in block header.\n");
                    m->restart_seen[substr] = 1;
                    m->restart_header_present[substr] = 0;
                } else if (m->restart_header_present[substr]) {
                    av_log(m->avctx, AV_LOG_INFO, "Restart header indicated in block header but not present.\n");
                    if (!m->restart_seen[substr])
                        goto error;
                }

                if (read_decoding_params(m, &gb, substr) < 0)
                    goto error;
            }

            if (read_block_data(m, &gb, substr) < 0)
                goto error;

        } while ((get_bits_count(&gb) < m->substream_data_len[substr] * 16)
                 && get_bits1(&gb) == 0);

        skip_bits(&gb, (-get_bits_count(&gb)) & 15);
        if (show_bits_long(&gb, 32) == 0xd234d234) {
            av_log(m->avctx, AV_LOG_INFO, "End of stream indicated\n");
            skip_bits(&gb, 32);
        }
        if (m->substream_parity_present[substr])
        {
            uint8_t parity, checksum;

            parity = calculate_parity(buf, m->substream_data_len[substr] * 2 - 2);
            if ((parity ^ get_bits(&gb, 8)) != 0xa9)
                av_log(m->avctx, AV_LOG_ERROR,
                       "Substream %d parity check failed\n", substr);

            checksum = mlp_checksum8(buf, m->substream_data_len[substr] * 2 - 2);
            if (checksum != get_bits(&gb, 8))
                av_log(m->avctx, AV_LOG_ERROR, "Substream %d checksum failed\n",
                       substr);
        }
        if (m->substream_data_len[substr] * 16 != get_bits_count(&gb)) {
            av_log(m->avctx, AV_LOG_ERROR, "Substream %d length mismatch.\n",
                   substr);
            goto error;
        }

        buf += m->substream_data_len[substr] * 2;
        buf_size -= m->substream_data_len[substr] * 2;
    }

    rematrix_channels(m, substr - 1);
    output_data(m, substr - 1);

    return length * 2;

error:
    m->in_sync = 0;
    return -1;
}

static int mlp_decode_frame(AVCodecContext *avctx,
                            void *data, int *data_size,
                            uint8_t *buf, int buf_size)
{
    MLPDecodeContext *m = avctx->priv_data;
    int ret;

    m->out_buf = data;
    m->out_buf_remaining = *data_size;
    m->bytes_output = 0;

    ret = read_access_unit(m, buf, buf_size);

    if (ret <= 0)
        return -1;

    *data_size = m->bytes_output;
    return ret;
}

AVCodec mlp_decoder = {
    "mlp",
    CODEC_TYPE_AUDIO,
    CODEC_ID_MLP,
    sizeof(MLPDecodeContext),
    mlp_decode_init,
    NULL,
    NULL,
    mlp_decode_frame,
};


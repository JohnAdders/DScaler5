///////////////////////////////////////////////////////////////////////////////
// $Id
///////////////////////////////////////////////////////////////////////////////
// MpegAudio.dll - DirectShow filter for decoding Mpeg audio streams
// Copyright (c) 2004 John Adcock
///////////////////////////////////////////////////////////////////////////////
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
///////////////////////////////////////////////////////////////////////////////

#pragma once 

#define DITHER_COUNT 2048
extern long DitherTable[2048];
extern long DitherCounter;
#define DITHER_MASK  (2048 - 1)

#define CREATE_CONVERT_TO_16(BITSINSAMPLE) \
static __inline void Convert##BITSINSAMPLE##To16(BYTE*& pOutput, long Sample) \
{\
    /* Dither - add some 2Q triangular noise */ \
    Sample += DitherTable[((DitherCounter++)&DITHER_MASK)] >> (32 - BITSINSAMPLE); \
    /* clip */ \
    if (Sample >= (1 << (BITSINSAMPLE - 1))) \
        Sample = (1 << (BITSINSAMPLE - 1)) - 1; \
    else if (Sample < -(1 << (BITSINSAMPLE - 1))) \
        Sample = -(1 << (BITSINSAMPLE - 1)); \
    /* quantize */ \
    Sample = Sample >> (BITSINSAMPLE - 16); \
    *pOutput++ = (BYTE)(Sample); \
    *pOutput++ = (BYTE)(Sample>>8); \
}

#define CREATE_CONVERT_TO_24(BITSINSAMPLE) \
static __inline void Convert##BITSINSAMPLE##To24(BYTE*& pOutput, long Sample) \
{\
    /* clip */ \
    if (Sample >= (1 << (BITSINSAMPLE - 1))) \
    Sample = (1 << (BITSINSAMPLE - 1)) - 1; \
    else if (Sample < -(1 << (BITSINSAMPLE - 1))) \
    Sample = -(1 << (BITSINSAMPLE - 1)); \
    /* quantize */ \
    Sample = Sample >> (BITSINSAMPLE - 24); \
    *pOutput++ = (BYTE)(Sample); \
    *pOutput++ = (BYTE)(Sample>>8); \
    *pOutput++ = (BYTE)(Sample>>16); \
}

#define CREATE_CONVERT_TO_32(BITSINSAMPLE) \
static __inline void Convert##BITSINSAMPLE##To32(BYTE*& pOutput, long Sample) \
{\
    /* clip */ \
    if (Sample >= (1 << (BITSINSAMPLE - 1))) \
    Sample = (1 << (BITSINSAMPLE - 1)) - 1; \
    else if (Sample < -(1 << (BITSINSAMPLE - 1))) \
    Sample = -(1 << (BITSINSAMPLE - 1)); \
    /* requantize */ \
    Sample = Sample << (32 - BITSINSAMPLE); \
    *pOutput++ = (BYTE)(Sample); \
    *pOutput++ = (BYTE)(Sample>>8); \
    *pOutput++ = (BYTE)(Sample>>16); \
    *pOutput++ = (BYTE)(Sample>>24); \
}


#define CREATE_CONVERT_TO_FLOAT(BITSINSAMPLE) \
static __inline void Convert##BITSINSAMPLE##ToFloat(BYTE*& pOutput, long Sample) \
{\
    /* clip */ \
    if (Sample >= (1 << (BITSINSAMPLE - 1))) \
    Sample = (1 << (BITSINSAMPLE - 1)) - 1; \
    else if (Sample < -(1 << (BITSINSAMPLE - 1))) \
    Sample = -(1 << (BITSINSAMPLE - 1)); \
    /* requantize */ \
    *(float*)pOutput = (float)Sample / (float)(1 << (BITSINSAMPLE - 1)); \
    pOutput += 4; \
}

static __inline void ConvertDoubleTo16(BYTE*& pOutput, double Sample)
{
    long lSample = (long)(Sample * (double)(1<<23));

    // Dither - add some 2Q triangular noise
    lSample += DitherTable[((DitherCounter++)&DITHER_MASK)] >> 8;

    if(lSample > 0x7FFFFF)
    {
        lSample = 0x7FFFFF;
    }
    else if(lSample < -(1<<23))
    {
        lSample = -(1<<23);
    }
    *pOutput++ = (BYTE)(lSample>>8);
    *pOutput++ = (BYTE)(lSample>>16);
}

static __inline void ConvertDoubleTo24(BYTE*& pOutput, double Sample)
{
    long lSample;
    if(Sample >= 1.0)
    {
        lSample = 0x7FFFFFFF;
    }
    else if(Sample < -1.0)
    {
        lSample = -(1<<31);
    }
    else
    {
        lSample = (long)(Sample * (double)(1<<31));
    }
    *pOutput++ = (BYTE)(lSample>>8);
    *pOutput++ = (BYTE)(lSample>>16);
    *pOutput++ = (BYTE)(lSample>>24);
}

static __inline void ConvertDoubleTo32(BYTE*& pOutput, double Sample)
{
    long lSample;
    if(Sample >= 1.0)
    {
        lSample = 0x7FFFFFFF;
    }
    else if(Sample < -1.0)
    {
        lSample = -(1<<31);
    }
    else
    {
        lSample = (long)(Sample * (double)(1<<31));
    }
    *pOutput++ = (BYTE)(lSample);
    *pOutput++ = (BYTE)(lSample>>8);
    *pOutput++ = (BYTE)(lSample>>16);
    *pOutput++ = (BYTE)(lSample>>24);
}

static __inline void ConvertFloatTo16(BYTE*& pOutput, float Sample)
{
    long lSample = (long)(Sample * (float)(1<<23));

    // Dither - add some 2Q triangular noise
    lSample += DitherTable[((DitherCounter++)&DITHER_MASK)] >> 8;

    if(lSample > 0x7FFFFF)
    {
        lSample = 0x7FFFFF;
    }
    else if(lSample < -(1<<23))
    {
        lSample = -(1<<23);
    }
    *pOutput++ = (BYTE)(lSample>>8);
    *pOutput++ = (BYTE)(lSample>>16);
}

static __inline void ConvertFloatTo24(BYTE*& pOutput, float Sample)
{
    long lSample;
    if(Sample >= 1.0)
    {
        lSample = 0x7FFFFFFF;
    }
    else if(Sample < -1.0)
    {
        lSample = -(1<<31);
    }
    else
    {
        lSample = (long)(Sample * (float)(1<<31));
    }

    *pOutput++ = (BYTE)(lSample>>8);
    *pOutput++ = (BYTE)(lSample>>16);
    *pOutput++ = (BYTE)(lSample>>24);
}

static __inline void ConvertFloatTo32(BYTE*& pOutput, float Sample)
{
    long lSample;
    if(Sample >= 1.0)
    {
        lSample = 0x7FFFFFFF;
    }
    else if(Sample < -1.0)
    {
        lSample = -(1<<31);
    }
    else
    {
        lSample = (long)(Sample * (float)(1<<31));
    }
    *pOutput++ = (BYTE)(lSample);
    *pOutput++ = (BYTE)(lSample>>8);
    *pOutput++ = (BYTE)(lSample>>16);
    *pOutput++ = (BYTE)(lSample>>24);
}


static __inline void ConvertFloatToFloat(BYTE*& pOutput, float Sample)
{
    *(float*)pOutput = Sample;
    pOutput += 4;
}

static __inline void ConvertDoubleToFloat(BYTE*& pOutput, double Sample)
{
    
    if(Sample >= 1.0)
    {
        Sample = 1.0;
    }
    else if(Sample < -1.0)
    {
        Sample = -1.0;
    }
     *(float*)pOutput = (float)Sample;
    pOutput += 4;
}

static __inline void Convert16ToFloat(BYTE*& pOutput, short Sample)
{
    float* test = (float*)pOutput;
    *test = (float)Sample / float(1 << 15);
    pOutput += 4;
}


static __inline void Convert16To32(BYTE*& pOutput, short Sample)
{
    *pOutput++ = 0;
    *pOutput++ = 0;
    *pOutput++ = (BYTE)(Sample);
    *pOutput++ = (BYTE)(Sample>>8);
}

static __inline void Convert16To24(BYTE*& pOutput, short Sample)
{
    *pOutput++ = 0;
    *pOutput++ = (BYTE)(Sample);
    *pOutput++ = (BYTE)(Sample>>8);
}

static __inline void Convert16To16(BYTE*& pOutput, short Sample)
{
    *pOutput++ = (BYTE)(Sample);
    *pOutput++ = (BYTE)(Sample>>8);
}

static __inline void Convert24ToFloat(BYTE*& pOutput, long Sample)
{
    *(float*)pOutput = (float)Sample / float(1 << 23);
    pOutput += 4;
}

static __inline void Convert24To32(BYTE*& pOutput, long Sample)
{
    *pOutput++ = 0;
    *pOutput++ = (BYTE)(Sample);
    *pOutput++ = (BYTE)(Sample>>8);
    *pOutput++ = (BYTE)(Sample>>16);
}

static __inline void Convert24To24(BYTE*& pOutput, long Sample)
{
    *pOutput++ = (BYTE)(Sample);
    *pOutput++ = (BYTE)(Sample>>8);
    *pOutput++ = (BYTE)(Sample>>16);
}

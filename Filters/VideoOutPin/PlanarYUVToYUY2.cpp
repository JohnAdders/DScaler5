///////////////////////////////////////////////////////////////////////////////
// $Id$
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2004 John Adcock
//
//  This file based on code taken from vd.cpp in the guliverkli project
//  which in turn took it from
//  VirtualDub - Video processing and capture application
//  Copyright (C) 1998-2001 Avery Lee
//
///////////////////////////////////////////////////////////////////////////////
//
//  This Program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2, or (at your option)
//  any later version.
//
//  This Program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with GNU Make; see the file COPYING.  If not, write to
//  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//  http://www.gnu.org/copyleft/gpl.html
//
///////////////////////////////////////////////////////////////////////////////
//  - BitBltFromI420ToRGB is from VirtualDub
//  - The core assembly function of CCpuID is from DVD2AVI
//  (- vd.cpp/h should be renamed to something more sensible already :)
///////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "PlanarYUVToYUY2.h"
#include "CPUID.h"

#pragma warning(disable : 4799) // no emms... blahblahblah

void memcpy_accel(void* dst, const void* src, size_t len);

extern "C"
{
    void yuvtoyuy2row_MMX(BYTE* dst, BYTE* srcy, BYTE* srcu, BYTE* srcv, DWORD width);
    void yuvtoyuy2row_avg_MMX(BYTE* dst, BYTE* srcy, BYTE* srcu, BYTE* srcv, DWORD width, DWORD pitchuv);
    void yuvtoyuy2row_avg_SSEMMX(BYTE* dst, BYTE* srcy, BYTE* srcu, BYTE* srcv, DWORD width, DWORD pitchuv);
    void yuvtoyuy2row_avg2_MMX(BYTE* dst, BYTE* srcy, BYTE* srcu, BYTE* srcv, DWORD width, DWORD pitchuv);
    void yuvtoyuy2row_avg2_SSEMMX(BYTE* dst, BYTE* srcy, BYTE* srcu, BYTE* srcv, DWORD width, DWORD pitchuv);
    void memcpy_accel_SSE(void* dst, const void* src, size_t len);
    void memcpy_accel_MMX(void* dst, const void* src, size_t len);
}

bool BitBltFromI420ToI420(int w, int h, BYTE* dsty, BYTE* dstu, BYTE* dstv, int dstpitch, BYTE* srcy, BYTE* srcu, BYTE* srcv, int srcpitch)
{
    if(w&1) return(false);

    int pitch = min(abs(srcpitch), abs(dstpitch));

    int y;
    for(y = 0; y < h; y++, srcy += srcpitch, dsty += dstpitch)
        memcpy_accel(dsty, srcy, pitch);

    srcpitch >>= 1;
    dstpitch >>= 1;

    pitch = min(abs(srcpitch), abs(dstpitch));

    for(y = 0; y < h; y+=2, srcu += srcpitch, dstu += dstpitch)
        memcpy_accel(dstu, srcu, pitch);

    for(y = 0; y < h; y+=2, srcv += srcpitch, dstv += dstpitch)
        memcpy_accel(dstv, srcv, pitch);

    if(CpuFeatureFlags & FEATURE_MMX)
        EndMMX();

    return(true);
}

bool BitBltFromI420ToNV12(int w, int h, BYTE* dsty, BYTE* dstuv, int dstpitch, BYTE* srcy, BYTE* srcu, BYTE* srcv, int srcpitch)
{
    if(w&1) return(false);

    int pitch = min(abs(srcpitch), abs(dstpitch));

    int y;
    for(y = 0; y < h; y++, srcy += srcpitch, dsty += dstpitch)
        memcpy_accel(dsty, srcy, pitch);

    pitch >>= 1;
    srcpitch >>= 1;


    for(y = 0; y < h; y+=2, srcu += srcpitch, srcv += srcpitch, dstuv += dstpitch)
    {
        for(int x(0); x < pitch; ++x)
        {
            dstuv[2*x] = srcu[x];
            dstuv[2*x + 1] = srcv[x];
        }
    }

    if(CpuFeatureFlags & FEATURE_MMX)
        EndMMX();

    return(true);
}


bool BitBltFromI422ToI420(int w, int h, BYTE* dsty, BYTE* dstu, BYTE* dstv, int dstpitch, BYTE* srcy, BYTE* srcu, BYTE* srcv, int srcpitch)
{
    if(w&1) return(false);

    int pitch = min(abs(srcpitch), abs(dstpitch));

    int y;
    for(y = 0; y < h; y++, srcy += srcpitch, dsty += dstpitch)
        memcpy_accel(dsty, srcy, pitch);

    srcpitch >>= 1;
    dstpitch >>= 1;

    pitch = min(abs(srcpitch), abs(dstpitch));

    for(y = 0; y < h; y+=2, srcu += 2 * srcpitch, dstu += dstpitch)
        memcpy_accel(dstu, srcu, pitch);

    for(y = 0; y < h; y+=2, srcv += 2 * srcpitch, dstv += dstpitch)
        memcpy_accel(dstv, srcv, pitch);

    if(CpuFeatureFlags & FEATURE_MMX)
        EndMMX();

    return(true);
}

// \todo optimise and filter
// we should probably do much better than this simple decimation
// but this is really just here for completeness
bool BitBltFromI444ToI420(int w, int h, BYTE* dsty, BYTE* dstu, BYTE* dstv, int dstpitch, BYTE* srcy, BYTE* srcu, BYTE* srcv, int srcpitch)
{
    if(w&1) return(false);

    int pitch = min(abs(srcpitch), abs(dstpitch));

    int y;
    int z;
    for(y = 0; y < h; y++, srcy += srcpitch, dsty += dstpitch)
        memcpy_accel(dsty, srcy, pitch);

    dstpitch >>= 1;

    pitch = min(abs(srcpitch)>>1, abs(dstpitch));

    for(y = 0; y < h; y+=2, srcu += 2 * srcpitch, dstu += dstpitch)
        for(z = 0; z < pitch; z++)
            dstu[z] = srcu[z<<1];

    for(y = 0; y < h; y+=2, srcv += 2 * srcpitch, dstv += dstpitch)
        for(z = 0; z < pitch; z++)
            dstv[z] = srcv[z<<1];

    return(true);
}


bool BitBltFromYUY2ToYUY2(int w, int h, BYTE* dst, int dstpitch, BYTE* src, int srcpitch)
{
    int pitch = min(abs(srcpitch), abs(dstpitch));

    for(int y = 0; y < h; y++, src += srcpitch, dst += dstpitch)
        memcpy_accel(dst, src, pitch);

    if(CpuFeatureFlags & FEATURE_MMX)
        EndMMX();

    return(true);
}

static void yuvtoyuy2row_c(BYTE* dst, BYTE* srcy, BYTE* srcu, BYTE* srcv, DWORD width)
{
    WORD* dstw = (WORD*)dst;
    for(; width > 1; width -= 2)
    {
        *dstw++ = (*srcu++<<8)|*srcy++;
        *dstw++ = (*srcv++<<8)|*srcy++;
    }
}

static void yuv444toyuy2row_c(BYTE* dst, BYTE* srcy, BYTE* srcu, BYTE* srcv, DWORD width)
{
    WORD* dstw = (WORD*)dst;
    for(; width > 1; width -= 2)
    {
        *dstw++ = (*srcu++<<8)|*srcy++;
        *dstw++ = (*srcv++<<8)|*srcy++;
        srcu++;
        srcv++;
    }
}


static void yuvtoyuy2row_avg_c(BYTE* dst, BYTE* srcy, BYTE* srcu, BYTE* srcv, DWORD width, DWORD pitchuv)
{
    WORD* dstw = (WORD*)dst;
    for(; width > 1; width -= 2, srcu++, srcv++)
    {
        *dstw++ = (((srcu[0]+srcu[pitchuv])>>1)<<8)|*srcy++;
        *dstw++ = (((srcv[0]+srcv[pitchuv])>>1)<<8)|*srcy++;
    }
}

static void yuvtoyuy2row_avg2_c(BYTE* dst, BYTE* srcy, BYTE* srcu, BYTE* srcv, DWORD width, DWORD pitchuv)
{
    WORD* dstw = (WORD*)dst;
    for(; width > 1; width -= 2, srcu++, srcv++)
    {
        *dstw++ = (((3*srcu[0]+srcu[pitchuv])>>2)<<8)|*srcy++;
        *dstw++ = (((3*srcv[0]+srcv[pitchuv])>>2)<<8)|*srcy++;
    }
}

bool BitBltFromI420ToYUY2(int w, int h, BYTE* dst, int dstpitch, BYTE* srcy, BYTE* srcu, BYTE* srcv, int srcpitch)
{
    if(w<=0 || h<=0 || (w&1) || (h&1))
        return(false);

    if(srcpitch == 0) srcpitch = w;

    void (*yuvtoyuy2row)(BYTE* dst, BYTE* srcy, BYTE* srcu, BYTE* srcv, DWORD width) = NULL;
    void (*yuvtoyuy2row_avg)(BYTE* dst, BYTE* srcy, BYTE* srcu, BYTE* srcv, DWORD width, DWORD pitchuv) = NULL;

    if((CpuFeatureFlags & FEATURE_MMX) && !(w&7))
    {
        yuvtoyuy2row = yuvtoyuy2row_MMX;
        yuvtoyuy2row_avg = yuvtoyuy2row_avg_MMX;
    }
    else
    {
        yuvtoyuy2row = yuvtoyuy2row_c;
        yuvtoyuy2row_avg = yuvtoyuy2row_avg_c;
    }

    if(!yuvtoyuy2row)
        return(false);

    do
    {
        yuvtoyuy2row(dst, srcy, srcu, srcv, w);
        yuvtoyuy2row_avg(dst + dstpitch, srcy + srcpitch, srcu, srcv, w, srcpitch/2);

        dst += 2*dstpitch;
        srcy += srcpitch*2;
        srcu += srcpitch/2;
        srcv += srcpitch/2;
    }
    while((h -= 2) > 2);

    yuvtoyuy2row(dst, srcy, srcu, srcv, w);
    yuvtoyuy2row(dst + dstpitch, srcy + srcpitch, srcu, srcv, w);

    if(CpuFeatureFlags & FEATURE_MMX)
        EndMMX();

    return(true);
}

bool BitBltFromI420ToYUY2_Int(int w, int h, BYTE* dst, int dstpitch, BYTE* srcy, BYTE* srcu, BYTE* srcv, int srcpitch)
{
    // need at least a mulitple of 4 for height
    if(w<=0 || h<=0 || (w&1) || (h&3))
        return(false);

    // we need at least 12 rows to work on
    if(h < 12)
    {
        BitBltFromI420ToYUY2(w, h, dst, dstpitch, srcy, srcu, srcv, srcpitch);
    }

    if(srcpitch == 0) srcpitch = w;

    void (*yuvtoyuy2row)(BYTE* dst, BYTE* srcy, BYTE* srcu, BYTE* srcv, DWORD width) = NULL;
    void (*yuvtoyuy2row_avg)(BYTE* dst, BYTE* srcy, BYTE* srcu, BYTE* srcv, DWORD width, DWORD pitchuv) = NULL;
    void (*yuvtoyuy2row_avg2)(BYTE* dst, BYTE* srcy, BYTE* srcu, BYTE* srcv, DWORD width, DWORD pitchuv) = NULL;

    if(((CpuFeatureFlags & FEATURE_MMXEXT) || (CpuFeatureFlags & FEATURE_3DNOW)) && !(w&7))
    {
        yuvtoyuy2row = yuvtoyuy2row_MMX;
        yuvtoyuy2row_avg = yuvtoyuy2row_avg_MMX;
        yuvtoyuy2row_avg2 = yuvtoyuy2row_avg2_SSEMMX;
    }
    else if((CpuFeatureFlags & FEATURE_MMX) && !(w&7))
    {
        yuvtoyuy2row = yuvtoyuy2row_MMX;
        yuvtoyuy2row_avg = yuvtoyuy2row_avg_MMX;
        yuvtoyuy2row_avg2 = yuvtoyuy2row_avg2_MMX;
    }
    else
    {
        yuvtoyuy2row = yuvtoyuy2row_c;
        yuvtoyuy2row_avg = yuvtoyuy2row_avg_c;
        yuvtoyuy2row_avg2 = yuvtoyuy2row_avg2_c;
    }

    if(!yuvtoyuy2row)
        return(false);

    // copy first chroma row and first luma row for first row
    yuvtoyuy2row(dst, srcy, srcu, srcv, w);
    dst += dstpitch;
    srcy += srcpitch;

    // copy second chroma row and second luma row for second row
    yuvtoyuy2row(dst, srcy, srcu + srcpitch/2, srcv + srcpitch/2, w);
    dst += dstpitch;
    srcy += srcpitch;

    h -= 2;

    do
    {
        // average between rows for field 1
        yuvtoyuy2row_avg(dst, srcy, srcu, srcv, w, srcpitch);
        dst += dstpitch;
        srcy += srcpitch;

        // put in chroma for field 2 line
        yuvtoyuy2row(dst, srcy, srcu + srcpitch/2, srcv + srcpitch/2, w);
        dst += dstpitch;
        srcy += srcpitch;

        // put in chroma for field 1 line
        yuvtoyuy2row(dst, srcy, srcu + srcpitch, srcv + srcpitch, w);
        dst += dstpitch;
        srcy += srcpitch;

        // average between rows for field 2
        yuvtoyuy2row_avg(dst, srcy, srcu + srcpitch/2, srcv + srcpitch/2, w, srcpitch);
        dst += dstpitch;
        srcy += srcpitch;

        srcu += srcpitch;
        srcv += srcpitch;
    }
    while((h -= 4) > 2);

    yuvtoyuy2row(dst, srcy, srcu , srcv, w);
    yuvtoyuy2row(dst + dstpitch, srcy + srcpitch, srcu + srcpitch/2, srcv + srcpitch/2, w);

    if(CpuFeatureFlags & FEATURE_MMX)
        EndMMX();

    return(true);
}

bool BitBltFromI422ToYUY2(int w, int h, BYTE* dst, int dstpitch, BYTE* srcy, BYTE* srcu, BYTE* srcv, int srcpitch)
{
    if(w<=0 || h<=0 || (w&1) || (h&1))
        return(false);

    if(srcpitch == 0) srcpitch = w;

    void (*yuvtoyuy2row)(BYTE* dst, BYTE* srcy, BYTE* srcu, BYTE* srcv, DWORD width) = NULL;

    if((CpuFeatureFlags & FEATURE_MMX) && !(w&7))
    {
        yuvtoyuy2row = yuvtoyuy2row_MMX;
    }
    else
    {
        yuvtoyuy2row = yuvtoyuy2row_c;
    }

    if(!yuvtoyuy2row)
        return(false);

    do
    {
        yuvtoyuy2row(dst, srcy, srcu, srcv, w);

        dst += dstpitch;
        srcy += srcpitch;
        srcu += srcpitch/2;
        srcv += srcpitch/2;
    }
    while((--h) > 0);

    if(CpuFeatureFlags & FEATURE_MMX)
        EndMMX();

    return(true);
}

bool BitBltFromI444ToYUY2(int w, int h, BYTE* dst, int dstpitch, BYTE* srcy, BYTE* srcu, BYTE* srcv, int srcpitch)
{
    if(w<=0 || h<=0 || (w&1) || (h&1))
        return(false);

    if(srcpitch == 0) srcpitch = w;

    do
    {
        yuv444toyuy2row_c(dst, srcy, srcu, srcv, w);

        dst += dstpitch;
        srcy += srcpitch;
        srcu += srcpitch;
        srcv += srcpitch;
    }
    while((--h) > 0);

    return(true);
}


void memcpy_accel(void* dst, const void* src, size_t len)
{
    if((CpuFeatureFlags & FEATURE_SSE) && len >= 128
        && !((DWORD)src&15) && !((DWORD)dst&15))
    {
        memcpy_accel_SSE(dst, src, len);
    }
    else if((CpuFeatureFlags & FEATURE_MMX) && len >= 64
        && !((DWORD)src&7) && !((DWORD)dst&7))
    {
        memcpy_accel_MMX(dst, src, len);
    }
    else
    {
        memcpy(dst, src, len);
    }
}

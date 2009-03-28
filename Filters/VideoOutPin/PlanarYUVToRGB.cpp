///////////////////////////////////////////////////////////////////////////////
// $Id$
///////////////////////////////////////////////////////////////////////////////
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


#include "stdafx.h"
#include "PlanarYUVToRGB.h"
#include "CPUID.h"

#pragma warning(disable : 4799) // no emms... blahblahblah


extern "C" void asm_YUVtoRGB32_row(void* ARGB1, void* ARGB2, BYTE* Y1, BYTE* Y2, BYTE* U, BYTE* V, long width);
extern "C" void asm_YUVtoRGB24_row(void* ARGB1, void* ARGB2, BYTE* Y1, BYTE* Y2, BYTE* U, BYTE* V, long width);
extern "C" void asm_YUVtoRGB16_row(void* ARGB1, void* ARGB2, BYTE* Y1, BYTE* Y2, BYTE* U, BYTE* V, long width);
extern "C" void asm_YUVtoRGB32_row_MMX(void* ARGB1, void* ARGB2, BYTE* Y1, BYTE* Y2, BYTE* U, BYTE* V, long width);
extern "C" void asm_YUVtoRGB24_row_MMX(void* ARGB1, void* ARGB2, BYTE* Y1, BYTE* Y2, BYTE* U, BYTE* V, long width);
extern "C" void asm_YUVtoRGB16_row_MMX(void* ARGB1, void* ARGB2, BYTE* Y1, BYTE* Y2, BYTE* U, BYTE* V, long width);
extern "C" void asm_YUVtoRGB32_row_ISSE(void* ARGB1, void* ARGB2, BYTE* Y1, BYTE* Y2, BYTE* U, BYTE* V, long width);
extern "C" void asm_YUVtoRGB24_row_ISSE(void* ARGB1, void* ARGB2, BYTE* Y1, BYTE* Y2, BYTE* U, BYTE* V, long width);
extern "C" void asm_YUVtoRGB16_row_ISSE(void* ARGB1, void* ARGB2, BYTE* Y1, BYTE* Y2, BYTE* U, BYTE* V, long width);

bool BitBltFromI420ToRGB(int w, int h, BYTE* dst, int dstpitch, int dbpp, BYTE* srcy, BYTE* srcu, BYTE* srcv, int srcpitch)
{
    if(w<=0 || h<=0 || (w&1) || (h&1))
        return(false);

    void (*asm_YUVtoRGB_row)(void* ARGB1, void* ARGB2, BYTE* Y1, BYTE* Y2, BYTE* U, BYTE* V, long width) = NULL;;

    if((CpuFeatureFlags & FEATURE_SSE) && !(w&7))
    {
        switch(dbpp)
        {
        case 16: asm_YUVtoRGB_row = asm_YUVtoRGB16_row/*_ISSE*/; break; // TODO: fix _ISSE (555->565)
        case 24: asm_YUVtoRGB_row = asm_YUVtoRGB24_row; break;
        case 32: asm_YUVtoRGB_row = asm_YUVtoRGB32_row_ISSE; break;
        }
    }
    else if((CpuFeatureFlags & FEATURE_MMX) && !(w&7))
    {
        switch(dbpp)
        {
        case 16: asm_YUVtoRGB_row = asm_YUVtoRGB16_row/*_MMX*/; break; // TODO: fix _MMX (555->565)
        case 24: asm_YUVtoRGB_row = asm_YUVtoRGB24_row; break;
        case 32: asm_YUVtoRGB_row = asm_YUVtoRGB32_row_MMX; break;
        }
    }
    else
    {
        switch(dbpp)
        {
        case 16: asm_YUVtoRGB_row = asm_YUVtoRGB16_row; break;
        case 24: asm_YUVtoRGB_row = asm_YUVtoRGB24_row; break;
        case 32: asm_YUVtoRGB_row = asm_YUVtoRGB32_row; break;
        }
    }

    if(!asm_YUVtoRGB_row)
        return(false);

    do
    {
        asm_YUVtoRGB_row(dst + dstpitch, dst, srcy + srcpitch, srcy, srcu, srcv, w/2);

        dst += 2*dstpitch;
        srcy += srcpitch*2;
        srcu += srcpitch/2;
        srcv += srcpitch/2;
    }
    while(h -= 2);

    if(CpuFeatureFlags & FEATURE_SSE)
        EndSSE();
    else if(CpuFeatureFlags & FEATURE_MMX)
        EndMMX();


    return true;
}

bool BitBltFromI422ToRGB(int w, int h, BYTE* dst, int dstpitch, int dbpp, BYTE* srcy, BYTE* srcu, BYTE* srcv, int srcpitch)
{
    if(w<=0 || h<=0 || (w&1) || (h&1))
        return(false);

    void (*asm_YUVtoRGB_row)(void* ARGB1, void* ARGB2, BYTE* Y1, BYTE* Y2, BYTE* U, BYTE* V, long width) = NULL;;

    if((CpuFeatureFlags & FEATURE_SSE) && !(w&7))
    {
        switch(dbpp)
        {
        case 16: asm_YUVtoRGB_row = asm_YUVtoRGB16_row/*_ISSE*/; break; // TODO: fix _ISSE (555->565)
        case 24: asm_YUVtoRGB_row = asm_YUVtoRGB24_row_ISSE; break;
        case 32: asm_YUVtoRGB_row = asm_YUVtoRGB32_row_ISSE; break;
        }
    }
    else if((CpuFeatureFlags & FEATURE_MMX) && !(w&7))
    {
        switch(dbpp)
        {
        case 16: asm_YUVtoRGB_row = asm_YUVtoRGB16_row/*_MMX*/; break; // TODO: fix _MMX (555->565)
        case 24: asm_YUVtoRGB_row = asm_YUVtoRGB24_row_MMX; break;
        case 32: asm_YUVtoRGB_row = asm_YUVtoRGB32_row_MMX; break;
        }
    }
    else
    {
        switch(dbpp)
        {
        case 16: asm_YUVtoRGB_row = asm_YUVtoRGB16_row; break;
        case 24: asm_YUVtoRGB_row = asm_YUVtoRGB24_row; break;
        case 32: asm_YUVtoRGB_row = asm_YUVtoRGB32_row; break;
        }
    }

    if(!asm_YUVtoRGB_row)
        return(false);

    do
    {
        asm_YUVtoRGB_row(dst + dstpitch, dst, srcy + srcpitch, srcy, srcu, srcv, w/2);

        dst += 2*dstpitch;
        srcy += srcpitch*2;
        srcu += srcpitch/2;
        srcv += srcpitch/2;
    }
    while(h -= 2);

    if(CpuFeatureFlags & FEATURE_SSE)
        EndSSE();
    else if(CpuFeatureFlags & FEATURE_MMX)
        EndMMX();

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// $Id: PlanarYUVToYUY2.cpp,v 1.1 2004-02-06 12:17:16 adcockj Exp $
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2004 John Adcock
//
//  This file based on code taken from vd.cpp in the guliverkli project
//  which in turn took it from
//	VirtualDub - Video processing and capture application
//	Copyright (C) 1998-2001 Avery Lee
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
//
// CVS Log
//
// $Log: not supported by cvs2svn $
//  Notes: 
///////////////////////////////////////////////////////////////////////////////
//  - BitBltFromI420ToRGB is from VirtualDub
//	- The core assembly function of CCpuID is from DVD2AVI
//	(- vd.cpp/h should be renamed to something more sensible already :)
///////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "PlanarYUVToYUY2.h"
#include "CPUID.h"

#pragma warning(disable : 4799) // no emms... blahblahblah

void memcpy_accel(void* dst, const void* src, size_t len);

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
		__asm emms

	return(true);
}

bool BitBltFromI422ToI422(int w, int h, BYTE* dsty, BYTE* dstu, BYTE* dstv, int dstpitch, BYTE* srcy, BYTE* srcu, BYTE* srcv, int srcpitch)
{
	if(w&1) return(false);

	int pitch = min(abs(srcpitch), abs(dstpitch));
    
    int y;
	for(y = 0; y < h; y++, srcy += srcpitch, dsty += dstpitch)
		memcpy_accel(dsty, srcy, pitch);

	srcpitch >>= 1;
	dstpitch >>= 1;

	pitch = min(abs(srcpitch), abs(dstpitch));

	for(y = 0; y < h; y+=1, srcu += srcpitch, dstu += dstpitch)
		memcpy_accel(dstu, srcu, pitch);

	for(y = 0; y < h; y+=1, srcv += srcpitch, dstv += dstpitch)
		memcpy_accel(dstv, srcv, pitch);

	if(CpuFeatureFlags & FEATURE_MMX)
		__asm emms

	return(true);
}

// \todo optimise and filter
// we should probably do much better than this simple decimation
// but this is really just here for completeness
bool BitBltFromI444ToI422(int w, int h, BYTE* dsty, BYTE* dstu, BYTE* dstv, int dstpitch, BYTE* srcy, BYTE* srcu, BYTE* srcv, int srcpitch)
{
	if(w&1) return(false);

	int pitch = min(abs(srcpitch), abs(dstpitch));
    
    int y;
    int z;
	for(y = 0; y < h; y++, srcy += srcpitch, dsty += dstpitch)
		memcpy_accel(dsty, srcy, pitch);

	dstpitch >>= 1;

	pitch = min(abs(srcpitch), abs(dstpitch));

	for(y = 0; y < h; y++, srcu += srcpitch, dstu += dstpitch)
		for(z = 0; z < pitch; z++)
			dstu[z] = srcu[z<<1];

	for(y = 0; y < h; y++, srcv += srcpitch, dstv += dstpitch)
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
		__asm emms

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

static void __declspec(naked) yuvtoyuy2row_MMX(BYTE* dst, BYTE* srcy, BYTE* srcu, BYTE* srcv, DWORD width)
{
	__asm {
		push	ebp
		push	edi
		push	esi
		push	ebx

		mov		edi, [esp+20] // dst
		mov		ebp, [esp+24] // srcy
		mov		ebx, [esp+28] // srcu
		mov		esi, [esp+32] // srcv
		mov		ecx, [esp+36] // width

		shr		ecx, 3

yuvtoyuy2row_loop:

		movd		mm0, [ebx]
		punpcklbw	mm0, [esi]

		movq		mm1, [ebp]
		movq		mm2, mm1
		punpcklbw	mm1, mm0
		punpckhbw	mm2, mm0

		movq		[edi], mm1
		movq		[edi+8], mm2

		add		ebp, 8
		add		ebx, 4
		add		esi, 4
        add		edi, 16

		loop	yuvtoyuy2row_loop

		pop		ebx
		pop		esi
		pop		edi
		pop		ebp
		ret
	};
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

static void __declspec(naked) yuvtoyuy2row_avg_MMX(BYTE* dst, BYTE* srcy, BYTE* srcu, BYTE* srcv, DWORD width, DWORD pitchuv)
{
	static const __int64 mask = 0x7f7f7f7f7f7f7f7fi64;

	__asm {
		push	ebp
		push	edi
		push	esi
		push	ebx

		movq	mm7, mask

		mov		edi, [esp+20] // dst
		mov		ebp, [esp+24] // srcy
		mov		ebx, [esp+28] // srcu
		mov		esi, [esp+32] // srcv
		mov		ecx, [esp+36] // width
		mov		eax, [esp+40] // pitchuv

		shr		ecx, 3

yuvtoyuy2row_avg_loop:

		movd		mm0, [ebx]
		punpcklbw	mm0, [esi]
		movq		mm1, mm0

		movd		mm2, [ebx + eax]
		punpcklbw	mm2, [esi + eax]
		movq		mm3, mm2

		// (x+y)>>1 == (x&y)+((x^y)>>1)

		pand		mm0, mm2
		pxor		mm1, mm3
		psrlq		mm1, 1
		pand		mm1, mm7
		paddb		mm0, mm1

		movq		mm1, [ebp]
		movq		mm2, mm1
		punpcklbw	mm1, mm0
		punpckhbw	mm2, mm0

		movq		[edi], mm1
		movq		[edi+8], mm2

		add		ebp, 8
		add		ebx, 4
		add		esi, 4
        add		edi, 16

		loop	yuvtoyuy2row_avg_loop

		pop		ebx
		pop		esi
		pop		edi
		pop		ebp
		ret
	};
}

static void __declspec(naked) yuvtoyuy2row_avg_SSEMMX(BYTE* dst, BYTE* srcy, BYTE* srcu, BYTE* srcv, DWORD width, DWORD pitchuv)
{
	static const __int64 mask = 0x7f7f7f7f7f7f7f7fi64;

	__asm {
		push	ebp
		push	edi
		push	esi
		push	ebx

		movq	mm7, mask

		mov		edi, [esp+20] // dst
		mov		ebp, [esp+24] // srcy
		mov		ebx, [esp+28] // srcu
		mov		esi, [esp+32] // srcv
		mov		ecx, [esp+36] // width
		mov		eax, [esp+40] // pitchuv

		shr		ecx, 3

yuvtoyuy2row_avg_loop:

		movd		mm0, [ebx]
		punpcklbw	mm0, [esi]

		movd		mm2, [ebx + eax]
		punpcklbw	mm2, [esi + eax]

		pavgb mm0, mm2

		movq		mm1, [ebp]
		movq		mm2, mm1
		punpcklbw	mm1, mm0
		punpckhbw	mm2, mm0

		movq		[edi], mm1
		movq		[edi+8], mm2

		add		ebp, 8
		add		ebx, 4
		add		esi, 4
        add		edi, 16

		loop	yuvtoyuy2row_avg_loop

		pop		ebx
		pop		esi
		pop		edi
		pop		ebp
		ret
	};
}


static void __declspec(naked) yuvtoyuy2row_avg2_MMX(BYTE* dst, BYTE* srcy, BYTE* srcu, BYTE* srcv, DWORD width, DWORD pitchuv)
{
	static const __int64 mask = 0x7f7f7f7f7f7f7f7fi64;

	__asm {
		push	ebp
		push	edi
		push	esi
		push	ebx

		movq	mm7, mask

		mov		edi, [esp+20] // dst
		mov		ebp, [esp+24] // srcy
		mov		ebx, [esp+28] // srcu
		mov		esi, [esp+32] // srcv
		mov		ecx, [esp+36] // width
		mov		eax, [esp+40] // pitchuv

		shr		ecx, 3

yuvtoyuy2row_avg2_loop:

		movd		mm0, [ebx]
		punpcklbw	mm0, [esi]
		movq		mm1, mm0
		movq		mm4, mm0

		movd		mm2, [ebx + eax]
		punpcklbw	mm2, [esi + eax]
		movq        mm3, mm2

		// average first with second and then with first again
		pand		mm0, mm2
		pxor		mm1, mm3
		psrlq		mm1, 1
		pand		mm1, mm7
		paddb		mm0, mm1
		
		movq        mm5, mm4
		movq        mm1, mm0

		pand		mm0, mm4
		pxor		mm1, mm5
		psrlq		mm1, 1
		pand		mm1, mm7
		paddb		mm0, mm1

		movq		mm1, [ebp]
		movq		mm2, mm1
		punpcklbw	mm1, mm0
		punpckhbw	mm2, mm0

		movq		[edi], mm1
		movq		[edi+8], mm2

		add		ebp, 8
		add		ebx, 4
		add		esi, 4
        add		edi, 16

		loop	yuvtoyuy2row_avg2_loop

		pop		ebx
		pop		esi
		pop		edi
		pop		ebp
		ret
	};
}


static void __declspec(naked) yuvtoyuy2row_avg2_SSEMMX(BYTE* dst, BYTE* srcy, BYTE* srcu, BYTE* srcv, DWORD width, DWORD pitchuv)
{
	static const __int64 mask = 0x7f7f7f7f7f7f7f7fi64;

	__asm {
		push	ebp
		push	edi
		push	esi
		push	ebx

		movq	mm7, mask

		mov		edi, [esp+20] // dst
		mov		ebp, [esp+24] // srcy
		mov		ebx, [esp+28] // srcu
		mov		esi, [esp+32] // srcv
		mov		ecx, [esp+36] // width
		mov		eax, [esp+40] // pitchuv

		shr		ecx, 3

yuvtoyuy2row_avg2_loop_SSEMMX:

		movd		mm0, [ebx]
		punpcklbw	mm0, [esi]
		movq		mm1, mm0

		movd		mm2, [ebx + eax]
		punpcklbw	mm2, [esi + eax]

		pavgb       mm0, mm2
		pavgb       mm0, mm1

		movq		mm1, [ebp]
		movq		mm2, mm1
		punpcklbw	mm1, mm0
		punpckhbw	mm2, mm0

		movq		[edi], mm1
		movq		[edi+8], mm2

		add		ebp, 8
		add		ebx, 4
		add		esi, 4
        add		edi, 16

		loop	yuvtoyuy2row_avg2_loop_SSEMMX

		pop		ebx
		pop		esi
		pop		edi
		pop		ebp
		ret
	};
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
		__asm emms

	return(true);
}

bool BitBltFromI420ToYUY2_Int(int w, int h, BYTE* dst, int dstpitch, BYTE* srcy, BYTE* srcu, BYTE* srcv, int srcpitch)
{
	if(w<=0 || h<=0 || (w&1) || (h&1))
		return(false);

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

    // \todo: Fix me .....
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
		__asm emms

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
		__asm emms

	return(true);
}

void memcpy_accel(void* dst, const void* src, size_t len)
{
	if((CpuFeatureFlags & FEATURE_SSE) && len >= 128 
		&& !((DWORD)src&15) && !((DWORD)dst&15))
	{
		__asm
		{
			mov     esi, dword ptr [src]
			mov     edi, dword ptr [dst]
			mov     ecx, len
			shr     ecx, 7
	memcpy_accel_sse_loop:
			prefetchnta	[esi]
			movaps		xmm0, [esi]
			movaps		xmm1, [esi+16*1]
			movaps		xmm2, [esi+16*2]
			movaps		xmm3, [esi+16*3]
			movaps		xmm4, [esi+16*4]
			movaps		xmm5, [esi+16*5]
			movaps		xmm6, [esi+16*6]
			movaps		xmm7, [esi+16*7]
			movntps		[edi], xmm0
			movntps		[edi+16*1], xmm1
			movntps		[edi+16*2], xmm2
			movntps		[edi+16*3], xmm3
			movntps		[edi+16*4], xmm4
			movntps		[edi+16*5], xmm5
			movntps		[edi+16*6], xmm6
			movntps		[edi+16*7], xmm7
			add			esi, 128
			add			edi, 128
			loop	memcpy_accel_sse_loop
			mov     ecx, len
			and     ecx, 127
			cmp     ecx, 0
			je		memcpy_accel_sse_end
	memcpy_accel_sse_loop2:
			mov		dl, byte ptr[esi] 
			mov		byte ptr[edi], dl
			inc		esi
			inc		edi
			dec		ecx
			jne		memcpy_accel_sse_loop2
	memcpy_accel_sse_end:
			sfence
		}
	}
	else if((CpuFeatureFlags & FEATURE_MMX) && len >= 64
		&& !((DWORD)src&7) && !((DWORD)dst&7))
	{
		__asm 
		{
			mov     esi, dword ptr [src]
			mov     edi, dword ptr [dst]
			mov     ecx, len
			shr     ecx, 6
	memcpy_accel_mmx_loop:
			movq    mm0, qword ptr [esi]
			movq    mm1, qword ptr [esi+8*1]
			movq    mm2, qword ptr [esi+8*2]
			movq    mm3, qword ptr [esi+8*3]
			movq    mm4, qword ptr [esi+8*4]
			movq    mm5, qword ptr [esi+8*5]
			movq    mm6, qword ptr [esi+8*6]
			movq    mm7, qword ptr [esi+8*7]
			movq    qword ptr [edi], mm0
			movq    qword ptr [edi+8*1], mm1
			movq    qword ptr [edi+8*2], mm2
			movq    qword ptr [edi+8*3], mm3
			movq    qword ptr [edi+8*4], mm4
			movq    qword ptr [edi+8*5], mm5
			movq    qword ptr [edi+8*6], mm6
			movq    qword ptr [edi+8*7], mm7
			add     esi, 64
			add     edi, 64
			loop	memcpy_accel_mmx_loop
			mov     ecx, len
			and     ecx, 63
			cmp     ecx, 0
			je		memcpy_accel_mmx_end
	memcpy_accel_mmx_loop2:
			mov		dl, byte ptr [esi] 
			mov		byte ptr [edi], dl
			inc		esi
			inc		edi
			dec		ecx
			jne		memcpy_accel_mmx_loop2
	memcpy_accel_mmx_end:
		}
	}
	else
	{
		memcpy(dst, src, len);
	}
}

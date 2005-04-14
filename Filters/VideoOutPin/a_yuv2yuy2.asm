;///////////////////////////////////////////////////////////////////////////////
;// $Id: a_yuv2yuy2.asm,v 1.1 2005-04-14 11:21:07 adcockj Exp $
;///////////////////////////////////////////////////////////////////////////////
;//	This program is free software; you can redistribute it and/or modify
;//	it under the terms of the GNU General Public License as published by
;//	the Free Software Foundation; either version 2 of the License, or
;//	(at your option) any later version.
;//
;//	This program is distributed in the hope that it will be useful,
;//	but WITHOUT ANY WARRANTY; without even the implied warranty of
;//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;//	GNU General Public License for more details.
;//
;//	You should have received a copy of the GNU General Public License
;//	along with this program; if not, write to the Free Software
;//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
;///////////////////////////////////////////////////////////////////////////////
;//
;//  This file based on code taken from vd.cpp in the guliverkli project
;//  which in turn took it from
;//	  VirtualDub - Video processing and capture application
;//	  Copyright (C) 1998-2001 Avery Lee
;//
;///////////////////////////////////////////////////////////////////////////////
;// 08/03/2004 John Adcock - Converted to nasm format
;//
;///////////////////////////////////////////////////////////////////////////////
;// CVS Log
;//
;// $Log: not supported by cvs2svn $
;// Revision 1.1  2004/10/28 15:52:24  adcockj
;// Moved video output pin code into new class
;//
;// Revision 1.2  2004/08/06 08:38:54  adcockj
;// Added optional YV12 output type
;//
;// Revision 1.1  2004/03/08 17:04:02  adcockj
;// Removed all inline assembler to remove dependence on MS compilers
;//
;//////////////////////////////////////////////////////////////////////////////

USE32

segment .data

segment .const

    align	16

    MMX_mask	dd	0x7f7f7f7f, 0x7f7f7f7f

	.code


proc _yuvtoyuy2row_MMX, 12

    %$dst arg
    %$srcy arg
    %$srcu arg
    %$srcv arg
    %$width arg

	push	edi
	push	esi
	push	ebx

	mov		edi, [ebp+%$dst]
	mov		eax, [ebp+%$srcy]
	mov		ebx, [ebp+%$srcu]
	mov		esi, [ebp+%$srcv]
	mov		ecx, [ebp+%$width]

	shr		ecx, 3

yuvtoyuy2row_loop:

	movd		mm0, [ebx]
	punpcklbw	mm0, [esi]

	movq		mm1, [eax]
	movq		mm2, mm1
	punpcklbw	mm1, mm0
	punpckhbw	mm2, mm0

	movq		[edi], mm1
	movq		[edi+8], mm2

	add		eax, 8
	add		ebx, 4
	add		esi, 4
    add		edi, 16

	loop	yuvtoyuy2row_loop

	pop		ebx
	pop		esi
	pop		edi
endproc


proc _yuvtoyuy2row_avg_MMX, 0

    %$dst arg
    %$srcy arg
    %$srcu arg
    %$srcv arg
    %$width arg
    %$pitchuv arg

	push	edi
	push	esi
	push	ebx

	movq	mm7, [MMX_mask]

	mov		edi, [ebp+%$dst]
	mov		edx, [ebp+%$srcy]
	mov		ebx, [ebp+%$srcu]
	mov		esi, [ebp+%$srcv]
	mov		ecx, [ebp+%$width]
	mov		eax, [ebp+%$pitchuv]

	shr		ecx, 3

yuvtoyuy2row_avg_mmx_loop:

	movd		mm0, [ebx]
	punpcklbw	mm0, [esi]
	movq		mm1, mm0

	movd		mm2, [ebx + eax]
	punpcklbw	mm2, [esi + eax]
	movq		mm3, mm2

	; (x+y)>>1 == (x&y)+((x^y)>>1)

	pand		mm0, mm2
	pxor		mm1, mm3
	psrlq		mm1, 1
	pand		mm1, mm7
	paddb		mm0, mm1

	movq		mm1, [edx]
	movq		mm2, mm1
	punpcklbw	mm1, mm0
	punpckhbw	mm2, mm0

	movq		[edi], mm1
	movq		[edi+8], mm2

	add		edx, 8
	add		ebx, 4
	add		esi, 4
    add		edi, 16

	loop	yuvtoyuy2row_avg_mmx_loop

	pop		ebx
	pop		esi
	pop		edi

endproc

proc _yuvtoyuy2row_avg_SSEMMX, 0

    %$dst arg
    %$srcy arg
    %$srcu arg
    %$srcv arg
    %$width arg
    %$pitchuv arg

	push	edi
	push	esi
	push	ebx

	movq	mm7, [MMX_mask]

	mov		edi, [ebp+%$dst]
	mov		edx, [ebp+%$srcy]
	mov		ebx, [ebp+%$srcu]
	mov		esi, [ebp+%$srcv]
	mov		ecx, [ebp+%$width]
	mov		eax, [ebp+%$pitchuv]

	shr		ecx, 3

yuvtoyuy2row_avg_sse_loop:

	movd		mm0, [ebx]
	punpcklbw	mm0, [esi]

	movd		mm2, [ebx + eax]
	punpcklbw	mm2, [esi + eax]

	pavgb mm0, mm2

	movq		mm1, [edx]
	movq		mm2, mm1
	punpcklbw	mm1, mm0
	punpckhbw	mm2, mm0

	movq		[edi], mm1
	movq		[edi+8], mm2

	add		edx, 8
	add		ebx, 4
	add		esi, 4
    add		edi, 16

	loop	yuvtoyuy2row_avg_sse_loop

	pop		ebx
	pop		esi
	pop		edi

endproc

proc _yuvtoyuy2row_avg2_MMX, 0

    %$dst arg
    %$srcy arg
    %$srcu arg
    %$srcv arg
    %$width arg
    %$pitchuv arg

	push	edi
	push	esi
	push	ebx

	movq	mm7, [MMX_mask]

	mov		edi, [ebp+%$dst]
	mov		edx, [ebp+%$srcy]
	mov		ebx, [ebp+%$srcu]
	mov		esi, [ebp+%$srcv]
	mov		ecx, [ebp+%$width]
	mov		eax, [ebp+%$pitchuv]

	shr		ecx, 3

yuvtoyuy2row_avg2_mmx_loop:

	movd		mm0, [ebx]
	punpcklbw	mm0, [esi]
	movq		mm1, mm0
	movq		mm4, mm0

	movd		mm2, [ebx + eax]
	punpcklbw	mm2, [esi + eax]
	movq        mm3, mm2

	; average first with second and then with first again
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

	movq		mm1, [edx]
	movq		mm2, mm1
	punpcklbw	mm1, mm0
	punpckhbw	mm2, mm0

	movq		[edi], mm1
	movq		[edi+8], mm2

	add		edx, 8
	add		ebx, 4
	add		esi, 4
    add		edi, 16

	loop	yuvtoyuy2row_avg2_mmx_loop

	pop		ebx
	pop		esi
	pop		edi
endproc

proc _yuvtoyuy2row_avg2_SSEMMX, 0

    %$dst arg
    %$srcy arg
    %$srcu arg
    %$srcv arg
    %$width arg
    %$pitchuv arg

	push	edi
	push	esi
	push	ebx

	movq	mm7, [MMX_mask]

	mov		edi, [ebp+%$dst]
	mov		edx, [ebp+%$srcy]
	mov		ebx, [ebp+%$srcu]
	mov		esi, [ebp+%$srcv]
	mov		ecx, [ebp+%$width]
	mov		eax, [ebp+%$pitchuv]

	shr		ecx, 3

yuvtoyuy2row_avg2_loop_SSEMMX:

	movd		mm0, [ebx]
	punpcklbw	mm0, [esi]
	movq		mm1, mm0

	movd		mm2, [ebx + eax]
	punpcklbw	mm2, [esi + eax]

	pavgb       mm0, mm2
	pavgb       mm0, mm1

	movq		mm1, [edx]
	movq		mm2, mm1
	punpcklbw	mm1, mm0
	punpckhbw	mm2, mm0

	movq		[edi], mm1
	movq		[edi+8], mm2

	add		edx, 8
	add		ebx, 4
	add		esi, 4
    add		edi, 16

	loop	yuvtoyuy2row_avg2_loop_SSEMMX

	pop		ebx
	pop		esi
	pop		edi
endproc

proc _memcpy_accel_SSE, 0
    %$dst arg
    %$src arg
    %$len arg

	push	edi
	push	esi
	push	ebx

	mov     esi, [ebp+%$src]
	mov     edi, [ebp+%$dst]
	mov     ecx, [ebp+%$len]
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
	mov     ecx, [ebp+%$len]
	and     ecx, 127
	cmp     ecx, 0
	je		memcpy_accel_sse_end
memcpy_accel_sse_loop2:
	mov		dl,[esi] 
	mov		[edi], dl
	inc		esi
	inc		edi
	dec		ecx
	jne		memcpy_accel_sse_loop2
memcpy_accel_sse_end:
	sfence

	pop		ebx
	pop		esi
	pop		edi
endproc

proc _memcpy_accel_MMX, 0
    %$dst arg
    %$src arg
    %$len arg

	push	edi
	push	esi
	push	ebx

	mov     esi, [ebp+%$src]
	mov     edi, [ebp+%$dst]
	mov     ecx, [ebp+%$len]
	shr     ecx, 6
memcpy_accel_mmx_loop:
	movq    mm0, [esi]
	movq    mm1, [esi+8*1]
	movq    mm2, [esi+8*2]
	movq    mm3, [esi+8*3]
	movq    mm4, [esi+8*4]
	movq    mm5, [esi+8*5]
	movq    mm6, [esi+8*6]
	movq    mm7, [esi+8*7]
	movq    [edi], mm0
	movq    [edi+8*1], mm1
	movq    [edi+8*2], mm2
	movq    [edi+8*3], mm3
	movq    [edi+8*4], mm4
	movq    [edi+8*5], mm5
	movq    [edi+8*6], mm6
	movq    [edi+8*7], mm7
	add     esi, 64
	add     edi, 64
	loop	memcpy_accel_mmx_loop
	mov     ecx, [ebp+%$len]
	and     ecx, 63
	cmp     ecx, 0
	je		memcpy_accel_mmx_end
memcpy_accel_mmx_loop2:
	mov		dl, [esi] 
	mov		[edi], dl
	inc		esi
	inc		edi
	dec		ecx
	jne		memcpy_accel_mmx_loop2
memcpy_accel_mmx_end:

	pop		ebx
	pop		esi
	pop		edi
endproc

end

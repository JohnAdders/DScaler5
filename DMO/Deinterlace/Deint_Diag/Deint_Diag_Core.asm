;///////////////////////////////////////////////////////////////////////////////
;// $Id: Deint_Diag_Core.asm,v 1.7 2004-12-07 16:53:32 adcockj Exp $
;///////////////////////////////////////////////////////////////////////////////
;// CVS Log
;//
;// $Log: not supported by cvs2svn $
;// Revision 1.6  2004/12/06 18:04:56  adcockj
;// Major improvements to deinterlacing
;//
;// Revision 1.5  2004/08/31 16:33:40  adcockj
;// Minor improvements to quality control
;// Preparation for next version
;// Start on integrating film detect
;//
;// Revision 1.4  2003/10/31 17:19:37  adcockj
;// Added support for manual pulldown selection (works with Elecard Filters)
;//
;// Revision 1.3  2003/09/24 16:33:00  adcockj
;// Bug fixes - starting to work now..
;//
;// Revision 1.2  2003/09/24 07:01:02  adcockj
;// fix some release issues
;//
;// Revision 1.1  2003/07/18 09:26:34  adcockj
;// Corrections to assembler files (does not compile)
;//
;//////////////////////////////////////////////////////////////////////////////

USE32

segment .data

ShiftMask dd 0xfefffeff, 0xfefffeff
YMask dd 0x00ff00ff, 0x00ff00ff
UVMask dd 0xff00ff00, 0xff00ff00

global _MOVE
_MOVE dd 0x0f0f0f0f, 0x0f0f0f0f

global _STILL
_STILL dd 0x01010101, 0x01010101

segment .text

;---------------------------------------------------------------------------
; Do diagonal Interpolating YUY2 method
;---------------------------------------------------------------------------
%imacro Deint_Diag_Core_YUY2 1

global _Deint_Diag_Core_YUY2%1

proc _Deint_Diag_Core_YUY2%1, 28

    %define %1 1

    %$M1 arg
    %$T2 arg
    %$B2 arg
    %$M3 arg
    %$T4 arg
    %$B4 arg
    %$Dest arg
    %$PixelCount arg

	mov [esp+16], esi
	mov [esp+20], edi
	mov [esp+24], ebx

	mov ebx, [ebp + %$T2]
	mov edx, [ebp + %$B2]

    ; PixelCount -= 8
    mov eax, [ebp + %$PixelCount]
    sub eax, 8
    mov [ebp + %$PixelCount], eax

    ; now loop over the stuff in the middle
	mov		esi, [ebp + %$M1]
	mov		edi, [ebp + %$M3]

	; no diagonal on first 8 bytes
	movq    mm2, [ebx]
	movq    mm5, [edx]
    movq    mm1, [esi]
    DS_PAVGB mm5, mm2, mm0, [ShiftMask]
	
    mov     eax, [ebp + %$T4]
    movq	mm2, [eax]

    movq    mm3, [edi]

    mov     eax, [ebp + %$B4]
    movq	mm4, [eax]
    
    pcmpgtb mm2, [_STILL]
    pcmpgtb mm3, [_STILL]
    pcmpgtb mm4, [_STILL]

    por  mm2, mm4                ; top or bottom still
    pand  mm3, mm2               ; where we should weave

	mov		eax, [ebp + %$Dest]
    DS_COMBINE mm1, mm5, mm3
    DS_MOVNTQ [eax], mm1

	mov		ecx, 8				; curr offset into all lines

LoopYUY2%1:	
	add		esi, 8		
	add		edi, 8		
	add		ebx, 8
	add		edx, 8

	; First, get and save our possible Bob values
	; Assume our pixels are layed out as follows with x the calc'd bob value
	; and the other pixels are from the current field
	;  
	;         a b c 		current field
	;           x			calculated line
	;         d e f 		current field
	;

	movq    mm1, [ebx-2]
	movq    mm2, [ebx]
	movq    mm3, [ebx + 2]

	movq    mm4, [edx - 2]
	movq    mm5, [edx]
	movq    mm6, [edx + 2]

    ; mm1 = a
    ; mm2 = b
    ; mm3 = c
    ; mm4 = d
    ; mm5 = e
    ; mm6 = f

    movq mm7, mm1
    DS_PAVGB mm7, mm6, mm0, [ShiftMask]
    movq [esp], mm7

    movq mm7, mm3
    DS_PAVGB mm7, mm4, mm0, [ShiftMask]
    movq [esp + 8], mm7

    movq mm7, mm1
    DS_PABS mm1, mm6, mm0
    DS_PABS mm6, mm2, mm0
    DS_PABS mm7, mm5, mm0
    DS_PAVGB mm6, mm7, mm0, [ShiftMask]

    movq mm7, mm3
    DS_PABS mm3, mm4, mm0
    DS_PABS mm4, mm2, mm0
    DS_PABS mm7, mm5, mm0
    DS_PAVGB mm4, mm7, mm0, [ShiftMask]

    movq mm7, mm2
    DS_PABS mm2, mm5, mm0
    DS_PAVGB mm5, mm7, mm0, [ShiftMask]

    ; at this point the  following registers are set
    ; mm1 = |a - f|
    ; mm2 = |b - e|
    ; mm3 = |c - d|
    ; mm4 = avg(|b - d|, |c - e|)
    ; mm5 = avg(b, e)
    ; mm6 = avg(|a - e|, |b - f|)
    ; [esp] = avg(a, f)
    ; [esp + 8] = avg(c, d)
    
	pxor	mm0, mm0			
	psubusb mm4, mm6			; nonzero where mm4 > mm6 
	pcmpeqb mm4, mm0			; now ff where mm4 <= mm6
    movq    mm6, mm4
    pcmpeqb mm6, mm0            ; now ff where mm6 < mm4

    ; make the corresponding one unsuitable
    paddusb mm1, mm4
    paddusb mm3, mm6

    movq mm4, mm1
    movq mm6, mm3
    psubusb mm1, mm3            ; nonzero  where (|a - f| >= |c - d|)
	pcmpeqb mm1, mm0			; now ff where (|a - f| < |c - d|)
    movq mm7, mm1
    DS_COMBINE mm4, mm6, mm1
    movq mm6, [esp]
    movq mm3, [esp + 8]
    DS_COMBINE mm6, mm3, mm7

    ; so now we should have mm4 as current best diff and mm6 as best average
    ; of the diagonals`
    movq    mm1, mm2           ; save |b - e|
    psubusb mm2, mm4           ; nonzero  where (|b - e| >= Best)
	pcmpeqb mm2, mm0		   ; now ff where (|b - e| < Best)
    por mm2, [UVMask]          ; we only want luma from diagonals
    psubusb mm1, [_MOVE]        ; nonzero where |b - e| > MOVE is Bob is visible
	pcmpeqb mm1, mm0		   ; now ff where (|b - e| <= MOVE)
    por mm2, mm1               ; we'll let bob through always if the diff is small
 
    DS_COMBINE mm5, mm6, mm2
    movq mm6, mm5


    ;//////////////////////////////////////////////////////////////////////////
    ; simple weave
    ;//////////////////////////////////////////////////////////////////////////

    ; at the start of this function
    ; mm0 = 0
    ; mm6 = Bob pixels          

    movq mm1, [esi]

   ;//////////////////////////////////////////////////////////////////////////
    ; Bottom
    ;//////////////////////////////////////////////////////////////////////////
    ; these should be the values held as we go in
    ; mm0 = 0
    ; mm1 = weave pixels
    ; mm6 = Bob pixels          

    mov     eax, [ebp + %$T4]
    movq	mm2, [eax + ecx]

    movq mm3, [edi]

    mov     eax, [ebp + %$B4]
    movq	mm4, [eax + ecx]
    
    ; mm0 = 0
    ; mm1 = weave pixels
    ; mm2 = movement map in the top
    ; mm3 = movement map in the centre
    ; mm4 = movement map in the bottom
    ; mm6 = Bob pixels          

    pcmpgtb mm2, [_STILL]           ; FF where still
    pcmpgtb mm3, [_STILL]           ; non-zero where mm3 > MOVE i.e. still
    pcmpgtb mm4, [_STILL]           ; non-zero where mm4 > MOVE i.e. still

    por  mm2, mm4                   ; top or bottom still
    pand  mm3, mm2                  ; middle and top or bottom still

   
	mov		eax, [ebp + %$Dest]
    DS_COMBINE mm1, mm6, mm3
    DS_MOVNTQ [eax + ecx], mm1

    add     ecx, 8
    cmp     ecx, [ebp + %$PixelCount]          ; done with line?
    jb      LoopYUY2%1

    add ebx, 8
    add edx, 8
    add esi, 8
    add edi, 8

    ; finish up
	; no diag on  last 8 bytes

	movq    mm2, [ebx]
	movq    mm5, [edx]
    movq    mm1, [esi]
    DS_PAVGB mm5, mm2, mm0, [ShiftMask]
	
    mov     eax, [ebp + %$T4]
    movq	mm2, [eax + ecx]

    movq    mm3, [edi]

    mov     eax, [ebp + %$B4]
    movq	mm4, [eax + ecx]
    
    pcmpgtb mm2, [_STILL]
    pcmpgtb mm3, [_STILL]
    pcmpgtb mm4, [_STILL]

    por  mm2, mm4                ; top or bottom still
    pand  mm3, mm2               ; where we should weave

	mov		eax, [ebp + %$Dest]
    DS_COMBINE mm1, mm5, mm3
    DS_MOVNTQ [eax + ecx], mm1

	mov esi, [esp+16]
	mov edi, [esp+20]
	mov ebx, [esp+24]

	emms
    
    %undef %1
endproc
%endmacro

;---------------------------------------------------------------------------
; Do diagonal Interpolating packed Values method
;---------------------------------------------------------------------------
%imacro Deint_Diag_Core_Packed 1

global _Deint_Diag_Core_Packed%1

proc _Deint_Diag_Core_Packed%1, 28

    %define %1 1

    %$M1 arg
    %$T2 arg
    %$B2 arg
    %$M3 arg
    %$T4 arg
    %$B4 arg
    %$Dest arg
    %$PixelCount arg

	mov [esp+16], esi
	mov [esp+20], edi
	mov [esp+24], ebx

	mov ebx, [ebp + %$T2]
	mov edx, [ebp + %$B2]
	mov	esi, [ebp + %$M1]
	mov	edi, [ebp + %$M3]

    ; PixelCount -= 8
    mov eax, [ebp + %$PixelCount]
    sub eax, 8
    mov [ebp + %$PixelCount], eax

	; no diagonal on first 8 bytes
	movq    mm2, [ebx]
	movq    mm5, [edx]
    movq    mm1, [esi]
    DS_PAVGB mm5, mm2, mm0, [ShiftMask]
	
    mov     eax, [ebp + %$T4]
    movq	mm2, [eax]

    movq    mm3, [edi]

    mov     eax, [ebp + %$B4]
    movq	mm4, [eax]
    
    pcmpgtb mm2, [_STILL]
    pcmpgtb mm3, [_STILL]
    pcmpgtb mm4, [_STILL]

    por  mm2, mm4                ; top or bottom still
    pand  mm3, mm2               ; where we should weave

	mov		eax, [ebp + %$Dest]
    DS_COMBINE mm1, mm5, mm3
    DS_MOVNTQ [eax], mm1

	mov		ecx, 8				; curr offset into all lines

LoopLuma%1:	
	add		esi, 8		
	add		edi, 8		
	add		ebx, 8
	add		edx, 8

	; First, get and save our possible Bob values
	; Assume our pixels are layed out as follows with x the calc'd bob value
	; and the other pixels are from the current field
	;  
	;         a b c 		current field
	;           x			calculated line
	;         d e f 		current field
	;

	movq    mm1, [ebx-1]
	movq    mm2, [ebx]
	movq    mm3, [ebx + 1]

	movq    mm4, [edx - 1]
	movq    mm5, [edx]
	movq    mm6, [edx + 1]

    ; mm1 = a
    ; mm2 = b
    ; mm3 = c
    ; mm4 = d
    ; mm5 = e
    ; mm6 = f

    movq mm7, mm1
    DS_PAVGB mm7, mm6, mm0, [ShiftMask]
    movq [esp], mm7

    movq mm7, mm3
    DS_PAVGB mm7, mm4, mm0, [ShiftMask]
    movq [esp + 8], mm7

    movq mm7, mm1
    DS_PABS mm1, mm6, mm0
    DS_PABS mm6, mm2, mm0
    DS_PABS mm7, mm5, mm0
    DS_PAVGB mm6, mm7, mm0, [ShiftMask]

    movq mm7, mm3
    DS_PABS mm3, mm4, mm0
    DS_PABS mm4, mm2, mm0
    DS_PABS mm7, mm5, mm0
    DS_PAVGB mm4, mm7, mm0, [ShiftMask]

    movq mm7, mm2
    DS_PABS mm2, mm5, mm0
    DS_PAVGB mm5, mm7, mm0, [ShiftMask]

    ; at this point the  following registers are set
    ; mm1 = |a - f|
    ; mm2 = |b - e|
    ; mm3 = |c - d|
    ; mm4 = avg(|b - d|, |c - e|)
    ; mm5 = avg(b, e)
    ; mm6 = avg(|a - e|, |b - f|)
    ; [esp] = avg(a, f)
    ; [esp + 8] = avg(c, d)
    
	pxor	mm7, mm7			
	pxor	mm0, mm0			
	psubusb mm4, mm6			; nonzero where mm4 > mm6 
	pcmpeqb mm4, mm0			; now ff where mm4 <= mm6
    movq    mm6, mm4
    pcmpeqb mm6, mm0            ; now ff where mm6 < mm4

    ; make the corresponding one unsuitable
    paddusb mm1, mm4
    paddusb mm3, mm6

    movq mm4, mm1
    movq mm6, mm3
    psubusb mm1, mm3            ; nonzero  where (|a - f| > |c - d|)
	pcmpeqb mm1, mm0			; now ff where (|a - f| < |c - d|)
    movq mm7, mm1
    DS_COMBINE mm4, mm6, mm1
    movq mm6, [esp]
    movq mm3, [esp + 8]
    DS_COMBINE mm6, mm3, mm7

    ; so now we should have mm4 as current best diff and mm6 as best average
    ; of the diagonals
    movq    mm1, mm2           ; save |b - e|
    psubusb mm2, mm4           ; nonzero  where (|b - e| >= Best)
	pcmpeqb mm2, mm0		   ; now ff where (|b - e| < Best)
    psubusb mm1, [_MOVE]        ; nonzero where |b - e| > MOVE is Bob is visible
	pcmpeqb mm1, mm0		   ; now ff where (|b - e| <= MOVE)
    por mm2, mm1               ; we'll let bob through always if the diff is small

    DS_COMBINE mm5, mm6, mm2
    movq mm6, mm5


    ;//////////////////////////////////////////////////////////////////////////
    ; get simple weave value
    ;//////////////////////////////////////////////////////////////////////////

    movq mm1, [esi]

    ;//////////////////////////////////////////////////////////////////////////
    ; Bottom
    ;//////////////////////////////////////////////////////////////////////////
    ; these should be the values held as we go in
    ; mm0 = 0
    ; mm1 = weave pixels
    ; mm6 = Bob pixels          

    mov     eax, [ebp + %$T4]
    movq	mm2, [eax + ecx]

    movq    mm3, [edi]

    mov     eax, [ebp + %$B4]
    movq	mm4, [eax + ecx]
    
    ; mm0 = 0
    ; mm1 = weave pixels
    ; mm2 = "movement" in the top
    ; mm3 = "movement" in the centre
    ; mm4 = "movement" in the bottom
    ; mm6 = Bob pixels          

    pcmpgtb mm2, [_STILL]
    pcmpgtb mm3, [_STILL]
    pcmpgtb mm4, [_STILL]

    por  mm2, mm4                ; top or bottom still
    pand  mm3, mm2               ; where we should weave

	mov		eax, [ebp + %$Dest]
    DS_COMBINE mm1, mm6, mm3
    DS_MOVNTQ [eax + ecx], mm1

    add     ecx, 8
    cmp     ecx, [ebp + %$PixelCount]          ; done with line?
    jb      LoopLuma%1

    add ebx, 8
    add edx, 8
    add esi, 8
    add edi, 8

    ; finish up
	; no diag on  last 8 bytes

	movq    mm2, [ebx]
	movq    mm5, [edx]
    movq    mm1, [esi]
    DS_PAVGB mm5, mm2, mm0, [ShiftMask]
	
    mov     eax, [ebp + %$T4]
    movq	mm2, [eax + ecx]

    movq    mm3, [edi]

    mov     eax, [ebp + %$B4]
    movq	mm4, [eax + ecx]
    
    pcmpgtb mm2, [_STILL]
    pcmpgtb mm3, [_STILL]
    pcmpgtb mm4, [_STILL]

    por  mm2, mm4                ; top or bottom still
    pand  mm3, mm2               ; where we should weave

	mov		eax, [ebp + %$Dest]
    DS_COMBINE mm1, mm5, mm3
    DS_MOVNTQ [eax + ecx], mm1
	
	mov esi, [esp+16]
	mov edi, [esp+20]
	mov ebx, [esp+24]

	emms

    %undef %1
endproc
%endmacro

; creates external C function _Deint_Diag_Core_YUY2_MMX
Deint_Diag_Core_YUY2 _MMX

; creates external C function _Deint_Diag_Core_YUY2_3DNOW
Deint_Diag_Core_YUY2 _3DNOW

; creates external C function _Deint_Diag_Core_YUY2_SSE
Deint_Diag_Core_YUY2 _SSE

; creates external C function _Deint_Diag_Core_Packed_MMX
Deint_Diag_Core_Packed _MMX

; creates external C function _Deint_Diag_Core_Packed_3DNOW
Deint_Diag_Core_Packed _3DNOW

; creates external C function _Deint_Diag_Core_Packed_SSE
Deint_Diag_Core_Packed _SSE

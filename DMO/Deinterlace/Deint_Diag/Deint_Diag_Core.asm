;///////////////////////////////////////////////////////////////////////////////
;// $Id: Deint_Diag_Core.asm,v 1.3 2003-09-24 16:33:00 adcockj Exp $
;///////////////////////////////////////////////////////////////////////////////
;// CVS Log
;//
;// $Log: not supported by cvs2svn $
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

segment .text

;---------------------------------------------------------------------------
; Do diagonal Interpolating YUY2 method
;---------------------------------------------------------------------------
%imacro Deint_Diag_Core_YUY2 1

global _Deint_Diag_Core_YUY2%1

proc _Deint_Diag_Core_YUY2%1

    %define %1 1

    %$M1 arg
    %$T2 arg
    %$B2 arg
    %$M3 arg
    %$T4 arg
    %$B4 arg
    %$Dest arg
    %$PixelCount arg

    sub     esp, 24                 ; 24 bytes of local stack space 
	
	mov [esp+16], esi
	mov [esp+20], edi

	mov ebx, [ebp + %$T2]
	mov edx, [ebp + %$B2]
	mov edi, [ebp + %$Dest]

    ; PixelCount -= 8
    mov eax, [ebp + %$PixelCount]
    sub eax, 8
    mov [ebp + %$PixelCount], eax

	; simple bob first 8 bytes
	movq mm0, [ebx]
	DS_PAVGB mm0, [edx], mm2, [ShiftMask]

	mov		eax, [ebp + %$Dest]
	DS_MOVNTQ [eax], mm0

	; now loop over the stuff in the middle
	mov		esi, [ebp + %$M1]
	mov		edi, [ebp + %$M3]

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

    movq mm1, [edi]
    movq mm2, [esi]
    movq mm3, mm1			; another copy of our pixel1 value
    movq mm4, mm2			; another copy of our pixel2 value

    DS_PABS mm3, mm4, mm5

    DS_PAVGB mm1, mm2, mm4, [ShiftMask]  ; avg of 2 pixels

    ; these must be the values as we exit
    ; mm0 = 0
    ; mm1 = weave pixels
    ; mm3 = "movement" in the centre
    ; mm6 = Bob pixels          

    ;//////////////////////////////////////////////////////////////////////////
    ; Bottom
    ;//////////////////////////////////////////////////////////////////////////
    ; these should be the values held as we go in
    ; mm0 = 0
    ; mm1 = weave pixels
    ; mm3 = "movement" in the centre
    ; mm6 = Bob pixels          

    ; operate only on luma as we will always bob the chroma
    pand    mm3, [YMask]

    mov     eax, [ebp + %$T4]
    movq	mm2, [ebx]
    movq	mm4, [eax + ecx]

    DS_PABS mm2, mm4, mm5
    pand    mm2, [YMask]

    mov     eax, [ebp + %$B4]
    movq	mm4, [edx]
    movq	mm7, [eax + ecx]

    DS_PABS mm4, mm7, mm5
    pand    mm4, [YMask]
    
    ; mm0 = 0
    ; mm1 = weave pixels
    ; mm2 = "movement" in the top
    ; mm3 = "movement" in the centre
    ; mm4 = "movement" in the bottom
    ; mm6 = Bob pixels          

    psubusb mm2, [_MOVE]           ; non-zero where mm2 > MOVE i.e. Movement
    pcmpeqd mm2, mm0            ; FFFF where the luma has no movement in two pixels
    pcmpeqd mm2, mm0            ; all ff where movement in either of the two pixels

    psubusb mm3, [_MOVE]           ; non-zero where mm3 > MOVE i.e. Movement
    pcmpeqd mm3, mm0            ; FFFF where the luma has no movement in two pixels
    pcmpeqd mm3, mm0            ; all ff where movement in either of the two pixels

    psubusb mm4, [_MOVE]           ; non-zero where mm4 > MOVE i.e. Movement
    pcmpeqd mm4, mm0            ; FFFF where the luma has no movement in two pixels
    pcmpeqd mm4, mm0            ; all ff where movement in either of the two pixels

    pand  mm2, mm4              ; top and bottom moving
    por  mm3, mm2               ; where we should bob

    por mm3, [UVMask]

	mov		eax, [ebp + %$Dest]
    DS_COMBINE mm6, mm1, mm3
    DS_MOVNTQ [eax+ecx], mm6

    add     ecx, 8
    cmp     ecx, [ebp + %$PixelCount]          ; done with line?
    jb      LoopYUY2%1

    add ebx, 8
    add edx, 8

    ; finish up
	; simple bob last 8 bytes
	movq mm0, [ebx]

	mov		eax, [ebp + %$Dest]
	DS_PAVGB mm0, [edx], mm2, [ShiftMask]
	DS_MOVNTQ [eax + ecx], mm0

	mov esi, [esp+16]
	mov edi, [esp+20]

	emms
    
    %undef %1
endproc
%endmacro

;---------------------------------------------------------------------------
; Do diagonal Interpolating packed Luma method
;---------------------------------------------------------------------------
%imacro Deint_Diag_Core_Luma 1

global _Deint_Diag_Core_Luma%1

proc _Deint_Diag_Core_Luma%1

    %define %1 1

    %$M1 arg
    %$T2 arg
    %$B2 arg
    %$M3 arg
    %$T4 arg
    %$B4 arg
    %$Dest arg
    %$PixelCount arg

    sub     esp, 24                 ; 24 bytes of local stack space 
	
	mov [esp+16], esi
	mov [esp+20], edi

	mov ebx, [ebp + %$T2]
	mov edx, [ebp + %$B2]
	mov edi, [ebp + %$Dest]

    ; PixelCount -= 8
    mov eax, [ebp + %$PixelCount]
    sub eax, 8
    mov [ebp + %$PixelCount], eax

	; simple bob first 8 bytes
	movq mm0, [ebx]
	DS_PAVGB mm0, [edx], mm2, [ShiftMask]

	mov		eax, [ebp + %$Dest]
	DS_MOVNTQ [eax], mm0

	; now loop over the stuff in the middle
	mov		esi, [ebp + %$M1]
	mov		edi, [ebp + %$M3]

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
    ; simple weave
    ;//////////////////////////////////////////////////////////////////////////

    ; at the start of this function
    ; mm0 = 0
    ; mm6 = Bob pixels          

    movq mm1, [edi]
    movq mm2, [esi]
    movq mm3, mm1			; another copy of our pixel1 value
    movq mm4, mm2			; another copy of our pixel2 value

    DS_PABS mm3, mm4, mm5

    DS_PAVGB mm1, mm2, mm4, [ShiftMask]  ; avg of 2 pixels

    ; these must be the values as we exit
    ; mm0 = 0
    ; mm1 = weave pixels
    ; mm3 = "movement" in the centre
    ; mm6 = Bob pixels          

    ;//////////////////////////////////////////////////////////////////////////
    ; Bottom
    ;//////////////////////////////////////////////////////////////////////////
    ; these should be the values held as we go in
    ; mm0 = 0
    ; mm1 = weave pixels
    ; mm3 = "movement" in the centre
    ; mm6 = Bob pixels          

    mov     eax, [ebp + %$T4]
    movq	mm2, [ebx]
    movq	mm4, [eax + ecx]

    DS_PABS mm2, mm4, mm5

    mov     eax, [ebp + %$B4]
    movq	mm4, [edx]
    movq	mm7, [eax + ecx]

    DS_PABS mm4, mm7, mm5
    
    ; mm0 = 0
    ; mm1 = weave pixels
    ; mm2 = "movement" in the top
    ; mm3 = "movement" in the centre
    ; mm4 = "movement" in the bottom
    ; mm6 = Bob pixels          

    psubusb mm2, [_MOVE]           ; non-zero where mm2 > MOVE i.e. Movement
    pcmpeqw mm2, mm0            ; FFFF where the luma has no movement in two pixels
    pcmpeqw mm2, mm0            ; all ff where movement in either of the two pixels

    psubusb mm3, [_MOVE]           ; non-zero where mm3 > MOVE i.e. Movement
    pcmpeqw mm3, mm0            ; FFFF where the luma has no movement in two pixels
    pcmpeqw mm3, mm0            ; all ff where movement in either of the two pixels

    psubusb mm4, [_MOVE]           ; non-zero where mm4 > MOVE i.e. Movement
    pcmpeqw mm4, mm0            ; FFFF where the luma has no movement in two pixels
    pcmpeqw mm4, mm0            ; all ff where movement in either of the two pixels

    pand  mm2, mm4              ; top and bottom moving
    por  mm3, mm2               ; where we should bob

	mov		eax, [ebp + %$Dest]
    DS_COMBINE mm6, mm1, mm3
    DS_MOVNTQ [eax+ecx], mm6

    add     ecx, 8
    cmp     ecx, [ebp + %$PixelCount]          ; done with line?
    jb      LoopLuma%1

    add ebx, 8
    add edx, 8

    ; finish up
	; simple bob last 8 bytes
	movq mm0, [ebx]

	mov		eax, [ebp + %$Dest]
	DS_PAVGB mm0, [edx], mm2, [ShiftMask]
	DS_MOVNTQ [eax + ecx], mm0
	
	mov esi, [esp+16]
	mov edi, [esp+20]

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

; creates external C function _Deint_Diag_Core_Luma_MMX
Deint_Diag_Core_Luma _MMX

; creates external C function _Deint_Diag_Core_Luma_3DNOW
Deint_Diag_Core_Luma _3DNOW

; creates external C function _Deint_Diag_Core_Luma_SSE
Deint_Diag_Core_Luma _SSE

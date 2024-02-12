**************************************************************************************
*	GRF_B4_S.S
*
*	4 BitPlane Blitter Rendering Functions
*
*	[c] 2002 Reservoir Gods
*       2015 Modified for ToyBox.
**************************************************************************************



**************************************************************************************
*	EXPORTS / IMPORTS
**************************************************************************************

|	.global	_m68_cgimage_set
|	.global	_m68_cgimage_get
	.global	_m68_cgimage_draw_aligned
        .global _m68_cgimage_draw
        .global    _m68_cgimage_draw_rect

**************************************************************************************
*	STRUCTS
**************************************************************************************

    .struct
s_cgpoint_x:                     ds.w	1
s_cgpoint_y:                     ds.w	1
s_cgpoint_sizeof:		ds.w	1


    .struct
s_cgrect_x:                      ds.w	1
s_cgrect_y:                      ds.w	1
s_cgrect_width:                  ds.w	1
s_cgrect_height:                 ds.w	1
s_cgrect_sizeof:                 ds.w	1


    .struct
s_cgimage_superImage:            ds.l    1
s_cgimage_palette:               ds.l    1
s_cgimage_width:                 ds.w    1
s_cgimage_height:                ds.w    1
s_cgimage_x:                     ds.w    1
s_cgimage_y:                     ds.w    1
s_cgimage_lineWords:             ds.w	1
s_cgimage_bitmap:                ds.l	1
s_cgimage_maskmap:               ds.l	1
s_cgimage_owns_bitmap:           ds.b    1
s_cgimage_sizeof:                ds.w	1

  .struct
sBLITTER_HALFTONE:		ds.w 16  | 0x00
sBLITTER_SRC_INC_X:		ds.w  1  | 0x20
sBLITTER_SRC_INC_Y:		ds.w  1  | 0x22
sBLITTER_pSRC:			ds.l  1  | 0x24
sBLITTER_ENDMASK_1:		ds.w  1  | 0x28
sBLITTER_ENDMASK_2:		ds.w  1  | 0x2A
sBLITTER_ENDMASK_3:		ds.w  1  | 0x2C
sBLITTER_DST_INC_X:		ds.w  1  | 0x2E
sBLITTER_DST_INC_Y:		ds.w  1  | 0x30
sBLITTER_pDST:			ds.l  1  | 0x32
sBLITTER_COUNT_X:		ds.w  1  | 0x36
sBLITTER_COUNT_Y:		ds.w  1  | 0x38
sBLITTER_HOP:			ds.b  1  | 0x3A
sBLITTER_LOP:			ds.b  1  | 0x3B
sBLITTER_MODE:			ds.b  1  | 0x3C
sBLITTER_SKEW:			ds.b  1  | 0x3D
sBLITTER_sizeof:                ds.w  1

**************************************************************************************
*	DEFINES
**************************************************************************************

.equ eBLITTER_BASE				,0xFFFF8A00

.equ eBLITTERLOP_ZERO				,0
.equ eBLITTERLOP_SRC_AND_DST			,1
.equ eBLITTERLOP_SRC_ANDNOT_DST			,2
.equ eBLITTERLOP_SRC				,3
.equ eBLITTERLOP_NOTSRC_AND_DST			,4
.equ eBLITTERLOP_DST				,5
.equ eBLITTERLOP_SRC_XOR_DST			,6
.equ eBLITTERLOP_SRC_OR_DST			,7
.equ eBLITTERLOP_NOTSRC_ANDNOT_DST		,8
.equ eBLITTERLOP_NOTSRC_XOR_DST			,9
.equ eBLITTERLOP_NOTDST				,10
.equ eBLITTERLOP_SRC_ORNOT_DST			,11
.equ eBLITTERLOP_NOTSRC				,12
.equ eBLITTERLOP_NOTSRC_OR_DST			,13
.equ eBLITTERLOP_NOTSRC_ORNOT_DST		,14
.equ eBLITTERLOP_ONE				,15

.equ eBLITTERHOP_ONE				,0
.equ eBLITTERHOP_HALFTONE			,1
.equ eBLITTERHOP_SRC				,2
.equ eBLITTERHOP_SRC_AND_HALFTONE		,3

.equ eBLITTERMODE_LINENUMBER_MASK		,0x0F
.equ eBLITTERMODE_SMUDGE_BIT			,0x20
.equ eBLITTERMODE_SMUDGE_MASK			,0xDF
.equ eBLITTERMODE_HOG_BIT			,0x40
.equ eBLITTERMODE_HOG_MASK			,0xBF
.equ eBLITTERMODE_BUSY_BIT			,0x80
.equ eBLITTERMODE_BUSY_BIT_NUM			,7
.equ eBLITTERMODE_BUSY_MASK			,0x7F

.equ eBLITTERSKEW_SKEW_MASK			,0x0F
.equ eBLITTERSKEW_NFSR_BIT			,0x40
.equ eBLITTERSKEW_NFSR_MASK			,0xBF
.equ eBLITTERSKEW_FXSR_BIT			,0x80
.equ eBLITTERSKEW_FXSR_MASK			,0x7F
.equ eBLITTERSKEW_NFSR_FXSR_BIT			,0xC0


**************************************************************************************
	.text
**************************************************************************************


|
| void pCGImageSetPixel(TGImageRef image, TGPoint point, TGColorIndex color)
|
_m68_cgimage_set:
    rts

|
| TGColorIndex pCGImageGetPixel(TGImageRef image, TGPoint point)
|
_m68_cgimage_get:
    moveq.l #0,d0
    rts

|
| m68_cgimage_draw_aligned(cgimage_t *dst, cgimage_t *src, cgpoint_t point)|
|
_m68_cgimage_draw_aligned:
    move.l	a2,-(sp)
|	_TGBlitter *blitter = (void *)0xffff8240|
    lea		(eBLITTER_BASE+sBLITTER_SRC_INC_X).w,a2
|	TCInteger words{d1} = (srcImage->size.width >> 4)|
    move.w	s_cgimage_width(a1),d1
    asr.w	#4,d1
|	blitter->SRC_INC_X = 2|
    move.w	#2,(a2)+
|	blitter->SRC_INC_Y = ((srcImage->lineWords - words) << 3) + 2|
    move.w	s_cgimage_lineWords(a1),d2
    sub.w	d1,d2
    asl.w	#3,d2
    addq.w	#2,d2
    move.w	d2,(a2)+
|	blitter->pSRC = srcImage->bitmap|
    move.l	s_cgimage_bitmap(a1),(a2)+
|	blitter->ENDMASK_1 = 0xffff|
|	blitter->ENDMASK_2 = 0xffff|
|	blitter->ENDMASK_3 = 0xffff|
    moveq.l	#-1,d2
    move.l	d2,(a2)+
    move.w	d2,(a2)+

|	blitter->DST_INC_X = 2|
    move.w	#2,(a2)+
|	blitter->DST_INC_Y = ((image->lineWords - words) << 3) + 2|
    move.w	s_cgimage_lineWords(a0),d2
    sub.w	d1,d2
    asl.w	#3,d2
    addq.w	#2,d2
    move.w	d2,(a2)+
|	TCInteger dstOffsYBase{d2} = (point.x >> 4) + image->lineWords * point.y|
|	blitter->pDST = image->bitmap + (dstOffsYBase << 3)|
    move.w	d0,d2
    mulu	s_cgimage_lineWords(a0),d2
    swap	d0
    ext.l	d0
    asr.w	#4,d0
    add.l	d0,d2
    asl.l	#3,d2
    add.l	s_cgimage_bitmap(a0),d2
    move.l	d2,(a2)+

|	blitter->COUNT_X = (words << 2)|
    asl.w	#2,d1
    move.w	d1,(a2)+
|	blitter->COUNT_Y = srcImage->size.height|
    move.w	s_cgimage_height(a1),(a2)+

|	blitter->HOP = 2|
|	blitter->LOP = 3|
    move.w	#0x0203,(a2)+
|	blitter->SKEW = 0|
    clr.b	(eBLITTER_BASE+sBLITTER_SKEW).w

|	blitter->MODE = 0x80|
    move.b  #eBLITTERMODE_BUSY_BIT,(a2)
    nop
.wait_1:
    bset.b  #7,(a2)					      	| restart blitter if needed
    nop
    bne.s   .wait_1
    move.l	(sp)+,a2
    rts
	
|
| m68_cgimage_draw(cgimage_t *dst, cgimage_t *src, cgpoint_t point)
|
_m68_cgimage_draw:
    movem.l	d3-d6/a2,-(a7)	   		| a0 = image, a1 = srcCtx
    swap	d0						| point.y{d0:high} for later

|    TCInteger width{d5} = (srcImage->size.width){s_cgimage_width(a1)} - 1|
    move.w	s_cgimage_width(a1),d5
    subq.w	#1,d5
|    TCInteger leftEndMask{d1} = endMasks[point.x{d0} % 16]|
    moveq.l	#0xf,d6
    move.w	d0,d1
    and.w	d6,d1
    move.w	d1,d3			| (point.x % 16{d6}){d3} for later
    add.w	d1,d1
    lea		gGraphic_EndMasks(pc),a2	| endMasks{a2} for later
    move.w	(a2,d1.w),d1

|    TCInteger rightEndMask{d2} = endMasks{a2}[(point.x{d0} + width{d5}) % 16{d6} + 1]|
    move.w	d0,d2
    add.w	d5,d2
    and.w	d6,d2
    add.w	d2,d2
    move.w	2(a2,d2.w),d2
    not.w	d2

|    blitter->HOP = 2|
    move.b	#eBLITTERHOP_SRC,(eBLITTER_BASE+sBLITTER_HOP).w
|    blitter->DST_INC_X = 8|
    move.w	#8,(eBLITTER_BASE+sBLITTER_DST_INC_X).w

|    int8_t skew{d3} = ((point.x % 16){d3}|
|    int8_t flags{d4} = 0|
    clr.w	d4

|    TCInteger dstWords{d6} = ((point.x{d0} + width{d5}) >> 4) - (point.x{d0} >> 4)|
    move.w	d0,d6
    add.w	d5,d6
    asr.w	#4,d6
    asr.w	#4,d0		| (point.x >> 4){d0} for later
    sub.w	d0,d6

|    TCInteger srcWords{d5} = (width{d5} >> 4)|
    asr.w	#4,d5

|    if (dstWords{d6} == 0) {
    bne.s	.not_zero_2
|      blitter->ENDMASK_1 = leftEndMask{d1} & rightEndMask{d2}|
    and.w	d2,d1
    move.w	d1,(eBLITTER_BASE+sBLITTER_ENDMASK_1).w
|      flags{d4} += 4|
    addq.w	#4,d4
|    } else {
    bra.s	.was_zero_2
.not_zero_2:
|      blitter->ENDMASK_1 = leftEndMask{d1}|
    move.w	d1,(eBLITTER_BASE+sBLITTER_ENDMASK_1).w
|      blitter->ENDMASK_2 = 0xffff|
    move.w	#0xffff,(eBLITTER_BASE+sBLITTER_ENDMASK_2).w
|      blitter->ENDMASK_3 = rightEndMask{d2}|
    move.w	d2,(eBLITTER_BASE+sBLITTER_ENDMASK_3).w
|    }
.was_zero_2:
|    if (srcWords{}d5 == dstWords{d6}) {
    cmp.w	d5,d6
    bne.s	.not_same_2
|      flags{d4} += 2|
    addq.w	#2,d4
|    }
.not_same_2:
|    skew{d3} |= skewFlags[flags{d4}]|
    lea		gGraphic_4BP_SkewFlags(pc),a2
    or.b	(a2,d4.w),d3
|    blitter->SKEW = skew{d3}|
    move.b	d3,(eBLITTER_BASE+sBLITTER_SKEW).w

|    TCInteger srcIncYBase{d1} = srcImage->lineWords{s_cgimage_lineWord(a1)} - srcWords{d5}|
    move.w	s_cgimage_lineWords(a1),d1
    move.w	d1,d4			| srcImage->lineWords{d4} for later
    sub.w	d5,d1
|    blitter->DST_INC_Y = (image->lineWords{s_cgimage_lineWords(a0)} - dstWords{d6}) << 3|
    move.w	s_cgimage_lineWords(a0),d2
    move.w	d2,d3			| image->lineWords{d3} for later
    sub.w	d6,d2
    asl.w	#3,d2
    move.w	d2,(eBLITTER_BASE+sBLITTER_DST_INC_Y).w
|    TCInteger dstOffsYBase{d3.l} = (point.x >> 4){d0} + image->lineWords{d3} * point.y{d0:high}|
    swap	d0
    mulu	d0,d3
    swap	d0
    add.w	d0,d3
    ext.l	d3
  
|    blitter->COUNT_X = dstWords{d6} + 1|
    addq.w	#1,d6
    move.w	d6,(eBLITTER_BASE+sBLITTER_COUNT_X).w
|    TCInteger countY{d0} = srcImage->size.height{sTGContex_height(a1)}|
    move.w	s_cgimage_height(a1),d0
  
|    void *pDST{a0} = image->bitmap{s_cgimage_bitmap(a0)} + (dstOffsYBase{d3} << 3)|
    move.l	s_cgimage_bitmap(a0),a0
    asl.l	#3,d3
    add.l	d3,a0
	
    lea		(eBLITTER_BASE+sBLITTER_MODE).w,a2			| {a2} fast blitter mode access
|    if (srcImage->maskmap{s_cgimage_maskmap(a1)} != NULL) {
    move.l	s_cgimage_maskmap(a1),d3				| srcImage->maskmap{d3} for later
    beq.s	.no_mask_2
|      blitter->LOP = 1|
    move.b	#eBLITTERLOP_NOTSRC_AND_DST,(eBLITTER_BASE+sBLITTER_LOP).w
|      blitter->SRC_INC_X = 2|
    move.w	#2,(eBLITTER_BASE+sBLITTER_SRC_INC_X).w
|      blitter->SRC_INC_Y = srcIncYBase{d1} << 1|
    add.w	d1,d1				| (srcIncYBase << 1){d1}
    move.w	d1,(eBLITTER_BASE+sBLITTER_SRC_INC_Y).w
|      for (int bp = 0| bp < 4| bp++) {
    moveq.l	#3,d6
    move.l	a0,d4				| pDST{d4} temp
.mask_loop_2:
|        blitter->pSRC = srcImage->maskmap{d3}|
    move.l	d3,(eBLITTER_BASE+sBLITTER_pSRC).w
|        blitter->pDST = (pDST + (bp << 1)){d4}|
    move.l	d4,(eBLITTER_BASE+sBLITTER_pDST).w
|        blitter->COUNT_Y = countY{d0}|
    move.w	d0,(eBLITTER_BASE+sBLITTER_COUNT_Y).w
|        blitter->MODE = 0x80|
    move.b  #eBLITTERMODE_BUSY_BIT,(a2)
    addq.w  #2,d4
.wait_mask_2:
    bset.b  #7,(a2)					      	| restart blitter if needed
    nop
    bne.s   .wait_mask_2
|      }
    dbra	d6,.mask_loop_2
|      blitter->LOP = 7|
    move.b	#eBLITTERLOP_SRC_OR_DST,(eBLITTER_BASE+sBLITTER_LOP).w
    asl.w	#2,d1							| (srcIncYBase << 3){d1}
    bra.s	.had_mask_2
|    } else {
.no_mask_2:
|      blitter->LOP = 3|
    move.b	#eBLITTERLOP_SRC,(eBLITTER_BASE+sBLITTER_LOP).w
    asl.w	#3,d1							| (srcIncYBase << 3){d1}
|    }
.had_mask_2:
|    blitter->SRC_INC_X = 8|
    move.w	#8,(eBLITTER_BASE+sBLITTER_SRC_INC_X).w
|    blitter->SRC_INC_Y = (srcIncYBase << 3){d1}|
    move.w	d1,(eBLITTER_BASE+sBLITTER_SRC_INC_Y).w
|    for (int bp = 0| bp < 4| bp++) {
    moveq.l	#3,d6
    move.l	s_cgimage_bitmap(a1),a1	| srcImage->bitmap{a1}
.bitmap_loop_2:
|      blitter->pSRC = (srcImage->bitmap + (srcOffsYBase << 3) + (bp << 1)){a1}|
    move.l	a1,(eBLITTER_BASE+sBLITTER_pSRC).w
|      blitter->pDST = (pDST + (bp << 1)){a0}|
    move.l	a0,(eBLITTER_BASE+sBLITTER_pDST).w
|      blitter->COUNT_Y = countY{d0}|
    move.w	d0,(eBLITTER_BASE+sBLITTER_COUNT_Y).w
|        blitter->MODE = 0x80|
    move.b  #eBLITTERMODE_BUSY_BIT,(a2)
    addq.l	#2,a0
    addq.l	#2,a1
.wait_bitmap_2:
    bset.b  #7,(a2)					      	| restart blitter if needed
    nop
    bne.s   .wait_bitmap_2
|    }
    dbra	d6,.bitmap_loop_2

    movem.l	(a7)+,d3-d6/a2
    rts
	
|
| m68_cgimage_draw_rect(cgimage_t *image, cgimage_t *src, cgrect_t *rect, cgpoint_t point)
|
_m68_cgimage_draw_rect:
    movem.l	d3-d7/a2-a3,-(a7)	   	| a0 = image, a1 = srcCtx
    move.l  7*4+4(sp),a2		   	| a2 = rect
    swap	d0						| point.y{d0:high} for later
  
|    TCInteger width{d6} = (rect->size.width){s_cgrect_width(a2)} - 1|
    move.w	s_cgrect_width(a2),d6
    subq.w	#1,d6
|    TCInteger leftEndMask{d1} = endMasks[point.x{d0} % 16]|
    moveq.l	#0xf,d7
    move.w	d0,d1
    and.w	d7,d1
    move.w	d1,d3			| (point.x % 16{d7}){d3} for later
    add.w	d1,d1
    lea		gGraphic_EndMasks(pc),a3	| endMasks{a3} for later
    move.w	(a3,d1.w),d1

|    TCInteger rightEndMask{d2} = endMasks{a3}[(point.x{d0} + width{d6}) % 16{d7} + 1]|
    move.w	d0,d2
    add.w	d6,d2
    and.w	d7,d2
    add.w	d2,d2
    move.w	2(a3,d2.w),d2
    not.w	d2

|    blitter->HOP = 2|
    move.b	#eBLITTERHOP_SRC,(eBLITTER_BASE+sBLITTER_HOP).w
|    blitter->DST_INC_X = 8|
    move.w	#8,(eBLITTER_BASE+sBLITTER_DST_INC_X).w|

|    int8_t skew{d3} = ((point.x % 16){d3} - rect->origin.x{s_cgrect_x(a2)} % 16{d7})|
    move.w	s_cgrect_x(a2),d4
    move.w	d4,d5			| rect->origin.x{d5} for later
    and.w	d7,d4
    sub.w	d4,d3
|    int8_t flags{d4} = (skew{d3} < 0) ? 1 : 0|
    clr.w	d4
    addx.w	d4,d4
|    skew{d3} %= 16{d7}|
    and.w	d7,d3

|    TCInteger srcWords{d5} = ((rect->origin.x{d5} + width{d6}) >> 4) - (rect->origin.x{d5} >> 4)|
    move.w	d5,d7
    add.w	d6,d5
    asr.w	#4,d5
    asr.w	#4,d7			| (rect->origin.x >> 4){d7} for later
    sub.w	d7,d5

|    TCInteger dstWords{d6} = ((point.x{d0} + width{d6}) >> 4) - (point.x{d0} >> 4)|
    add.w	d0,d6
    asr.w	#4,d6
    asr.w	#4,d0		| (point.x >> 4){d0} for later
    sub.w	d0,d6

|    if (dstWords{d6} == 0) {
    bne.s	.not_zero_3
|      blitter->ENDMASK_1 = leftEndMask{d1} & rightEndMask{d2}|
    and.w	d2,d1
    move.w	d1,(eBLITTER_BASE+sBLITTER_ENDMASK_1).w
|      flags{d4} += 4|
    addq.w	#4,d4
|    } else {
    bra.s	.was_zero_3
.not_zero_3:
|      blitter->ENDMASK_1 = leftEndMask{d1}|
    move.w	d1,(eBLITTER_BASE+sBLITTER_ENDMASK_1).w
|      blitter->ENDMASK_2 = 0xffff|
    move.w	#0xffff,(eBLITTER_BASE+sBLITTER_ENDMASK_2).w
|      blitter->ENDMASK_3 = rightEndMask{d2}|
    move.w	d2,(eBLITTER_BASE+sBLITTER_ENDMASK_3).w
|    }
.was_zero_3:
|    if (srcWords{}d5 == dstWords{d6}) {
    cmp.w	d5,d6
    bne.s	.not_same_3
|      flags{d4} += 2|
    addq.w	#2,d4
|    }
.not_same_3:
|    skew{d3} |= skewFlags[flags{d4}]|
    lea		gGraphic_4BP_SkewFlags(pc),a3
    or.b	(a3,d4.w),d3
|    blitter->SKEW = skew{d3}|
    move.b	d3,(eBLITTER_BASE+sBLITTER_SKEW).w

|    TCInteger srcIncYBase{d1} = srcImage->lineWords{s_cgimage_lineWord(a1)} - srcWords{d5}|
    move.w	s_cgimage_lineWords(a1),d1
    move.w	d1,d4			| srcImage->lineWords{d4} for later
    sub.w	d5,d1
|    blitter->DST_INC_Y = (image->lineWords{s_cgimage_lineWords(a0)} - dstWords{d6}) << 3|
    move.w	s_cgimage_lineWords(a0),d2
    move.w	d2,d3			| image->lineWords{d3} for later
    sub.w	d6,d2
    asl.w	#3,d2
    move.w	d2,(eBLITTER_BASE+sBLITTER_DST_INC_Y).w
|    TCInteger srcOffsYBase{d2.l} = ((rect->origin.x >> 4){d7} + srcImage->lineWords{d4} * rect->origin.y{s_cgrect_y(a2)})|
    move.w	s_cgrect_y(a2),d2
    mulu	d4,d2
    add.w	d7,d2
    ext.l	d2
|    TCInteger dstOffsYBase{d3.l} = (point.x >> 4){d0} + image->lineWords{d3} * point.y{d0:high}|
    swap	d0
    mulu	d0,d3
    swap	d0
    add.w	d0,d3
    ext.l	d3
  
|    blitter->COUNT_X = dstWords{d6} + 1|
    addq.w	#1,d6
    move.w	d6,(eBLITTER_BASE+sBLITTER_COUNT_X).w
|    TCInteger countY{d0} = rect->size.height{s_cgrect_height(a2)}|
    move.w	s_cgrect_height(a2),d0
  
|    void *pDST{a2} = image->bitmap{s_cgimage_bitmap(a0)} + (dstOffsYBase{d3} << 3)|
    move.l	s_cgimage_bitmap(a0),a2
    asl.l	#3,d3
    add.l	d3,a2
	
|    if (srcImage->maskmap{s_cgimage_maskmap(a1)} != NULL) {
    move.l	s_cgimage_maskmap(a1),d3				| srcImage->maskmap{d3} for later
    beq		.no_mask_3
    add.l	d2,d2				| (srcOffsYBase{d2} << 1){d2}
    add.l	d2,d3
|      blitter->LOP = 1|
    move.b	#eBLITTERLOP_NOTSRC_AND_DST,(eBLITTER_BASE+sBLITTER_LOP).w
|      blitter->SRC_INC_X = 2|
    move.w	#2,(eBLITTER_BASE+sBLITTER_SRC_INC_X).w
|      blitter->SRC_INC_Y = srcIncYBase{d1} << 1|
    add.w	d1,d1				| (srcIncYBase << 1){d1}
    move.w	d1,(eBLITTER_BASE+sBLITTER_SRC_INC_Y).w
|    if (TGImageHasMask(image)) {
    move.l	s_cgimage_maskmap(a0),d4		| image->maskmap{d4} for later
    beq.s	.no_dst_mask
|      blitter->DST_INC_X = 2|
    move.w	#2,(eBLITTER_BASE+sBLITTER_DST_INC_X).w
|      blitter->DST_INC_Y /= 4|
    move.w	(eBLITTER_BASE+sBLITTER_DST_INC_Y).w,d7
    move.w	d7,d6							| blitter->DST_INC_Y{d6} for reset later
    asr.w	#2,d7
    move.w	d7,(eBLITTER_BASE+sBLITTER_DST_INC_Y).w
|      blitter->pSRC = srcImage->maskmap + (srcOffsYBase << 1)|
    move.l	d3,(eBLITTER_BASE+sBLITTER_pSRC).w
|      blitter->pDST = image->maskmap + (dstOffsYBase << 1)|
    move.l	a2,d7
    sub.l	s_cgimage_maskmap(a0),d7
    asr.l	#2,d7
    add.l	d7,d4
    move.l	d4,(eBLITTER_BASE+sBLITTER_pDST).w
|      blitter->COUNT_Y = countY|
    move.w	d0,(eBLITTER_BASE+sBLITTER_COUNT_Y).w
|      blitter->MODE = 0x80|
    nop
    move.b  #eBLITTERMODE_BUSY_BIT,(eBLITTER_BASE+sBLITTER_MODE).w
    nop
.wait_dst_mask_3:
    bset.b  #7,(eBLITTER_BASE+sBLITTER_MODE).w			      	| restart blitter if needed
    nop
    bne.s   .wait_dst_mask_3
|    blitter->DST_INC_X = 8|
    move.w	#8,(eBLITTER_BASE+sBLITTER_DST_INC_X).w
|    blitter->DST_INC_Y *= 4|
    move.w	d6,(eBLITTER_BASE+sBLITTER_DST_INC_Y).w
|    }
.no_dst_mask:
|      for (int bp = 0| bp < 4| bp++) {
    moveq.l	#3,d7
    move.l	a2,d4				| pDST{d4} temp
    lea		(eBLITTER_BASE+sBLITTER_MODE).w,a0			| {a0} fast blitter mode access
.mask_loop_3:
|        blitter->pSRC = (srcImage->maskmap + (srcOffsYBase << 1){d2}){d3}|
    move.l	d3,(eBLITTER_BASE+sBLITTER_pSRC).w
|        blitter->pDST = (pDST + (bp << 1)){d4}|
    move.l	d4,(eBLITTER_BASE+sBLITTER_pDST).w
|        blitter->COUNT_Y = countY{d0}|
    move.w	d0,(eBLITTER_BASE+sBLITTER_COUNT_Y).w
|        blitter->MODE = 0x80|
    move.b  #eBLITTERMODE_BUSY_BIT,(a0)
    addq.w  #2,d4
.wait_mask_3:
    bset.b  #7,(a0)					      	| restart blitter if needed
    nop
    bne.s   .wait_mask_3
|      }
    dbra	d7,.mask_loop_3
|      blitter->LOP = 7|
    move.b	#eBLITTERLOP_SRC_OR_DST,(eBLITTER_BASE+sBLITTER_LOP).w
    asl.w	#2,d1							| (srcIncYBase << 3){d1}
    asl.l	#2,d2							| (srcOffsYBase << 3){d2}
    bra.s	.had_mask_3
|    } else {
.no_mask_3:
|      blitter->LOP = 3|
    lea		(eBLITTER_BASE+sBLITTER_MODE).w,a0			| {a0} fast blitter mode access
    move.b	#eBLITTERLOP_SRC,(eBLITTER_BASE+sBLITTER_LOP).w
    asl.w	#3,d1							| (srcIncYBase << 3){d1}
    asl.l	#3,d2							| (srcOffsYBase << 3){d2}
|    }
.had_mask_3:
|    blitter->SRC_INC_X = 8|
    move.w	#8,(eBLITTER_BASE+sBLITTER_SRC_INC_X).w
|    blitter->SRC_INC_Y = (srcIncYBase << 3){d1}|
    move.w	d1,(eBLITTER_BASE+sBLITTER_SRC_INC_Y).w
|    for (int bp = 0| bp < 4| bp++) {
    moveq.l	#3,d7
    move.l	s_cgimage_bitmap(a1),a1	| srcImage->bitmap{a1}
    adda.l	d2,a1						| (srcImage->bitmap + (srcOffsYBase << 3){d2}){a1}
.bitmap_loop_3:
|      blitter->pSRC = (srcImage->bitmap + (srcOffsYBase << 3) + (bp << 1)){a1}|
    move.l	a1,(eBLITTER_BASE+sBLITTER_pSRC).w
|      blitter->pDST = (pDST + (bp << 1)){a2}|
    move.l	a2,(eBLITTER_BASE+sBLITTER_pDST).w
|      blitter->COUNT_Y = countY{d0}|
    move.w	d0,(eBLITTER_BASE+sBLITTER_COUNT_Y).w
|        blitter->MODE = 0x80|
    move.b  #eBLITTERMODE_BUSY_BIT,(a0)
    addq.l	#2,a2
    addq.l	#2,a1
.wait_bitmap_3:
    bset.b  #7,(a0)					      	| restart blitter if needed
    nop
    bne.s   .wait_bitmap_3
|    }
    dbra	d7,.bitmap_loop_3

    movem.l	(a7)+,d3-d7/a2-a3
    rts


**************************************************************************************
	.data
**************************************************************************************


*
*          T h E   s E t T i N g   O f   S k E w    F l A g S
*
*
* QUALIFIERS   ACTIONS           BITBLT DIRECTION: LEFT -> RIGHT
*
* equal Sx&F>
* spans Dx&F FXSR NFSR
* 0     0     0    1 |..ssssssssssssss|ssssssssssssss..|
*                    |......dddddddddd|dddddddddddddddd|dd..............|
*
* 0     1     1    0 |......ssssssssss|ssssssssssssssss|ss..............|
*                    |..dddddddddddddd|dddddddddddddd..|
*
* 1     0     0    0 |..ssssssssssssss|ssssssssssssss..|
*                    |...ddddddddddddd|ddddddddddddddd.|
*
* 1     1     1    1 |...sssssssssssss|sssssssssssssss.|
*                    |..dddddddddddddd|dddddddddddddd..|
    
    
gGraphic_4BP_SkewFlags:
    dc.b    eBLITTERSKEW_NFSR_BIT           | Source span < Destination span
    dc.b    eBLITTERSKEW_FXSR_BIT           | Source span > Destination span
    dc.b    0                               | Spans equal Shift Source right
    dc.b    eBLITTERSKEW_NFSR_FXSR_BIT      | Spans equal Shift Source left
* When Destination span is but a single word ...
    dc.b    0                               | Implies a Source span of no words
    dc.b    eBLITTERSKEW_FXSR_BIT           | Source span of two words
    dc.b    0                               | Skew flags aren't set if Source and
    dc.b    0                               | Destination spans are both one word

| +2 and NOT for right mask
gGraphic_EndMasks:
    dc.w	0xFFFF
    dc.w	0x7FFF
    dc.w	0x3FFF
    dc.w	0x1FFF
    dc.w	0x0FFF
    dc.w	0x07FF
    dc.w	0x03FF
    dc.w	0x01FF
    dc.w	0x00FF
    dc.w	0x007F
    dc.w	0x003F
    dc.w	0x001F
    dc.w	0x000F
    dc.w	0x0007
    dc.w	0x0003
    dc.w	0x0001
    dc.w	0x0000


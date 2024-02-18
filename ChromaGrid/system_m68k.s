|
| Primitive Interupt handlers.
|
    .extern _pVBLFunc
    .extern _pVBLTick
    .global _pVBLInterupt
    .global _pSystemVBLInterupt

    .extern _pMouseButtons
    .extern _pMousePosition
    .global _pMouseInterupt
    .global _pSystemMouseInterupt

    .text

    .even
_pVBLInterupt:
    move.l  d0,-(sp)
    add.l   #1,_pVBLTick
    move.l  _pVBLFunc, d0
    beq.s   .L1
    movem.l d1-d2/a0-a1,-(sp)
    move.l  d0,a0
    jsr     (a0)
    movem.l (sp)+,d1-d2/a0-a1
.L1:
    move.l  (sp)+,d0
    .dc.w    0x4ef9         | jmp $xxxxxxxx.l
_pSystemVBLInterupt:
    .dc.l    0x0

    .even
_pMouseInterupt:
    move.w  d0, -(sp)
    move.b  (a0),d0
    and.b   #0x3,d0
    move.b  d0,_pMouseButtons
    move.b  1(a0),d0
    ext.w   d0
    add.w   d0,_pMousePosition
    move.b  2(a0),d0
    ext.w   d0
    add.w   d0,_pMousePosition+2
    move.w  (sp)+,d0
    .dc.w    0x4ef9         | jmp $xxxxxxxx.l
_pSystemMouseInterupt:
    .dc.l    0x0

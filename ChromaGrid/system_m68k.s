|
| Primitive Interupt handlers.
|
    .extern _pVBLFunc
    .extern _pVBLTick
    .global _pVBLInterupt
    .global _pSystemVBLInterupt

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

|
| Primitive Interupt handlers.
|
    .extern _cgg_vbl_functions
    .extern _cgg_vbl_tick
    .global _cgg_vbl_interupt
    .global _pSystemVBLInterupt

    .extern _cgg_mouse_buttons
    .extern _cgg_mouse_position
    .global _pMouseInterupt
    .global _pSystemMouseInterupt

    .text

    .even
_cgg_vbl_interupt:
    movem.l d0-d2/a0-a2,-(sp)
    add.l   #1,_cgg_vbl_tick
    lea     _cgg_vbl_functions, a2
.L1:
    move.l  (a2)+,d0
    beq.s   .L2
    move.l  d0,a0
    jsr     (a0)
    bra.s   .L1
.L2:
    movem.l (sp)+,d0-d2/a0-a2
    .dc.w    0x4ef9         | jmp $xxxxxxxx.l
_pSystemVBLInterupt:
    .dc.l    0x0

    .even
_pMouseInterupt:
    move.w  d0, -(sp)
    move.b  (a0),d0
    and.b   #0x3,d0
    move.b  d0,_cgg_mouse_buttons
    move.b  1(a0),d0
    ext.w   d0
    add.w   d0,_cgg_mouse_position
    move.b  2(a0),d0
    ext.w   d0
    add.w   d0,_cgg_mouse_position+2
    move.w  (sp)+,d0
    .dc.w    0x4ef9         | jmp $xxxxxxxx.l
_pSystemMouseInterupt:
    .dc.l    0x0

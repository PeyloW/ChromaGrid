|
| Primitive Interupt handlers.
|
    .extern _cgg_vbl_functions
    .extern _cgg_vbl_tick
    .extern _cgg_timer_c_functions
    .extern _cgg_timer_c_tick
    .extern _cgg_active_image
    .global _cgg_vbl_interupt
    .global _cgg_system_vbl_interupt
    .global _cgg_timer_c_interupt
    .global _cgg_system_timer_c_interupt

    .extern _cgg_mouse_buttons
    .extern _cgg_mouse_position
    .global _cgg_mouse_interupt
    .global _cgg_system_mouse_interupt

    .global _cgg_microwire_write

    .struct
cgtimer_func_freq:      ds.b    1
cgtimer_func_cnt:       ds.b    1
cgtimer_func_func:      ds.l    4

    .struct
cgimage_super_image:            ds.l    1
cgimage_palette:                ds.l    1
cgimage_bitmap:                  ds.l    1
cgimage_maskmap:                 ds.l    1
cgimage_dirtymap:                ds.l    1
cgimage_stencil:                 ds.l    1
cgimage_size:                    ds.w    2
cgimage_offset:                  ds.w    2
cgimage_line_words:              ds.w    1
cgimage_owns_bitmap:             ds.b    1
cgimage_clipping:                ds.b    1


    .text

    .even
_cgg_vbl_interupt:
    movem.l d0-d2/a0-a2,-(sp)
    move.l  _cgg_active_image,d0
    beq.s   .no_active_image
    move.l  d0,a0
    move.l  cgimage_bitmap(a0),d0   ||| MUST MATCH cgimage_c::_bitmap offset!!!
    move.b  d0,d1
    lsr.w #8,d0
    move.l  d0,0xffff8204.w
    move.b  d1,0xffff8209.w
.no_active_image:
    add.l   #1,_cgg_vbl_tick
    lea     _cgg_vbl_functions, a2
.next_func_v:
    move.b  (a2),d0
    beq.s   .no_more_funcs_v
    sub.b   d0,1(a2)
    bcc.s   .no_call_v
    add.b   #50,1(a2)
    move.l  2(a2),a0
    jsr     (a0)
.no_call_v:
    add.l   #6,a2
    bra.s   .next_func_v
.no_more_funcs_v:
    movem.l (sp)+,d0-d2/a0-a2
    .dc.w    0x4ef9         | jmp $xxxxxxxx.l
_cgg_system_vbl_interupt:
    .dc.l    0x0

.even
_cgg_timer_c_interupt:
    movem.l d0-d2/a0-a2,-(sp)
    add.l   #1,_cgg_timer_c_tick
    lea     _cgg_timer_c_functions,a2
.next_func_c:
    move.b  (a2),d0
    beq.s   .no_more_funcs_c
    sub.b   d0,1(a2)
    bcc.s   .no_call_c
    add.b   #200,1(a2)
    move.l  2(a2),a0
    jsr     (a0)
.no_call_c:
    add.l   #6,a2
    bra.s   .next_func_c
.no_more_funcs_c:
    movem.l (sp)+,d0-d2/a0-a2
    .dc.w    0x4ef9         | jmp $xxxxxxxx.l
_cgg_system_timer_c_interupt:
    .dc.l    0x0

    .even
_cgg_mouse_interupt:
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
_cgg_system_mouse_interupt:
    .dc.l    0x0

_cgg_microwire_write:
    move.w  #0x07ff,0xffff8924.w
    move.w  d0,0xffff8922.w
.wait:
    cmp.w   #0x07ff,0xffff8924.w
    bne.s   .wait
    move.w  d0,0xffff8922.w
    rts

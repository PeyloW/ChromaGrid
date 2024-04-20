#NO_APP
	.text
	.even
	.globl	__ZN6toybox13audio_mixer_c4playERKNS_7sound_cEh
__ZN6toybox13audio_mixer_c4playERKNS_7sound_cEh:
	move.w #-30463,%a0
	and.b #-2,(%a0)
	tst.l 2(%a1)
	sne %d0
	ext.w %d0
	ext.l %d0
	neg.l %d0
	clr.b -30461.w
	clr.b -30459.w
	move.b %d0,-30457.w
	add.l 6(%a1),%d0
	move.l %d0,%d1
	clr.w %d1
	swap %d1
	move.b %d1,-30449.w
	move.l %d0,%d1
	lsr.l #8,%d1
	move.b %d1,-30447.w
	move.b %d0,-30445.w
	move.b #-127,-30431.w
	move.b #1,(%a0)
	rts
	.even
	.globl	__ZN6toybox13audio_mixer_c4stopERKNS_7sound_cE
__ZN6toybox13audio_mixer_c4stopERKNS_7sound_cE:
	rts
	.even
	.globl	__ZN6toybox13audio_mixer_c4stopERKNS_7music_cE
__ZN6toybox13audio_mixer_c4stopERKNS_7music_cE:
	subq.l #4,%sp
	move.l %a4,-(%sp)
	move.l %a3,-(%sp)
	move.l %a0,%a3
	move.l %a1,%a4
#APP
| 119 "toybox/system.hpp" 1
	move.w #0x2700,sr
| 0 "" 2
#NO_APP
	moveq #1,%d0
	lea (10,%sp),%a0
	jsr __ZN6toybox7timer_cC1ENS0_7timer_eE
	lea (34,%a4),%a0
	jsr (%a0)
	lea (50,%a4),%a1
	lea (10,%sp),%a0
	jsr __ZN6toybox7timer_c11remove_funcEPFvvE
	lea (10,%sp),%a0
	jsr __ZN6toybox7timer_cD1Ev
#APP
| 121 "toybox/system.hpp" 1
	move.w #0x2300,sr
| 0 "" 2
#NO_APP
	clr.l (%a3)
	clr.w 4(%a3)
	move.l (%sp)+,%a3
	move.l (%sp)+,%a4
	addq.l #4,%sp
	rts
	.even
	.globl	__ZN6toybox13audio_mixer_c4playERKNS_7music_cEi
__ZN6toybox13audio_mixer_c4playERKNS_7music_cEi:
	subq.l #4,%sp
	movem.l #4120,-(%sp)
	move.l %a0,%a4
	move.l %a1,%a3
	move.w %d0,%d3
	move.l (%a0),%a1
	cmp.w #0,%a1
	jeq .L5
	jsr __ZN6toybox13audio_mixer_c4stopERKNS_7music_cE
.L5:
#APP
| 119 "toybox/system.hpp" 1
	move.w #0x2700,sr
| 0 "" 2
#NO_APP
	moveq #1,%d0
	lea (14,%sp),%a0
	jsr __ZN6toybox7timer_cC1ENS0_7timer_eE
	lea (18,%a3),%a0
	move.w %d3,%d0
	jsr (%a0)
	move.b 16(%a3),%d0
	lea (50,%a3),%a1
	lea (14,%sp),%a0
	jsr __ZN6toybox7timer_c8add_funcEPFvvEh
	lea (14,%sp),%a0
	jsr __ZN6toybox7timer_cD1Ev
#APP
| 121 "toybox/system.hpp" 1
	move.w #0x2300,sr
| 0 "" 2
#NO_APP
	move.l %a3,(%a4)
	move.w %d3,4(%a4)
	movem.l (%sp)+,#6152
	addq.l #4,%sp
	rts
	.even
	.globl	__ZN6toybox13audio_mixer_c8stop_allEv
__ZN6toybox13audio_mixer_c8stop_allEv:
	move.l (%a0),%a1
	cmp.w #0,%a1
	jeq .L6
	jra __ZN6toybox13audio_mixer_c4stopERKNS_7music_cE
.L6:
	rts
	.even
___tcf_0:
	lea __ZZN6toybox13audio_mixer_c6sharedEvE7s_mixer,%a0
	jra __ZN6toybox13audio_mixer_c8stop_allEv
	.even
	.globl	__ZN6toybox13audio_mixer_cC2Ev
__ZN6toybox13audio_mixer_cC2Ev:
	move.l %a3,-(%sp)
	clr.l (%a0)
	clr.w 4(%a0)
	lea _g_microwire_write,%a3
	moveq #108,%d0
	jsr (%a3)
	moveq #84,%d0
	jsr (%a3)
	moveq #84,%d0
	move.l (%sp)+,%a3
	jra _g_microwire_write
	.even
	.globl	__ZN6toybox13audio_mixer_c6sharedEv
__ZN6toybox13audio_mixer_c6sharedEv:
	tst.b __ZGVZN6toybox13audio_mixer_c6sharedEvE7s_mixer
	jne .L12
	lea __ZGVZN6toybox13audio_mixer_c6sharedEvE7s_mixer,%a0
	jsr ___cxa_guard_acquire
	tst.w %d0
	jeq .L12
	lea __ZZN6toybox13audio_mixer_c6sharedEvE7s_mixer,%a0
	jsr __ZN6toybox13audio_mixer_cC1Ev
	lea __ZGVZN6toybox13audio_mixer_c6sharedEvE7s_mixer,%a0
	jsr ___cxa_guard_release
	lea ___tcf_0,%a0
	jsr _atexit
.L12:
	lea __ZZN6toybox13audio_mixer_c6sharedEvE7s_mixer,%a0
	rts
	.even
	.globl	__ZN6toybox13audio_mixer_cD2Ev
__ZN6toybox13audio_mixer_cD2Ev:
	jra __ZN6toybox13audio_mixer_c8stop_allEv
.lcomm __ZGVZN6toybox13audio_mixer_c6sharedEvE7s_mixer,8
.lcomm __ZZN6toybox13audio_mixer_c6sharedEvE7s_mixer,6
	.globl	__ZN6toybox13audio_mixer_cC1Ev
	.set	__ZN6toybox13audio_mixer_cC1Ev,__ZN6toybox13audio_mixer_cC2Ev
	.globl	__ZN6toybox13audio_mixer_cD1Ev
	.set	__ZN6toybox13audio_mixer_cD1Ev,__ZN6toybox13audio_mixer_cD2Ev

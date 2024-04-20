#NO_APP
	.text
	.even
	.globl	__Znwm
__Znwm:
	jra _malloc
	.even
	.globl	__Znam
__Znam:
	jra _malloc
	.even
	.globl	__ZdlPv
__ZdlPv:
	jra _free
	.even
	.globl	__ZdaPv
__ZdaPv:
	jra _free
	.even
	.globl	___cxa_pure_virtual
___cxa_pure_virtual:
.L6:
	jra .L6
	.even
	.globl	__ZnwmPv
__ZnwmPv:
	rts
	.even
	.globl	__ZnamPv
__ZnamPv:
	rts

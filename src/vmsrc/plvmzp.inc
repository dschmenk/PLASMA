;**********************************************************
;*
;* VM ZERO PAGE LOCATIONS
;*
;**********************************************************
SRC	=	$06
SRCL	=	SRC
SRCH	=	SRC+1
DST	=	SRC+2
DSTL	=	DST
DSTH	=	DST+1
ESTKSZ	=	$20
ESTK	=	$C0
ESTKL	=	ESTK
ESTKH	=	ESTK+ESTKSZ/2
VMZP	=	ESTK+ESTKSZ
IFP	=	VMZP
IFPL	=	IFP
IFPH	=	IFP+1
;IP     =	IFP+2		; MOVED TO OVERLAY NEXTOP
;IPL	=	IP
;IPH	=	IP+1
;IPY	=	IP+2
IPY	=	IFP+2
TMP	=	IPY+1
TMPL	=	TMP
TMPH	=	TMP+1
NPARMS	=	TMPL
FRMSZ	=	TMPH
DVSIGN	=	TMP+2
ESP     =       TMP+2
DROP	=	$EF
NEXTOP	=	$F0
FETCHOP	=	NEXTOP+3
IP	=	FETCHOP+1
IPL	=	IP
IPH	=	IPL+1
OPIDX	=	FETCHOP+6
OPPAGE	=	OPIDX+1

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
ESP     =       VMZP
DVSIGN	=	VMZP
IFP	=	ESP+1
IFPL	=	IFP
IFPH	=	IFP+1
PP      =	IFP+2
PPL	=	PP
PPH	=	PP+1
IPY	=	PP+2
TMP	=	IPY+1
TMPL	=	TMP
TMPH	=	TMP+1
NPARMS	=	TMPL
FRMSZ	=	TMPH
DROP	=	$EF
NEXTOP	=	$F0
FETCHOP	=	NEXTOP+3
IP	=	FETCHOP+1
IPL	=	IP
IPH	=	IPL+1
OPIDX	=	FETCHOP+6
OPPAGE	=	OPIDX+1

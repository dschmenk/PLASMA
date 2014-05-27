;**********************************************************
;*
;* VM ZERO PAGE LOCATIONS
;*
;**********************************************************
ESTKSZ	=	$20
ESTK	=	$C0
ESTKL	=	ESTK
ESTKH	=	ESTK+ESTKSZ/2
VMZP	=	ESTK+ESTKSZ
IFP	=	VMZP
IFPL	=	IFP
IFPH	=	IFP+1
IP     	=	IFP+2
IPL	=	IP
IPH	=	IP+1
IPY	=	IP+2
TMP	=	IP+3
TMPL	=	TMP
TMPH	=	TMP+1
TMPX	=	TMP+2
NPARMS	=	TMPL
FRMSZ	=	TMPH
DVSIGN	=	TMPX
ESP     =       TMPX
TICTOC	=	TMP+3
SRC	=	$06
SRCL	=	SRC
SRCH	=	SRC+1
DST	=	SRC+2
DSTL	=	DST
DSTH	=	DST+1

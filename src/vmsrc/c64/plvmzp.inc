;**********************************************************
;*
;*                 VM ZERO PAGE LOCATIONS
;*
;**********************************************************
SRC     =   $4C
SRCL    =   SRC
SRCH    =   SRC+1
DST     =   SRC+2
DSTL    =   DST
DSTH    =   DST+1
ESGUARD =   $4E
ESTKSZ  =   $20
ESTK    =   $50
ESTKH   =   ESTK
ESTKL   =   ESTK+ESTKSZ/2
VMZP    =   ESTK+ESTKSZ
IFP     =   VMZP
IFPL    =   IFP
IFPH    =   IFP+1
PP      =   IFP+2
PPL     =   PP
PPH     =   PP+1
IPY     =   PP+2
ESP     =   IPY+1
JMPTMP  =   ESP+1
TMP     =   JMPTMP+1
TMPL    =   TMP
TMPH    =   TMP+1

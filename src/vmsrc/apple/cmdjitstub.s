INTERP  =       $03D0
LCRDEN  =       $C080
LCWTEN  =       $C081
ROMEN   =       $C082
LCRWEN  =       $C083
LCBNK2  =       $00
LCBNK1  =       $08
JITCOMP =       $03E2
JITCODE =       $03E4
!SOURCE "vmsrc/plvmzp.inc"
;*
;* MOVE CMD DOWN TO $1000-$2000
;*
        LDA     #<_CMDBEGIN
        STA     SRCL
        LDA     #>_CMDBEGIN
        STA     SRCH
        LDY     #$00
        STY     DSTL
        LDX     #$10
        STX     DSTH
-       LDA     (SRC),Y
        STA     (DST),Y
        INY
        BNE     -
        INC     SRCH
        INC     DSTH
        DEX                 ; STOP WHEN DST=$2000 REACHED
        BNE     -
;
; INIT VM ENVIRONMENT STACK POINTERS
;
        STY     $01FF
        STY     PPL
        STY     IFPL        ; INIT FRAME POINTER = $AF00 (4K FOR JIT CODE)
        STY     JITCODE
        STY     JITCOMP
        STY     JITCOMP+1
        LDA     #$AF
        STA     PPH
        STA     IFPH
        STA     JITCODE+1
        LDX     #$FE        ; INIT STACK POINTER (YES, $FE. SEE GETS)
        TXS
        LDX     #ESTKSZ/2   ; INIT EVAL STACK INDEX
        JMP     $1000
_CMDBEGIN   =   *
!PSEUDOPC   $1000 {
!SOURCE "vmsrc/apple/cmdjit.a"
_CMDEND     =   *
}

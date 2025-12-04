INTERP  =       $03D0
LCRDEN  =       $C080
LCWTEN  =       $C081
ROMEN   =       $C082
LCRWEN  =       $C083
LCBNK2  =       $00
LCBNK1  =       $08
    !SOURCE "vmsrc/plvmzp.inc"
        JMP     CMDMOVE
_CMDBEGIN   =   *
!PSEUDOPC   $0C00 {
!SOURCE "vmsrc/apple/cmd.a"
_CMDEND     =   *
}
;*
;* MOVE CMD DOWN TO $0C00-$1C00
;*
CMDMOVE LDA     #<_CMDBEGIN
        STA     SRCL
        LDA     #>_CMDBEGIN
        STA     SRCH
        LDY     #$00
        STY     DSTL
        LDA     #$0C
        STA     DSTH
        LDX     #$10
-       LDA     (SRC),Y
        STA     (DST),Y
        INY
        BNE     -
        INC     SRCH
        INC     DSTH
        DEX                 ; STOP WHEN DST=$2000 REACHED
        BPL     -
;
; INIT VM ENVIRONMENT STACK POINTERS
;
        STY     $01FF
        STY     IFPL        ; INIT FRAME POINTER = $BF00
        LDA     #$BF
        STA     IFPH
        LDX     #$FE        ; INIT STACK POINTER (YES, $FE. SEE GETS)
        TXS
        LDX     #ESTKSZ/2   ; INIT EVAL STACK INDEX
        JMP     $0C00

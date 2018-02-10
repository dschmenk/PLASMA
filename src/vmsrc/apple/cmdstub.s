INTERP  =   $03D0
LCRDEN  =   $C080
LCWTEN  =   $C081
ROMEN       =   $C082
LCRWEN  =   $C083
LCBNK2  =   $00
LCBNK1  =   $08
    !SOURCE "vmsrc/plvmzp.inc"
;*
;* MOVE CMD DOWN TO $1000-$2000
;*
    LDA #<_CMDBEGIN
    STA SRCL
    LDA #>_CMDBEGIN
    STA SRCH
    LDY #$00
    STY DSTL
    LDX #$10
    STX DSTH
-   LDA (SRC),Y
    STA (DST),Y
    INY
    BNE -
    INC SRCH
    INC DSTH
    DEX             ; STOP WHEN DST=$2000 REACHED
    BNE -
    LDA #<_CMDEND
    STA SRCL
    LDA #>_CMDEND
    STA SRCH
;
; INIT VM ENVIRONMENT STACK POINTERS
;
    STY PPL
    STY IFPL        ; INIT FRAME POINTER
    LDA #$BF
    STA PPH
    STA IFPH
    LDX #$FE        ; INIT STACK POINTER (YES, $FE. SEE GETS)
    TXS
    LDX #ESTKSZ/2   ; INIT EVAL STACK INDEX
    JMP $1000
_CMDBEGIN = *
    !PSEUDOPC   $1000 {
    !SOURCE "vmsrc/apple/cmd.a"
_CMDEND =   *
}

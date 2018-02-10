;**********************************************************
;*
;*             BBC B PLASMA INTERPETER
;*
;*             VM INITIALIZATION
;*
;**********************************************************
BRKHND  LDY     #0
        LDA     ($FD),Y
        STA     ERRNUM
ERRCPY  INY
        LDA     ($FD),Y
        BEQ     ERRCPD
        STA     ERRSTR,Y
        BNE     ERRCPY
ERRCPD  DEY
        STY     ERRSTR
        ;* ERRNUM now holds the error number
        ;* ERRSTR now holds the error message as a standard PLASMA string
        LDX     #$FF
        TXS
        LDX     #ESTKSZ/2       ; INIT EVAL STACK INDEX
        JSR     BRKJMP
        ;* TODO: Better "abort" behaviour
        LDA     #'!'
        JSR     $FFEE
BRKLP   JMP     BRKLP
BRKJMP  JMP ($400) ;* TODO: Better address
        LDA     #65
        JSR     $FFEE
        JMP     BRKHND
VMINIT  LDY     #$10        ; INSTALL PAGE 0 FETCHOP ROUTINE
-       LDA     PAGE0-1,Y
        STA     DROP-1,Y
        DEY
        BNE     -
        LDA     #$4C        ; SET JMPTMP OPCODE
        STA     JMPTMP
        STY     IFPL        ; INIT FRAME POINTER
        LDA     #$80
        STA     IFPH
        LDA     #<SEGEND    ; SAVE HEAP START
        STA     SRCL
        LDA     #>SEGEND
        STA     SRCH
        LDX     #ESTKSZ/2   ; INIT EVAL STACK INDEX
        LDA     #<BRKHND    ; Install BRK Handler
        STA     $0202
        LDA     #>BRKHND
        STA     $0203
        JMP     CMD
PAGE0   =       *
        !PSEUDOPC   DROP {
;*
;* INTERP BYTECODE INNER LOOP
;*
        INX                 ; DROP
        INY                 ; NEXTOP
        LDA     $FFFF,Y     ; FETCHOP @ $F3, IP MAPS OVER $FFFF @ $F4
        STA     OPIDX
        JMP     (OPTBL)
}

;**********************************************************
;*
;*            APPLE1+CFFA1 PLASMA INTERPETER
;*
;*             SYSTEM ROUTINES AND LOCATIONS
;*
;**********************************************************
SELFMODIFY  =   1
;*
;* VM ZERO PAGE LOCATIONS
;*
        !SOURCE "vmsrc/plvmzp.inc"
DVSIGN  =   TMP+2
DROP    =   $EF
NEXTOP  =   $F0
FETCHOP =   NEXTOP+1
IP      =   FETCHOP+1
IPL     =   IP
IPH     =   IPL+1
OPIDX   =   FETCHOP+6
OPPAGE  =   OPIDX+1
;*
;* INTERPRETER INSTRUCTION POINTER INCREMENT MACRO
;*
        !MACRO  INC_IP  {
        INY
        BPL     +
        INC    IPH
        TYA
        AND     #$7F
        TAY
+
        }
;*
;* INTERPRETER HEADER+INITIALIZATION
;*
        *=      $0280
SEGBEGIN JMP    VMINIT
;*
;* SYSTEM INTERPRETER ENTRYPOINT
;*
INTERP  PLA
        CLC
        ADC     #$01
        STA     IPL
        PLA
        ADC     #$00
        STA     IPH
        LDY     #$00
        JMP     FETCHOP
;*
;* ENTER INTO USER BYTECODE INTERPRETER
;*
IINTERP PLA
        STA     TMPL
        PLA
        STA     TMPH
        LDY     #$02
        LDA     (TMP),Y
        STA     IPH
        DEY
        LDA     (TMP),Y
        STA     IPL
        DEY
        JMP     FETCHOP
;*
;* MUL TOS-1 BY TOS
;*
MUL     STY     IPY
        LDY     #$10
        LDA     ESTKL+1,X
        EOR     #$FF
        STA     TMPL
        LDA     ESTKH+1,X
        EOR     #$FF
        STA     TMPH
        LDA     #$00
        STA     ESTKL+1,X       ; PRODL
;       STA     ESTKH+1,X       ; PRODH
MULLP   LSR     TMPH            ; MULTPLRH
        ROR     TMPL            ; MULTPLRL
        BCS     +
        STA     ESTKH+1,X       ; PRODH
        LDA     ESTKL,X         ; MULTPLNDL
        ADC     ESTKL+1,X       ; PRODL
        STA     ESTKL+1,X
        LDA     ESTKH,X         ; MULTPLNDH
        ADC     ESTKH+1,X       ; PRODH
+       ASL     ESTKL,X         ; MULTPLNDL
        ROL     ESTKH,X         ; MULTPLNDH
        DEY
        BNE     MULLP
        STA     ESTKH+1,X       ; PRODH
        LDY     IPY
    JMP DROP
;*
;* INCREMENT TOS
;*
INCR    INC     ESTKL,X
        BNE     INCR1
        INC     ESTKH,X
INCR1   JMP     NEXTOP
;*
;* DECREMENT TOS
;*
DECR    LDA     ESTKL,X
        BNE     DECR1
        DEC     ESTKH,X
DECR1   DEC     ESTKL,X
        JMP     NEXTOP
;*
;* BITWISE COMPLIMENT TOS
;*
COMP    LDA     #$FF
        EOR     ESTKL,X
        STA     ESTKL,X
        LDA     #$FF
        EOR     ESTKH,X
        STA     ESTKH,X
        JMP     NEXTOP
;*
;* OPCODE TABLE
;*
    !ALIGN  255,0
OPTBL   !WORD   ZERO,ADD,SUB,MUL,DIV,MOD,INCR,DECR          ; 00 02 04 06 08 0A 0C 0E
        !WORD   NEG,COMP,BAND,IOR,XOR,SHL,SHR,IDXW          ; 10 12 14 16 18 1A 1C 1E
        !WORD   LNOT,LOR,LAND,LA,LLA,CB,CW,CS               ; 20 22 24 26 28 2A 2C 2E
        !WORD   DROP,DUP,NEXTOP,DIVMOD,BRGT,BRLT,BREQ,BRNE  ; 30 32 34 36 38 3A 3C 3E
        !WORD   ISEQ,ISNE,ISGT,ISLT,ISGE,ISLE,BRFLS,BRTRU   ; 40 42 44 46 48 4A 4C 4E
        !WORD   BRNCH,IBRNCH,CALL,ICAL,ENTER,LEAVE,RET,CFFB ; 50 52 54 56 58 5A 5C 5E
        !WORD   LB,LW,LLB,LLW,LAB,LAW,DLB,DLW               ; 60 62 64 66 68 6A 6C 6E
        !WORD   SB,SW,SLB,SLW,SAB,SAW,DAB,DAW               ; 70 72 74 76 78 7A 7C 7E
;*
;* DIV TOS-1 BY TOS
;*
DIV     JSR     _DIV
        LSR     DVSIGN          ; SIGN(RESULT) = (SIGN(DIVIDEND) + SIGN(DIVISOR)) & 1
        BCS     NEG
        JMP     NEXTOP
;*
;* MOD TOS-1 BY TOS
;*
MOD     JSR     _DIV
        LDA     TMPL            ; REMNDRL
        STA     ESTKL,X
        LDA     TMPH            ; REMNDRH
        STA     ESTKH,X
        LDA     DVSIGN          ; REMAINDER IS SIGN OF DIVIDEND
        BMI     NEG
        JMP     NEXTOP
;*
;* DIVMOD TOS-1 BY TOS
;*
DIVMOD  JSR     _DIV
        LSR     DVSIGN           ; SIGN(RESULT) = (SIGN(DIVIDEND) + SIGN(DIVISOR)) & 1
        BCC     +
        INX
        JSR     _NEG
        DEX
+       LDA     TMPL            ; REMNDRL
        STA     ESTKL,X
        LDA     TMPH            ; REMNDRH
        STA     ESTKH,X
        LDA     DVSIGN          ; REMAINDER IS SIGN OF DIVIDEND
        BMI     NEG
        JMP     NEXTOP
;*
;* NEGATE TOS
;*
NEG     JSR     _NEG
        JMP     NEXTOP
;*
;* INTERNAL DIVIDE ALGORITHM
;*
_NEG    LDA     #$00
        SEC
        SBC     ESTKL,X
        STA     ESTKL,X
        LDA     #$00
        SBC     ESTKH,X
        STA     ESTKH,X
        RTS
_DIV    STY     IPY
        LDY     #$11            ; #BITS+1
        LDA     #$00
        STA     TMPL            ; REMNDRL
        STA     TMPH            ; REMNDRH
        LDA     ESTKH,X
        AND     #$80
        STA     DVSIGN
        BPL     +
        JSR     _NEG
        INC     DVSIGN
+       LDA     ESTKH+1,X
        BPL     +
        INX
        JSR     _NEG
        DEX
        INC     DVSIGN
        BNE     _DIV1
+       ORA     ESTKL+1,X       ; DVDNDL
        BEQ     _DIVEX
_DIV1   ASL     ESTKL+1,X       ; DVDNDL
        ROL     ESTKH+1,X       ; DVDNDH
        DEY
        BCC     _DIV1
_DIVLP  ROL     TMPL            ; REMNDRL
        ROL     TMPH            ; REMNDRH
        LDA     TMPL            ; REMNDRL
        CMP     ESTKL,X         ; DVSRL
        LDA     TMPH            ; REMNDRH
        SBC     ESTKH,X         ; DVSRH
        BCC     +
        STA     TMPH            ; REMNDRH
        LDA     TMPL            ; REMNDRL
        SBC     ESTKL,X         ; DVSRL
        STA     TMPL            ; REMNDRL
        SEC
+       ROL     ESTKL+1,X       ; DVDNDL
        ROL     ESTKH+1,X       ; DVDNDH
        DEY
        BNE     _DIVLP
_DIVEX  INX
        LDY     IPY
        RTS
;*
;* ADD TOS TO TOS-1
;*
ADD     LDA     ESTKL,X
        CLC
        ADC     ESTKL+1,X
        STA     ESTKL+1,X
        LDA     ESTKH,X
        ADC     ESTKH+1,X
        STA     ESTKH+1,X
        JMP     DROP
;*
;* SUB TOS FROM TOS-1
;*
SUB     LDA     ESTKL+1,X
        SEC
        SBC     ESTKL,X
        STA     ESTKL+1,X
        LDA     ESTKH+1,X
        SBC     ESTKH,X
        STA     ESTKH+1,X
        JMP     DROP
;
;*
;* SHIFT TOS LEFT BY 1, ADD TO TOS-1
;*
IDXW    LDA     ESTKL,X
        ASL
        ROL     ESTKH,X
        CLC
        ADC     ESTKL+1,X
        STA     ESTKL+1,X
        LDA     ESTKH,X
        ADC     ESTKH+1,X
        STA     ESTKH+1,X
        JMP     DROP
;*
;* BITWISE AND TOS TO TOS-1
;*
BAND    LDA     ESTKL+1,X
        AND     ESTKL,X
        STA     ESTKL+1,X
        LDA     ESTKH+1,X
        AND     ESTKH,X
        STA     ESTKH+1,X
        JMP     DROP
;*
;* INCLUSIVE OR TOS TO TOS-1
;*
IOR     LDA     ESTKL+1,X
        ORA     ESTKL,X
        STA     ESTKL+1,X
        LDA     ESTKH+1,X
        ORA     ESTKH,X
        STA     ESTKH+1,X
        JMP     DROP
;*
;* EXLUSIVE OR TOS TO TOS-1
;*
XOR     LDA     ESTKL+1,X
        EOR     ESTKL,X
        STA     ESTKL+1,X
        LDA     ESTKH+1,X
        EOR     ESTKH,X
        STA     ESTKH+1,X
        JMP     DROP
;*
;* SHIFT TOS-1 LEFT BY TOS
;*
SHL STY IPY
        LDA     ESTKL,X
        CMP     #$08
        BCC     SHL1
        LDY     ESTKL+1,X
        STY     ESTKH+1,X
        LDY     #$00
        STY     ESTKL+1,X
        SBC     #$08
SHL1    TAY
        BEQ     SHL3
        LDA     ESTKL+1,X
SHL2    ASL
        ROL     ESTKH+1,X
        DEY
        BNE     SHL2
        STA     ESTKL+1,X
SHL3    LDY     IPY
        JMP     DROP
;*
;* SHIFT TOS-1 RIGHT BY TOS
;*
SHR     STY     IPY
        LDA     ESTKL,X
        CMP     #$08
        BCC     SHR2
        LDY     ESTKH+1,X
        STY     ESTKL+1,X
        CPY     #$80
        LDY     #$00
        BCC     SHR1
        DEY
SHR1    STY     ESTKH+1,X
        SEC
        SBC     #$08
SHR2    TAY
        BEQ     SHR4
        LDA     ESTKH+1,X
SHR3    CMP     #$80
        ROR
        ROR     ESTKL+1,X
        DEY
        BNE     SHR3
        STA     ESTKH+1,X
SHR4    LDY     IPY
        JMP     DROP
;*
;* LOGICAL NOT
;*
LNOT    LDA     ESTKL,X
        ORA     ESTKH,X
        BEQ     LNOT1
        LDA     #$FF
LNOT1   EOR     #$FF
        STA     ESTKL,X
        STA     ESTKH,X
        JMP     NEXTOP
;*
;* LOGICAL AND
;*
LAND    LDA     ESTKL+1,X
        ORA     ESTKH+1,X
        BEQ     LAND2
        LDA     ESTKL,X
        ORA     ESTKH,X
        BEQ     LAND1
        LDA     #$FF
LAND1   STA     ESTKL+1,X
        STA     ESTKH+1,X
LAND2   JMP     DROP
;*
;* LOGICAL OR
;*
LOR     LDA     ESTKL,X
        ORA     ESTKH,X
        ORA     ESTKL+1,X
        ORA     ESTKH+1,X
        BEQ     LOR1
        LDA     #$FF
        STA     ESTKL+1,X
        STA     ESTKH+1,X
LOR1    JMP     DROP
;*
;* DUPLICATE TOS
;*
DUP     DEX
        LDA     ESTKL+1,X
        STA     ESTKL,X
        LDA     ESTKH+1,X
        STA     ESTKH,X
        JMP     NEXTOP
;*
;* CONSTANT
;*
ZERO    DEX
        LDA     #$00
        STA     ESTKL,X
        STA     ESTKH,X
        JMP     NEXTOP
CFFB    LDA     #$FF
        !BYTE   $2C         ; BIT $00A9 - effectively skips LDA #$00, no harm in reading this address
CB      LDA     #$00
        DEX
        STA     ESTKH,X
        +INC_IP
        LDA     (IP),Y
        STA     ESTKL,X
        JMP     NEXTOP
;*
;* LOAD ADDRESS & LOAD CONSTANT WORD (SAME THING, WITH OR WITHOUT FIXUP)
;*
LA      =       *
CW      DEX
        INY     ;+INC_IP
        LDA     (IP),Y
        STA     ESTKL,X
        +INC_IP
        LDA     (IP),Y
        STA     ESTKH,X
        JMP     NEXTOP
;*
;* CONSTANT STRING
;*
CS      DEX
        INY     ;+INC_IP
        TYA                 ; NORMALIZE IP AND SAVE STRING ADDR ON ESTK
        CLC
        ADC     IPL
        STA     IPL
        STA     ESTKL,X
        LDA     #$00
        TAY
        ADC     IPH
        STA     IPH
        STA     ESTKH,X
        LDA     (IP),Y
        TAY
        JMP     NEXTOP
;*
;* LOAD VALUE FROM ADDRESS TAG
;*
!IF     SELFMODIFY {
LB      LDA     ESTKL,X
        STA     LBLDA+1
        LDA     ESTKH,X
        STA     LBLDA+2
LBLDA   LDA     $FFFF
        STA     ESTKL,X
        LDA     #$00
        STA     ESTKH,X
        JMP     NEXTOP
} ELSE {
LB      LDA     ESTKL,X
        STA     TMPL
        LDA     ESTKH,X
        STA     TMPH
        STY     IPY
        LDY     #$00
        LDA     (TMP),Y
        STA     ESTKL,X
        STY     ESTKH,X
        LDY     IPY
        JMP     NEXTOP
}
LW      LDA     ESTKL,X
        STA     TMPL
        LDA     ESTKH,X
        STA     TMPH
        STY     IPY
        LDY     #$00
        LDA     (TMP),Y
        STA     ESTKL,X
        INY
        LDA     (TMP),Y
        STA     ESTKH,X
        LDY     IPY
        JMP     NEXTOP
;*
;* LOAD ADDRESS OF LOCAL FRAME OFFSET
;*
LLA     +INC_IP
        LDA     (IP),Y
        DEX
        CLC
        ADC     IFPL
        STA     ESTKL,X
        LDA     #$00
        ADC     IFPH
        STA     ESTKH,X
        JMP     NEXTOP
;*
;* LOAD VALUE FROM LOCAL FRAME OFFSET
;*
LLB     +INC_IP
        LDA     (IP),Y
        STY     IPY
        TAY
        DEX
        LDA     (IFP),Y
        STA     ESTKL,X
        LDA     #$00
        STA     ESTKH,X
        LDY     IPY
        JMP     NEXTOP
LLW     +INC_IP
        LDA     (IP),Y
        STY     IPY
        TAY
        DEX
        LDA     (IFP),Y
        STA     ESTKL,X
        INY
        LDA     (IFP),Y
        STA     ESTKH,X
        LDY     IPY
        JMP     NEXTOP
;*
;* LOAD VALUE FROM ABSOLUTE ADDRESS
;*
!IF     SELFMODIFY {
LAB     INY     ;+INC_IP
        LDA     (IP),Y
        STA     LABLDA+1
        +INC_IP
        LDA     (IP),Y
        STA     LABLDA+2
LABLDA  LDA     $FFFF
        DEX
        STA     ESTKL,X
        LDA     #$00
        STA     ESTKH,X
        JMP     NEXTOP
} ELSE {
LAB     INY     ;+INC_IP
        LDA     (IP),Y
        STA     TMPL
        +INC_IP
        LDA     (IP),Y
        STA     TMPH
        STY     IPY
        LDY     #$00
        LDA     (TMP),Y
        DEX
        STA     ESTKL,X
        STY     ESTKH,X
        LDY     IPY
        JMP     NEXTOP
}
LAW     INY     ;+INC_IP
        LDA     (IP),Y
        STA     TMPL
        +INC_IP
        LDA     (IP),Y
        STA     TMPH
        STY     IPY
        LDY     #$00
        LDA     (TMP),Y
        DEX
        STA     ESTKL,X
        INY
        LDA     (TMP),Y
        STA     ESTKH,X
        LDY     IPY
        JMP     NEXTOP
;*
;* STORE VALUE TO ADDRESS
;*
!IF     SELFMODIFY {
SB      LDA     ESTKL,X
        STA     SBSTA+1
        LDA     ESTKH,X
        STA     SBSTA+2
        LDA     ESTKL+1,X
SBSTA   STA     $FFFF
        INX
        JMP     DROP
} ELSE {
SB      LDA     ESTKL,X
        STA     TMPL
        LDA     ESTKH,X
        STA     TMPH
        LDA     ESTKL+1,X
        STY     IPY
        LDY     #$00
        STA     (TMP),Y
        LDY     IPY
        INX
        JMP     DROP
}
SW      LDA     ESTKL,X
        STA     TMPL
        LDA     ESTKH,X
        STA     TMPH
        STY     IPY
        LDY     #$00
        LDA     ESTKL+1,X
        STA     (TMP),Y
        INY
        LDA     ESTKH+1,X
        STA     (TMP),Y
        LDY     IPY
        INX
        JMP     DROP
;*
;* STORE VALUE TO LOCAL FRAME OFFSET
;*
SLB     +INC_IP
        LDA     (IP),Y
        STY     IPY
        TAY
        LDA     ESTKL,X
        STA     (IFP),Y
        LDY     IPY
        JMP     DROP
SLW     +INC_IP
        LDA     (IP),Y
        STY     IPY
        TAY
        LDA     ESTKL,X
        STA     (IFP),Y
        INY
        LDA     ESTKH,X
        STA     (IFP),Y
        LDY     IPY
        JMP     DROP
;*
;* STORE VALUE TO LOCAL FRAME OFFSET WITHOUT POPPING STACK
;*
DLB     +INC_IP
        LDA     (IP),Y
        STY     IPY
        TAY
        LDA     ESTKL,X
        STA     (IFP),Y
        LDY     IPY
        JMP     NEXTOP
DLW     +INC_IP
        LDA     (IP),Y
        STY     IPY
        TAY
        LDA     ESTKL,X
        STA     (IFP),Y
        INY
        LDA     ESTKH,X
        STA     (IFP),Y
        LDY     IPY
        JMP     NEXTOP
;*
;* STORE VALUE TO ABSOLUTE ADDRESS
;*
!IF     SELFMODIFY {
SAB     INY     ;+INC_IP
        LDA     (IP),Y
        STA     SABSTA+1
        +INC_IP
        LDA     (IP),Y
        STA     SABSTA+2
        LDA     ESTKL,X
SABSTA  STA     $FFFF
        JMP     DROP
} ELSE {
SAB     INY     ;+INC_IP
        LDA     (IP),Y
        STA     TMPL
        +INC_IP
        LDA     (IP),Y
        STA     TMPH
        LDA     ESTKL,X
        STY     IPY
        LDY     #$00
        STA     (TMP),Y
        LDY     IPY
        JMP     DROP
}
SAW     INY     ;+INC_IP
        LDA     (IP),Y
        STA     TMPL
        +INC_IP
        LDA     (IP),Y
        STA     TMPH
        STY     IPY
        LDY     #$00
        LDA     ESTKL,X
        STA     (TMP),Y
        INY
        LDA     ESTKH,X
        STA     (TMP),Y
        LDY     IPY
        JMP     DROP
;*
;* STORE VALUE TO ABSOLUTE ADDRESS WITHOUT POPPING STACK
;*
!IF     SELFMODIFY {
DAB     INY     ;+INC_IP
        LDA     (IP),Y
        STA     DABSTA+1
        +INC_IP
        LDA     (IP),Y
        STA     DABSTA+2
        LDA     ESTKL,X
DABSTA  STA     $FFFF
        JMP     NEXTOP
} ELSE {
DAB     INY     ;+INC_IP
        LDA     (IP),Y
        STA     TMPL
        +INC_IP
        LDA     (IP),Y
        STA     TMPH
        STY     IPY
        LDY     #$00
        LDA     ESTKL,X
        STA     (TMP),Y
        LDY     IPY
        JMP     NEXTOP
}
DAW     INY     ;+INC_IP
        LDA     (IP),Y
        STA     TMPL
        +INC_IP
        LDA     (IP),Y
        STA     TMPH
        STY     IPY
        LDY     #$00
        LDA     ESTKL,X
        STA     (TMP),Y
        INY
        LDA     ESTKH,X
        STA     (TMP),Y
        LDY     IPY
        JMP     NEXTOP
;*
;* COMPARES
;*
ISEQ    LDA     ESTKL,X
        CMP     ESTKL+1,X
        BNE     ISFLS
        LDA     ESTKH,X
        CMP     ESTKH+1,X
        BNE     ISFLS
ISTRU   LDA     #$FF
        STA     ESTKL+1,X
        STA     ESTKH+1,X
        JMP     DROP
;
ISNE    LDA     ESTKL,X
        CMP     ESTKL+1,X
        BNE     ISTRU
        LDA     ESTKH,X
        CMP     ESTKH+1,X
        BNE     ISTRU
ISFLS   LDA     #$00
        STA     ESTKL+1,X
        STA     ESTKH+1,X
        JMP     DROP
;
ISGE    LDA     ESTKL+1,X
        CMP     ESTKL,X
        LDA     ESTKH+1,X
        SBC     ESTKH,X
        BVC     ISGE1
        EOR     #$80
ISGE1   BPL     ISTRU
        BMI     ISFLS
;
ISGT    LDA     ESTKL,X
        CMP     ESTKL+1,X
        LDA     ESTKH,X
        SBC     ESTKH+1,X
        BVC     ISGT1
        EOR     #$80
ISGT1   BMI     ISTRU
        BPL     ISFLS
;
ISLE    LDA     ESTKL,X
        CMP     ESTKL+1,X
        LDA     ESTKH,X
        SBC     ESTKH+1,X
        BVC     ISLE1
        EOR     #$80
ISLE1   BPL     ISTRU
        BMI     ISFLS
;
ISLT    LDA     ESTKL+1,X
        CMP     ESTKL,X
        LDA     ESTKH+1,X
        SBC     ESTKH,X
        BVC     ISLT1
        EOR     #$80
ISLT1   BMI     ISTRU
        BPL     ISFLS
;*
;* BRANCHES
;*
BRTRU   INX
        LDA     ESTKH-1,X
        ORA     ESTKL-1,X
        BNE     BRNCH
NOBRNCH INY     ;+INC_IP
        +INC_IP
        JMP     NEXTOP
BRFLS   INX
        LDA     ESTKH-1,X
        ORA     ESTKL-1,X
        BNE     NOBRNCH
BRNCH   TYA                     ; FLATTEN IP
        SEC
        ADC     IPL
        STA     TMPL
        LDA     #$00
        TAY
        ADC     IPH
        STA     TMPH            ; ADD BRANCH OFFSET
        LDA     (TMP),Y
        ;CLC                    ; BETTER NOT CARRY OUT OF IP+Y
        ADC     TMPL
        STA     IPL
        INY
        LDA     (TMP),Y
        ADC     TMPH
        STA     IPH
        DEY
        JMP     FETCHOP
BREQ    INX
        LDA     ESTKL-1,X
        CMP     ESTKL,X
        BNE     NOBRNCH
        LDA     ESTKH-1,X
        CMP     ESTKH,X
        BEQ     BRNCH
        BNE     NOBRNCH
BRNE    INX
        LDA     ESTKL-1,X
        CMP     ESTKL,X
        BNE     BRNCH
        LDA     ESTKH-1,X
        CMP     ESTKH,X
        BEQ     NOBRNCH
        BNE     BRNCH
BRGT    INX
        LDA     ESTKL-1,X
        CMP     ESTKL,X
        LDA     ESTKH-1,X
        SBC     ESTKH,X
        BPL     NOBRNCH
        BMI     BRNCH
BRLT    INX
        LDA     ESTKL,X
        CMP     ESTKL-1,X
        LDA     ESTKH,X
        SBC     ESTKH-1,X
        BPL     NOBRNCH
        BMI     BRNCH
IBRNCH  LDA     IPL
        CLC
        ADC     ESTKL,X
        STA     IPL
        LDA     IPH
        ADC     ESTKH,X
        STA     IPH
        JMP     DROP
;*
;* INDIRECT CALL TO ADDRESS (NATIVE CODE)
;*
ICAL    LDA     ESTKL,X
!IF     SELFMODIFY {
        STA     CALLADR+1
} ELSE {
        STA     TMPL
}
        LDA     ESTKH,X
!IF     SELFMODIFY {
        STA     CALLADR+2
} ELSE {
        STA     TMPH
}
        INX
        BNE     _CALL
;*
;* CALL INTO ABSOLUTE ADDRESS (NATIVE CODE)
;*
CALL    INY     ;+INC_IP
        LDA     (IP),Y
!IF     SELFMODIFY {
        STA     CALLADR+1
} ELSE {
        STA     TMPL
}
        INY     ;+INC_IP
        LDA     (IP),Y
!IF     SELFMODIFY {
        STA     CALLADR+2
} ELSE {
        STA     TMPH
}
_CALL   TYA
        CLC
        ADC     IPL
        PHA
        LDA     IPH
        ADC     #$00
        PHA
!IF     SELFMODIFY {
CALLADR JSR $FFFF
} ELSE {
        JSR     JMPTMP
}
        PLA
        STA     IPH
        PLA
        STA     IPL
        LDY     #$01
        JMP     FETCHOP
;*
;* JUMP INDIRECT TRHOUGH TMP
;*
;JMPTMP  JMP     (TMP)
;*
;* ENTER FUNCTION WITH FRAME SIZE AND PARAM COUNT
;*
ENTER   INY
        LDA     (IP),Y
        EOR     #$FF
        SEC
        ADC     IFPL
        STA     IFPL
        BCS     +
        DEC     IFPH
+       INY
        LDA     (IP),Y
        BEQ     +
        ASL
        TAY
-       LDA     ESTKH,X
        DEY
        STA     (IFP),Y
        LDA     ESTKL,X
        INX
        DEY
        STA     (IFP),Y
        BNE     -
+       LDY     #$03
        JMP     FETCHOP
;*
;* LEAVE FUNCTION
;*
LEAVE   INY     ;+INC_IP
        LDA     (IP),Y
        CLC
        ADC     IFPL
        STA     IFPL
        BCS     LIFPH
        RTS
LIFPH   INC     IFPH
RET     RTS
A1CMD   !SOURCE "vmsrc/a1cmd.a"
SEGEND  =       *
VMINIT  LDY     #$10        ; INSTALL PAGE 0 FETCHOP ROUTINE
-       LDA     PAGE0-1,Y
        STA     DROP-1,Y
        DEY
        BNE     -
        STY     IFPL        ; INIT FRAME POINTER
        LDA     #$80
        STA     IFPH
        LDA     #<SEGEND    ; SAVE HEAP START
        STA     SRCL
        LDA     #>SEGEND
        STA     SRCH
        LDX     #ESTKSZ/2   ; INIT EVAL STACK INDEX
    JMP A1CMD
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

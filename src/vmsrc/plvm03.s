;**********************************************************
;*
;*              APPLE /// PLASMA INTERPETER
;*
;*             SYSTEM ROUTINES AND LOCATIONS
;*
;**********************************************************
;
; HARDWARE REGISTERS
;
MEMBANK =       $FFEF
        !SOURCE "vmsrc/plvmzp.inc"
;
; XPAGE ADDRESSES
;
XPAGE   =       $1600
DROPX   =       XPAGE+DROP
IFPX    =       XPAGE+IFPH
PPX     =       XPAGE+PPH
IPX     =       XPAGE+IPH
TMPX    =       XPAGE+TMPH
SRCX    =       XPAGE+SRCH
DSTX    =       XPAGE+DSTH
;*
;* SOS
;*
        !MACRO  SOS .CMD, .LIST {
        BRK
        !BYTE   .CMD
        !WORD   .LIST
        }
;*
;* INTERPRETER INSTRUCTION POINTER INCREMENT MACRO
;*
        !MACRO  INC_IP  {
        INY
        BNE     *+4
        INC     IPH
        }
;*
;* INTERPRETER HEADER+INITIALIZATION
;*
        SEGSTART        =       $A000
        *=      SEGSTART-$0E
        !TEXT   "SOS NTRP"
        !WORD   $0000
        !WORD   SEGSTART
        !WORD   SEGEND-SEGSTART

        +SOS    $40, SEGREQ     ; ALLOCATE SEG 1 AND MAP IT
        BNE     FAIL            ; PRHEX
        LDA     #$01
        STA     MEMBANK
        LDY     #$0F            ; INSTALL PAGE 0 FETCHOP ROUTINE
        LDA     #$00
-       LDX     PAGE0,Y
        STX     DROP,Y
        STA     DROPX,Y
        DEY
        BPL     -
        STA     TMPX            ; CLEAR ALL EXTENDED POINTERS
        STA     SRCX
        STA     DSTX
        STA     PPX             ; INIT FRAME & POOL POINTERS
        STA     IFPX
        LDA     #<SEGSTART
        STA     PPL
        STA     IFPL
        LDA     #>SEGSTART
        STA     PPH
        STA     IFPH
        LDX     #ESTKSZ/2       ; INIT EVAL STACK INDEX
        JMP     SOSCMD
;PRHEX   PHA
;        LSR
;        LSR
;        LSR
;        LSR
;        CLC
;        ADC     #'0'
;        CMP     #':'
;        BCC     +
;        ADC     #6
;+       STA     $480
;        PLA
;        AND     #$0F
;        ADC     #'0'
;        CMP     #':'
;        BCC     +
;        ADC     #6
;+       STA     $481    ;$880
FAIL    STA     $0480
        RTS
SEGREQ  !BYTE   4
        !WORD   $2001
        !WORD   $9F01
        !BYTE   $10
        !BYTE   $00
PAGE0   =       *
        !PSEUDOPC       $00EF {
;*
;* INTERP BYTECODE INNER LOOP
;*
        INX                     ; DROP
        INY                     ; NEXTOP
        BEQ     NEXTOPH
        LDA     $FFFF,Y         ; FETCHOP @ $F3, IP MAPS OVER $FFFF @ $F4
        STA     OPIDX
        JMP     (OPTBL)
NEXTOPH INC     IPH
        BNE     FETCHOP
}
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
        LDA     IFPH
        PHA                     ; SAVE ON STACK FOR LEAVE/RET
        LDA     IFPL
        PHA                     ; SAVE ON STACK FOR LEAVE/RET
        LDA     PPL             ; SET FP TO PP
        STA     IFPL
        LDA     PPH
        STA     IFPH
        LDY     #$00
        STY     IPX
        JMP     FETCHOP
;*
;* ENTER INTO USER BYTECODE INTERPRETER
;*
XINTERP PLA
        STA     TMPL
        PLA
        STA     TMPH
        LDY     #$03
        LDA     (TMP),Y
        STA     IPX
        DEY
        LDA     (TMP),Y
        STA     IPH
        DEY
        LDA     (TMP),Y
        STA     IPL
        DEY
        LDA     IFPH
        PHA                     ; SAVE ON STACK FOR LEAVE/RET
        LDA     IFPL
        PHA                     ; SAVE ON STACK FOR LEAVE/RET
        LDA     PPL             ; SET FP TO PP
        STA     IFPL
        LDA     PPH
        STA     IFPH
        JMP     FETCHOP
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
;* OPCODE TABLE
;*
        !ALIGN  255,0
OPTBL   !WORD   ZERO,ADD,SUB,MUL,DIV,MOD,INCR,DECR              ; 00 02 04 06 08 0A 0C 0E
        !WORD   NEG,COMP,BAND,IOR,XOR,SHL,SHR,IDXW              ; 10 12 14 16 18 1A 1C 1E
        !WORD   LNOT,LOR,LAND,LA,LLA,CB,CW,CS                   ; 20 22 24 26 28 2A 2C 2E
        !WORD   DROP,DUP,PUSHEP,PULLEP,BRGT,BRLT,BREQ,BRNE          ; 30 32 34 36 38 3A 3C 3E
        !WORD   ISEQ,ISNE,ISGT,ISLT,ISGE,ISLE,BRFLS,BRTRU       ; 40 42 44 46 48 4A 4C 4E
        !WORD   BRNCH,IBRNCH,CALL,ICAL,ENTER,LEAVE,RET,NEXTOP   ; 50 52 54 56 58 5A 5C 5E
        !WORD   LB,LW,LLB,LLW,LAB,LAW,DLB,DLW                   ; 60 62 64 66 68 6A 6C 6E
        !WORD   SB,SW,SLB,SLW,SAB,SAW,DAB,DAW                   ; 70 72 74 76 78 7A 7C 7E
        !WORD   CFFB                                            ; 80 82 84 86 88 8A 8C 8E
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
;       INX
;       JMP     NEXTOP
        JMP     DROP
;*
;* NEGATE TOS
;*
NEG     LDA     #$00
        SEC
        SBC     ESTKL,X
        STA     ESTKL,X
        LDA     #$00
        SBC     ESTKH,X
        STA     ESTKH,X
        JMP     NEXTOP
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
;* ADD TOS TO TOS-1
;*
ADD     LDA     ESTKL,X
        CLC
        ADC     ESTKL+1,X
        STA     ESTKL+1,X
        LDA     ESTKH,X
        ADC     ESTKH+1,X
        STA     ESTKH+1,X
;       INX
;       JMP     NEXTOP
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
;       INX
;       JMP     NEXTOP
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
;       INX
;       JMP     NEXTOP
        JMP     DROP
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
;* BITWISE AND TOS TO TOS-1
;*
BAND    LDA     ESTKL+1,X
        AND     ESTKL,X
        STA     ESTKL+1,X
        LDA     ESTKH+1,X
        AND     ESTKH,X
        STA     ESTKH+1,X
;       INX
;       JMP     NEXTOP
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
;       INX
;       JMP     NEXTOP
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
;       INX
;       JMP     NEXTOP
        JMP     DROP
;*
;* SHIFT TOS-1 LEFT BY TOS
;*
SHL     STY     IPY
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
SHL2    ASL     ESTKL+1,X
        ROL     ESTKH+1,X
        DEY
        BNE     SHL2
SHL3    LDY     IPY
;       INX
;       JMP     NEXTOP
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
;       INX
;       JMP     NEXTOP
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
;LAND2  INX
;       JMP     NEXTOP
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
;LOR1   INX
;       JMP     NEXTOP
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
;* PUSH EVAL STACK POINTER TO CALL STACK
;*
PUSHEP    TXA
        PHA
        JMP     NEXTOP
;*
;* PULL FROM CALL STACK TO EVAL STACK
;*
PULLEP    PLA
        TAX
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
        !BYTE   $2C             ; BIT $00A9 - effectively skips LDA #$00, no harm in reading this address
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
        +INC_IP
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
        +INC_IP
        TYA                     ; NORMALIZE IP
        CLC
        ADC     IPL
        STA     IPL
        LDA     #$00
        TAY
        ADC     IPH
        STA     IPH
        LDA     PPL             ; SCAN POOL FOR STRING ALREADY THERE
        STA     TMPL
        LDA     PPH
        STA     TMPH
_CMPPS  ;LDA    TMPH            ; CHECK FOR END OF POOL
        CMP     IFPH
        BCC     _CMPS           ; CHECK FOR MATCHING STRING
        BNE     _CPYS           ; BEYOND END OF POOL, COPY STRING OVER
        LDA     TMPL
        CMP     IFPL
        BCS     _CPYS           ; AT OR BEYOND END OF POOL, COPY STRING OVER
_CMPS   LDA     (TMP),Y         ; COMPARE STRINGS FROM AUX MEM TO STRINGS IN MAIN MEM
        CMP     (IP),Y          ; COMPARE STRING LENGTHS
        BNE     _CNXTS1
        TAY
_CMPCS  LDA     (TMP),Y         ; COMPARE STRING CHARS FROM END
        CMP     (IP),Y
        BNE     _CNXTS
        DEY
        BNE     _CMPCS
        LDA     TMPL            ; MATCH - SAVE EXISTING ADDR ON ESTK AND MOVE ON
        STA     ESTKL,X
        LDA     TMPH
        STA     ESTKH,X
        BNE     _CEXS
_CNXTS  LDY     #$00
        LDA     (TMP),Y
_CNXTS1 SEC
        ADC     TMPL
        STA     TMPL
        LDA     #$00
        ADC     TMPH
        STA     TMPH
        BNE     _CMPPS
_CPYS   LDA     (IP),Y          ; COPY STRING FROM AUX TO MAIN MEM POOL
        TAY                     ; MAKE ROOM IN POOL AND SAVE ADDR ON ESTK
        EOR     #$FF
        CLC
        ADC     PPL
        STA     PPL
        STA     ESTKL,X
        LDA     #$FF
        ADC     PPH
        STA     PPH
        STA     ESTKH,X         ; COPY STRING FROM AUX MEM BYTECODE TO MAIN MEM POOL
_CPYS1  LDA     (IP),Y          ; ALTRD IS ON,  NO NEED TO CHANGE IT HERE
        STA     (PP),Y          ; ALTWR IS OFF, NO NEED TO CHANGE IT HERE
        DEY
        CPY     #$FF
        BNE     _CPYS1
        INY
_CEXS   LDA     (IP),Y          ; SKIP TO NEXT OP ADDR AFTER STRING
        TAY
        JMP     NEXTOP
;*
;* LOAD VALUE FROM ADDRESS TAG
;*
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
LAB     +INC_IP
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
LAW     +INC_IP
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
;       INX
;       JMP     NEXTOP
        JMP     DROP
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
;       INX
;       JMP     NEXTOP
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
;       INX
;       JMP     NEXTOP
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
;       INX
;       JMP     NEXTOP
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
SAB     +INC_IP
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
;       INX
;       JMP     NEXTOP
        JMP     DROP
SAW     +INC_IP
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
;       INX
;       JMP     NEXTOP
        JMP     DROP
;*
;* STORE VALUE TO ABSOLUTE ADDRESS WITHOUT POPPING STACK
;*
DAB     +INC_IP
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
DAW     +INC_IP
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
;       INX
;       JMP     NEXTOP
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
;       INX
;       JMP     NEXTOP
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
NOBRNCH +INC_IP
        +INC_IP
        JMP     NEXTOP
BRFLS   INX
        LDA     ESTKH-1,X
        ORA     ESTKL-1,X
        BNE     NOBRNCH
BRNCH   LDA     IPH
        STA     TMPH
        LDA     IPL
        +INC_IP
        CLC
        ADC     (IP),Y
        STA     TMPL
        LDA     TMPH
        +INC_IP
        ADC     (IP),Y
        STA     IPH
        LDA     TMPL
        STA     IPL
        DEY
        DEY
        JMP     NEXTOP
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
        BMI     BRNCH
        BPL     NOBRNCH
BRLT    INX
        LDA     ESTKL,X
        CMP     ESTKL-1,X
        LDA     ESTKH,X
        SBC     ESTKH-1,X
        BMI     BRNCH
        BPL     NOBRNCH
IBRNCH  LDA     IPL
        CLC
        ADC     ESTKL,X
        STA     IPL
        LDA     IPH
        ADC     ESTKH,X
        STA     IPH
;       INX
;       JMP     NEXTOP
        JMP     DROP
;*
;* CALL INTO ABSOLUTE ADDRESS (NATIVE CODE)
;*
CALL    +INC_IP
        LDA     (IP),Y
        STA     CALLADR+1
        +INC_IP
        LDA     (IP),Y
        STA     CALLADR+2
        LDA     IPX
        PHA
        LDA     IPH
        PHA
        LDA     IPL
        PHA
        TYA
        PHA
CALLADR JSR     $FFFF
        PLA
        TAY
        PLA
        STA     IPL
        PLA
        STA     IPH
        PLA
        STA     IPX
        JMP     NEXTOP
;*
;* INDIRECT CALL TO ADDRESS (NATIVE CODE)
;*
ICAL    LDA     ESTKL,X
        STA     ICALADR+1
        LDA     ESTKH,X
        STA     ICALADR+2
        INX
        LDA     IPX
        PHA
        LDA     IPH
        PHA
        LDA     IPL
        PHA
        TYA
        PHA
ICALADR JSR     $FFFF
        PLA
        TAY
        PLA
        STA     IPL
        PLA
        STA     IPH
        PLA
        STA     IPX
        JMP     NEXTOP
;*
;* ENTER FUNCTION WITH FRAME SIZE AND PARAM COUNT
;*
ENTER   INY
        LDA     (IP),Y
        PHA                     ; SAVE ON STACK FOR LEAVE
        EOR     #$FF
        SEC
        ADC     PPL
        STA     PPL
        STA     IFPL
        LDA     #$FF
        ADC     PPH
        STA     PPH
        STA     IFPH
        INY
        LDA     (IP),Y
        ASL
        TAY
        BEQ     +
-       LDA     ESTKH,X
        DEY
        STA     (IFP),Y
        LDA     ESTKL,X
        INX
        DEY
        STA     (IFP),Y
        BNE     -
+       LDY     #$02
        JMP     NEXTOP
;*
;* LEAVE FUNCTION
;*
LEAVE   PLA
        CLC
        ADC     IFPL
        STA     PPL
        LDA     #$00
        ADC     IFPH
        STA     PPH
        PLA                     ; RESTORE PREVIOUS FRAME
        STA     IFPL
        PLA
        STA     IFPH
        RTS
;
RET     LDA     IFPL            ; DEALLOCATE POOL
        STA     PPL
        LDA     IFPH
        STA     PPH
        PLA                     ; RESTORE PREVIOUS FRAME
        STA     IFPL
        PLA
        STA     IFPH
        RTS
SOSCMD  =       *
        !SOURCE "vmsrc/soscmd.a"
SEGEND  =       *

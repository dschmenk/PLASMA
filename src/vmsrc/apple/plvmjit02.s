;**********************************************************
;*
;*          APPLE ][ 128K PLASMA INTERPRETER
;*
;*              SYSTEM ROUTINES AND LOCATIONS
;*
;**********************************************************
                !CPU    65C02
;*
;* MONITOR SPECIAL LOCATIONS
;*
CSWL    =       $36
CSWH    =       $37
PROMPT  =       $33
;*
;* PRODOS
;*
PRODOS  =       $BF00
DEVCNT  =       $BF31            ; GLOBAL PAGE DEVICE COUNT
DEVLST  =       $BF32            ; GLOBAL PAGE DEVICE LIST
MACHID  =       $BF98            ; GLOBAL PAGE MACHINE ID BYTE
RAMSLOT =       $BF26            ; SLOT 3, DRIVE 2 IS /RAM'S DRIVER VECTOR
NODEV   =       $BF10
;*
;* HARDWARE ADDRESSES
;*
KEYBD   =       $C000
CLRKBD  =       $C010
SPKR    =       $C030
LCRDEN  =       $C080
LCWTEN  =       $C081
ROMEN   =       $C082
LCRWEN  =       $C083
LCBNK2  =       $00
LCBNK1  =       $08
ALTZPOFF=       $C008
ALTZPON =       $C009
ALTRDOFF=       $C002
ALTRDON =       $C003
ALTWROFF=       $C004
ALTWRON =       $C005
        !SOURCE "vmsrc/plvmzp.inc"
DVSIGN  =       TMP+2
DROP    =       $EF
NEXTOP  =       $F0
FETCHOP =       NEXTOP+1
IP      =       FETCHOP+1
IPL     =       IP
IPH     =       IPL+1
OPIDX   =       FETCHOP+6
OPPAGE  =       OPIDX+1
STRBUF  =       $0300
JITMOD  =       $02F0
INTERP  =       $03D0
JITCOMP =       $03E2
JITCODE =       $03E4
;******************************
;*                            *
;* INTERPRETER INITIALIZATION *
;*                            *
;******************************
*       =       $2000
;*
;* MUST HAVE 128K FOR JIT
;*
+       LDA     MACHID
        AND     #$30
        CMP     #$30
        BEQ     ++
        LDY     #$00
-       LDA     NEEDAUX,Y
        BEQ     +
        ORA     #$80
        JSR     $FDED
        INY
        BNE     -
+       LDA     $C000
        BPL     -
        LDA     $C010
        JSR     PRODOS
        !BYTE   $65
        !WORD   BYEPARMS
BYEPARMS !BYTE  4
        !BYTE   4
        !WORD   0
        !BYTE   0
        !WORD   0
NEEDAUX !TEXT   "128K MEMORY REQUIRED.", 13
        !TEXT   "PRESS ANY KEY...", 0
;*
;* DISCONNECT /RAM
;*
++      ;SEI                    ; DISABLE /RAM
        LDA     RAMSLOT
        CMP     NODEV
        BNE     RAMCONT
        LDA     RAMSLOT+1
        CMP     NODEV+1
        BEQ     RAMDONE
RAMCONT LDY     DEVCNT
RAMLOOP LDA     DEVLST,Y
        AND     #$F3
        CMP     #$B3
        BEQ     GETLOOP
        DEY
        BPL     RAMLOOP
        BMI     RAMDONE
GETLOOP LDA     DEVLST+1,Y
        STA     DEVLST,Y
        BEQ     RAMEXIT
        INY
        BNE     GETLOOP
RAMEXIT LDA     NODEV
        STA     RAMSLOT
        LDA     NODEV+1
        STA     RAMSLOT+1
        DEC     DEVCNT
RAMDONE ;CLI UNTIL I KNOW WHAT TO DO WITH THE UNENHANCED IIE
;*
;* MOVE VM INTO LANGUAGE CARD
;*
        BIT     LCRWEN+LCBNK2
        BIT     LCRWEN+LCBNK2
        LDA     #<VMCORE
        STA     SRCL
        LDA     #>VMCORE
        STA     SRCH
        LDY     #$00
        STY     DSTL
        LDA     #$D0
        STA     DSTH
-       LDA     (SRC),Y         ; COPY VM+BYE INTO LANGUAGE CARD
        STA     (DST),Y
        INY
        BNE     -
        INC     SRCH
        INC     DSTH
        LDA     DSTH
        CMP     #$E0
        BNE     -
;*
;* MOVE FIRST PAGE OF 'BYE' INTO PLACE
;*
-       LDA     $D100,Y
        STA     $1000,Y
        INY
        BNE     -
;*
;* INSERT 65C02 OPS IF APPLICABLE
;*
        LDA     #$00
        INC
        BEQ     +
        JSR     C02OPS
;*
;* SAVE DEFAULT COMMAND INTERPRETER PATH IN LC
;*
+       JSR     PRODOS          ; GET PREFIX
        !BYTE   $C7
        !WORD   GETPFXPARMS
        LDY     STRBUF          ; APPEND "CMDJIT"
        LDA     #"/"
        CMP     STRBUF,Y
        BEQ     +
        INY
        STA     STRBUF,Y
+       LDA     #"C"
        INY
        STA     STRBUF,Y
        LDA     #"M"
        INY
        STA     STRBUF,Y
        LDA     #"D"
        INY
        STA     STRBUF,Y
        LDA     #"1"
        INY
        STA     STRBUF,Y
        LDA     #"2"
        INY
        STA     STRBUF,Y
        LDA     #"8"
        INY
        STA     STRBUF,Y
        STY     STRBUF
        BIT     LCRWEN+LCBNK2    ; COPY TO LC FOR BYE
        BIT     LCRWEN+LCBNK2
-       LDA     STRBUF,Y
        STA     LCDEFCMD,Y
        DEY
        BPL     -
        JMP     CMDENTRY
GETPFXPARMS !BYTE 1
        !WORD   STRBUF          ; PATH STRING GOES HERE
;************************************************
;*                                              *
;* LANGUAGE CARD RESIDENT PLASMA VM STARTS HERE *
;*                                              *
;************************************************
VMCORE  =        *
        !PSEUDOPC       $D000 {
;****************
;*              *
;* OPCODE TABLE *
;*              *
;****************
        !ALIGN  255,0
OPTBL   !WORD   ZERO,CN,CN,CN,CN,CN,CN,CN                               ; 00 02 04 06 08 0A 0C 0E
        !WORD   CN,CN,CN,CN,CN,CN,CN,CN                                 ; 10 12 14 16 18 1A 1C 1E
        !WORD   MINUS1,BREQ,BRNE,LA,LLA,CB,CW,CS                        ; 20 22 24 26 28 2A 2C 2E
        !WORD   DROP,DROP2,DUP,DIVMOD,ADDI,SUBI,ANDI,ORI                ; 30 32 34 36 38 3A 3C 3E
        !WORD   ISEQ,ISNE,ISGT,ISLT,ISGE,ISLE,BRFLS,BRTRU               ; 40 42 44 46 48 4A 4C 4E
        !WORD   BRNCH,SEL,CALL,ICAL,ENTER,LEAVE,RET,CFFB                ; 50 52 54 56 58 5A 5C 5E
        !WORD   LB,LW,LLB,LLW,LAB,LAW,DLB,DLW                           ; 60 62 64 66 68 6A 6C 6E
        !WORD   SB,SW,SLB,SLW,SAB,SAW,DAB,DAW                           ; 70 72 74 76 78 7A 7C 7E
        !WORD   LNOT,ADD,SUB,MUL,DIV,MOD,INCR,DECR                      ; 80 82 84 86 88 8A 8C 8E
        !WORD   NEG,COMP,BAND,IOR,XOR,SHL,SHR,IDXW                      ; 90 92 94 96 98 9A 9C 9E
        !WORD   BRGT,BRLT,INCBRLE,ADDBRLE,DECBRGE,SUBBRGE,BRAND,BROR    ; A0 A2 A4 A6 A8 AA AC AE
        !WORD   ADDLB,ADDLW,ADDAB,ADDAW,IDXLB,IDXLW,IDXAB,IDXAW         ; B0 B2 B4 B6 B8 BA BC BE
        !WORD   NATV                                                    ; C0
;*
;* DIRECTLY ENTER INTO BYTECODE INTERPRETER
;*
DINTRP  PLA
        CLC
        ADC     #$01
        STA     IPL
        PLA
        ADC     #$00
        STA     IPH
        LDY     #$00
        LDA     #>OPTBL
        STA     OPPAGE
        JMP     FETCHOP
;*
;* INDIRECTLY ENTER INTO BYTECODE INTERPRETER
;*
IINTRPX PLA
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
        LDA     #>OPXTBL
        STA     OPPAGE
        PHP                     ; SAVE PSR
        SEI
        STA     ALTRDON
        JMP     FETCHOP
;************************************************************
;*                                                          *
;* 'BYE' PROCESSING - COPIED TO $1000 ON PRODOS BYE COMMAND *
;*                                                          *
;************************************************************
        !ALIGN  255,0
        !PSEUDOPC       $1000 {
BYE     LDY     DEFCMD
-       LDA     DEFCMD,Y        ; SET DEFAULT COMMAND WHEN CALLED FROM 'BYE'
        STA     STRBUF,Y
        DEY
        BPL     -
;        INY                     ; CLEAR CMDLINE BUFF
;        STY     $01FF
CMDENTRY =      *
;
; DEACTIVATE 80 COL CARDS AND SET DCI STRING FOR JIT MODULE
;
        BIT     ROMEN
        LDY     #4
-       LDA     DISABLE80,Y
        ORA     #$80
        JSR     $FDED
        LDA     JITDCI,Y
        STA     JITMOD,Y
        DEY
        BPL     -
        BIT     $C054           ; SET TEXT MODE
        BIT     $C051
        BIT     $C05F
        JSR     $FC58           ; HOME
;
; INSTALL PAGE 0 FETCHOP ROUTINE
;
        LDY     #$0F
-       LDA     PAGE0,Y
        STA     DROP,Y
        DEY
        BPL     -
;
; SET JMPTMP OPCODE
;
        LDA     #$4C
        STA     JMPTMP
;
; INSTALL PAGE 3 VECTORS
;
        LDY     #$12
-       LDA     PAGE3,Y
        STA     INTERP,Y
        DEY
        BPL     -
;
; READ CMD INTO MEMORY
;
        JSR     PRODOS          ; CLOSE EVERYTHING
        !BYTE   $CC
        !WORD   CLOSEPARMS
        BNE     FAIL
        JSR     PRODOS          ; OPEN CMD
        !BYTE   $C8
        !WORD   OPENPARMS
        BNE     FAIL
        LDA     REFNUM
        STA     READPARMS+1
        JSR     PRODOS
        !BYTE   $CA
        !WORD   READPARMS
        BNE     FAIL
        JSR     PRODOS
        !BYTE   $CC
        !WORD   CLOSEPARMS
        BNE     FAIL
;
; CHANGE CMD STRING TO SYSPATH STRING
;
        LDA     STRBUF
        SEC
        SBC     #$06
        STA     STRBUF
        JMP     $2000           ; JUMP TO LOADED SYSTEM COMMAND
;
; PRINT FAIL MESSAGE, WAIT FOR KEYPRESS, AND REBOOT
;
FAIL    INC     $3F4            ; INVALIDATE POWER-UP BYTE
        LDY     #11
-       LDA     FAILMSG,Y
        ORA     #$80
        JSR     $FDED
        DEY
        BPL     -
        JSR     $FD0C           ; WAIT FOR KEYPRESS
        JMP     ($FFFC)         ; RESET
OPENPARMS !BYTE 3
        !WORD   STRBUF
        !WORD   $0800
REFNUM  !BYTE   0
READPARMS !BYTE 4
        !BYTE   0
        !WORD   $2000
        !WORD   $9F00
        !WORD   0
CLOSEPARMS !BYTE 1
        !BYTE   0
DISABLE80 !BYTE 21, 13, '1', 26, 13
JITDCI  !BYTE       'J'|$80,'I'|$80,'T'
FAILMSG !TEXT   ".DMC GNISSIM"
PAGE0    =      *
;******************************
;*                            *
;* INTERP BYTECODE INNER LOOP *
;*                            *
;******************************
        !PSEUDOPC       DROP {
        INX                     ; DROP @ $EF
        INY                     ; NEXTOP @ $F0
        LDA     $FFFF,Y         ; FETCHOP @ $F1, IP MAPS OVER $FFFF @ $F2
        STA     OPIDX
        JMP     (OPTBL)         ; OPIDX AND OPPAGE MAP OVER OPTBL
}
PAGE3   =       *
;*
;* PAGE 3 VECTORS INTO INTERPRETER
;*
        !PSEUDOPC       $03D0 {
        BIT     LCRDEN+LCBNK2   ; $03D0 - BYTECODE DIRECT INTERP ENTRY
        JMP     DINTRP
        BIT     LCRDEN+LCBNK2   ; $03D6 - JIT INDIRECT INTERPX ENTRY
        JMP     JITINTRPX
        BIT     LCRDEN+LCBNK2   ; $03DC - BYTECODE INDIRECT INTERPX ENTRY
        JMP     IINTRPX
}
DEFCMD  =       *               ;!FILL   28
ENDBYE  =       *
}
LCDEFCMD =      *               ;*-28            ; DEFCMD IN LC MEMORY
;*****************
;*               *
;* OPXCODE TABLE *
;*               *
;*****************
        !ALIGN  255,0
OPXTBL  !WORD   ZERO,CN,CN,CN,CN,CN,CN,CN                               ; 00 02 04 06 08 0A 0C 0E
        !WORD   CN,CN,CN,CN,CN,CN,CN,CN                                 ; 10 12 14 16 18 1A 1C 1E
        !WORD   MINUS1,BREQ,BRNE,LA,LLA,CB,CW,CSX                       ; 20 22 24 26 28 2A 2C 2E
        !WORD   DROP,DROP2,DUP,DIVMOD,ADDI,SUBI,ANDI,ORI                ; 30 32 34 36 38 3A 3C 3E
        !WORD   ISEQ,ISNE,ISGT,ISLT,ISGE,ISLE,BRFLS,BRTRU               ; 40 42 44 46 48 4A 4C 4E
        !WORD   BRNCH,SEL,CALLX,ICALX,ENTERX,LEAVEX,RETX,CFFB           ; 50 52 54 56 58 5A 5C 5E
        !WORD   LBX,LWX,LLBX,LLWX,LABX,LAWX,DLB,DLW                     ; 60 62 64 66 68 6A 6C 6E
        !WORD   SB,SW,SLB,SLW,SAB,SAW,DAB,DAW                           ; 70 72 74 76 78 7A 7C 7E
        !WORD   LNOT,ADD,SUB,MUL,DIV,MOD,INCR,DECR                      ; 80 82 84 86 88 8A 8C 8E
        !WORD   NEG,COMP,BAND,IOR,XOR,SHL,SHR,IDXW                      ; 90 92 94 96 98 9A 9C 9E
        !WORD   BRGT,BRLT,INCBRLE,ADDBRLE,DECBRGE,SUBBRGE,BRAND,BROR    ; A0 A2 A4 A6 A8 AA AC AE
        !WORD   ADDLBX,ADDLWX,ADDABX,ADDAWX,IDXLBX,IDXLWX,IDXABX,IDXAWX ; B0 B2 B4 B6 B8 BA BC BE
        !WORD   NATV                                                    ; C0
;*
;* JIT PROFILING ENTRY INTO INTERPRETER
;*
JITINTRPX PLA
        SEC
        SBC     #$02            ; POINT TO DEF ENTRY
        STA     TMPL
        PLA
        SBC     #$00
        STA     TMPH
        LDY     #$05
        LDA     (TMP),Y         ; DEC JIT COUNT
        SEC
        SBC     #$01
        STA     (TMP),Y
        BEQ     RUNJIT
        DEY                     ; INTERP BYTECODE AS USUAL
        LDA     (TMP),Y
        STA     IPH
        DEY
        LDA     (TMP),Y
        STA     IPL
        LDY     #$00
        LDA     #>OPXTBL
        STA     OPPAGE
        PHP
        SEI
        STA     ALTRDON
        JMP     FETCHOP
RUNJIT  LDA     JITCOMP
        STA     SRCL
        LDA     JITCOMP+1
        STA     SRCH
        DEY                     ; LDY     #$04
        LDA     (SRC),Y
        STA     IPH
        DEY
        LDA     (SRC),Y
        STA     IPL
        DEX                     ; ADD PARAMETER TO DEF ENTRY
        LDA     TMPL
        PHA                     ; AND SAVE IT FOR LATER
        STA     ESTKL,X
        LDA     TMPH
        PHA
        STA     ESTKH,X
        LDY     #$00
        LDA     #>OPXTBL
        STA     OPPAGE
        LDA     #>(RETJIT-1)
        PHA
        LDA     #<(RETJIT-1)
        PHA
        PHP
        SEI
        STA     ALTRDON
        JMP     FETCHOP         ; CALL JIT COMPILER
RETJIT  PLA
        STA     TMPH
        PLA
        STA     TMPL
        JMP     (TMP)           ; RE-CALL ORIGINAL DEF ENTRY
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
_MULLP  LSR     TMPH            ; MULTPLRH
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
        BNE     _MULLP
        STA     ESTKH+1,X       ; PRODH
        LDY     IPY
        JMP     DROP
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
        STA     DVSIGN
        LDA     ESTKH+1,X
        BPL     +
        INX
        JSR     _NEG
        DEX
        LDA     #$81
        STA     DVSIGN
+       ORA     ESTKL+1,X         ; DVDNDL
        BEQ     _DIVEX
        LDA     ESTKH,X
        BPL     _DIV1
        JSR     _NEG
        INC     DVSIGN
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
;* DIVMOD TOS-1 BY TOS
;*
DIVMOD  JSR     _DIV
        LSR     DVSIGN          ; SIGN(RESULT) = (SIGN(DIVIDEND) + SIGN(DIVISOR)) & 1
        BCC     +
        JSR     _NEG
+       DEX
        LDA     TMPL            ; REMNDRL
        STA     ESTKL,X
        LDA     TMPH            ; REMNDRH
        STA     ESTKH,X
        ASL     DVSIGN          ; REMAINDER IS SIGN OF DIVIDEND
        BMI     NEG
        JMP     NEXTOP
;*
;* INCREMENT TOS
;*
INCR    INC     ESTKL,X
        BEQ     +
        JMP     NEXTOP
+       INC     ESTKH,X
        JMP     NEXTOP
;*
;* DECREMENT TOS
;*
DECR    LDA     ESTKL,X
        BEQ     +
        DEC     ESTKL,X
        JMP     NEXTOP
+       DEC     ESTKL,X
        DEC     ESTKH,X
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
SHL     STY     IPY
        LDA     ESTKL,X
        CMP     #$08
        BCC     +
        LDY     ESTKL+1,X
        STY     ESTKH+1,X
        LDY     #$00
        STY     ESTKL+1,X
        SBC     #$08
+       TAY
        BEQ     +
        LDA     ESTKL+1,X
-       ASL
        ROL     ESTKH+1,X
        DEY
        BNE     -
        STA     ESTKL+1,X
+       LDY     IPY
        JMP     DROP
;*
;* SHIFT TOS-1 RIGHT BY TOS
;*
SHR     STY     IPY
        LDA     ESTKL,X
        CMP     #$08
        BCC     ++
        LDY     ESTKH+1,X
        STY     ESTKL+1,X
        CPY     #$80
        LDY     #$00
        BCC     +
        DEY
+       STY     ESTKH+1,X
        SEC
        SBC     #$08
++      TAY
        BEQ     +
        LDA     ESTKH+1,X
-       CMP     #$80
        ROR
        ROR     ESTKL+1,X
        DEY
        BNE     -
        STA     ESTKH+1,X
+       LDY     IPY
        JMP     DROP
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
;* ADD IMMEDIATE TO TOS
;*
ADDI    INY                     ;+INC_IP
        LDA     (IP),Y
        CLC
        ADC     ESTKL,X
        STA     ESTKL,X
        BCC     +
        INC     ESTKH,X
+       JMP     NEXTOP
;*
;* SUB IMMEDIATE FROM TOS
;*
SUBI    INY                     ;+INC_IP
        LDA     ESTKL,X
        SEC
        SBC     (IP),Y
        STA     ESTKL,X
        BCS     +
        DEC     ESTKH,X
+       JMP     NEXTOP
;*
;* AND IMMEDIATE TO TOS
;*
ANDI    INY                     ;+INC_IP
        LDA     (IP),Y
        AND     ESTKL,X
        STA     ESTKL,X
        LDA     #$00
        STA     ESTKH,X
        JMP     NEXTOP
;*
;* IOR IMMEDIATE TO TOS
;*
ORI     INY                     ;+INC_IP
        LDA     (IP),Y
        ORA     ESTKL,X
        STA     ESTKL,X
        JMP     NEXTOP
;*
;* LOGICAL NOT
;*
LNOT    LDA     ESTKL,X
        ORA     ESTKH,X
        BEQ     +
        LDA     #$00
        STA     ESTKL,X
        STA     ESTKH,X
        JMP     NEXTOP
;*
;* CONSTANT -1, ZERO, NYBBLE, BYTE, $FF BYTE, WORD (BELOW)
;*
MINUS1  DEX
+       LDA     #$FF
        STA     ESTKL,X
        STA     ESTKH,X
        JMP     NEXTOP
ZERO    DEX
        STA     ESTKL,X
        STA     ESTKH,X
        JMP     NEXTOP
CN      DEX
        LSR                     ; A = CONST * 2
        STA     ESTKL,X
        LDA     #$00
        STA     ESTKH,X
        JMP     NEXTOP
CB      DEX
        LDA     #$00
        STA     ESTKH,X
        INY                     ;+INC_IP
        LDA     (IP),Y
        STA     ESTKL,X
        JMP     NEXTOP
CFFB    DEX
        LDA     #$FF
        STA     ESTKH,X
        INY                     ;+INC_IP
        LDA     (IP),Y
        STA     ESTKL,X
        JMP     NEXTOP
;*
;* LOAD ADDRESS & LOAD CONSTANT WORD (SAME THING, WITH OR WITHOUT FIXUP)
;*
-       TYA                     ; RENORMALIZE IP
        CLC
        ADC     IPL
        STA     IPL
        BCC     +
        INC     IPH
+       LDY     #$FF
LA      INY                     ;+INC_IP
        BMI     -
        DEX
        LDA     (IP),Y
        STA     ESTKL,X
        INY
        LDA     (IP),Y
        STA     ESTKH,X
        JMP     NEXTOP
CW      DEX
        INY                     ;+INC_IP
        LDA     (IP),Y
        STA     ESTKL,X
        INY
        LDA     (IP),Y
        STA     ESTKH,X
        JMP     NEXTOP
;*
;* CONSTANT STRING
;*
CS      DEX
        ;INY                     ;+INC_IP
        TYA                     ; NORMALIZE IP AND SAVE STRING ADDR ON ESTK
        SEC
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
CSX     DEX
        ;INY                     ;+INC_IP
        TYA                     ; NORMALIZE IP
        SEC
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
_CMPPSX ;LDA    TMPH            ; CHECK FOR END OF POOL
        CMP     IFPH
        BCC     _CMPSX          ; CHECK FOR MATCHING STRING
        BNE     _CPYSX          ; BEYOND END OF POOL, COPY STRING OVER
        LDA     TMPL
        CMP     IFPL
        BCS     _CPYSX          ; AT OR BEYOND END OF POOL, COPY STRING OVER
_CMPSX  STA     ALTRDOFF
        LDA     (TMP),Y         ; COMPARE STRINGS FROM AUX MEM TO STRINGS IN MAIN MEM
        STA     ALTRDON
        CMP     (IP),Y          ; COMPARE STRING LENGTHS
        BNE     _CNXTSX1
        TAY
_CMPCSX STA     ALTRDOFF
        LDA     (TMP),Y         ; COMPARE STRING CHARS FROM END
        STA     ALTRDON
        CMP     (IP),Y
        BNE     _CNXTSX
        DEY
        BNE     _CMPCSX
        LDA     TMPL            ; MATCH - SAVE EXISTING ADDR ON ESTK AND MOVE ON
        STA     ESTKL,X
        LDA     TMPH
        STA     ESTKH,X
        BNE     _CEXSX
_CNXTSX LDY     #$00
        STA     ALTRDOFF
        LDA     (TMP),Y
        STA     ALTRDON
_CNXTSX1 SEC
        ADC     TMPL
        STA     TMPL
        LDA     #$00
        ADC     TMPH
        STA     TMPH
        BNE     _CMPPSX
_CPYSX  LDA     (IP),Y          ; COPY STRING FROM AUX TO MAIN MEM POOL
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
_CPYSX1 LDA     (IP),Y          ; ALTRD IS ON,  NO NEED TO CHANGE IT HERE
        STA     (PP),Y          ; ALTWR IS OFF, NO NEED TO CHANGE IT HERE
        DEY
        CPY     #$FF
        BNE     _CPYSX1
        INY
_CEXSX  LDA     (IP),Y          ; SKIP TO NEXT OP ADDR AFTER STRING
        TAY
        JMP     NEXTOP
;*
;* LOAD VALUE FROM ADDRESS TAG
;*
LB      LDA     ESTKL,X
        STA     ESTKH-1,X
        LDA     (ESTKH-1,X)
        STA     ESTKL,X
        LDA     #$00
        STA     ESTKH,X
        JMP     NEXTOP
LW      LDA     ESTKL,X
        STA     ESTKH-1,X
        LDA     (ESTKH-1,X)
        STA     ESTKL,X
        INC     ESTKH-1,X
        BEQ     +
        LDA     (ESTKH-1,X)
        STA     ESTKH,X
        JMP     NEXTOP
+       INC     ESTKH,X
        LDA     (ESTKH-1,X)
        STA     ESTKH,X
        JMP     NEXTOP
LBX     LDA     ESTKL,X
        STA     ESTKH-1,X
        STA     ALTRDOFF
        LDA     (ESTKH-1,X)
        STA     ESTKL,X
        LDA     #$00
        STA     ESTKH,X
        STA     ALTRDON
        JMP     NEXTOP
LWX     LDA     ESTKL,X
        STA     ESTKH-1,X
        STA     ALTRDOFF
        LDA     (ESTKH-1,X)
        STA     ESTKL,X
        INC     ESTKH-1,X
        BEQ     +
        LDA     (ESTKH-1,X)
        STA     ESTKH,X
        STA     ALTRDON
        JMP     NEXTOP
+       INC     ESTKH,X
        LDA     (ESTKH-1,X)
        STA     ESTKH,X
        STA     ALTRDON
        JMP     NEXTOP
;*
;* LOAD ADDRESS OF LOCAL FRAME OFFSET
;*
-       TYA                     ; RENORMALIZE IP
        CLC
        ADC     IPL
        STA     IPL
        BCC     +
        INC     IPH
+       LDY     #$FF
LLA     INY                     ;+INC_IP
        BMI     -
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
LLB     INY                     ;+INC_IP
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
LLW     INY                     ;+INC_IP
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
LLBX    INY                     ;+INC_IP
        LDA     (IP),Y
        STY     IPY
        TAY
        DEX
        STA     ALTRDOFF
        LDA     (IFP),Y
        STA     ESTKL,X
        LDA     #$00
        STA     ESTKH,X
        STA     ALTRDON
        LDY     IPY
        JMP     NEXTOP
LLWX    INY                     ;+INC_IP
        LDA     (IP),Y
        STY     IPY
        TAY
        DEX
        STA     ALTRDOFF
        LDA     (IFP),Y
        STA     ESTKL,X
        INY
        LDA     (IFP),Y
        STA     ESTKH,X
        STA     ALTRDON
        LDY     IPY
        JMP     NEXTOP
;*
;* ADD VALUE FROM LOCAL FRAME OFFSET
;*
ADDLB   INY                     ;+INC_IP
        LDA     (IP),Y
        STY     IPY
        TAY
        LDA     (IFP),Y
        CLC
        ADC     ESTKL,X
        STA     ESTKL,X
        BCC     +
        INC     ESTKH,X
+       LDY     IPY
        JMP     NEXTOP
ADDLBX  INY                     ;+INC_IP
        LDA     (IP),Y
        STY     IPY
        TAY
        STA     ALTRDOFF
        LDA     (IFP),Y
        CLC
        ADC     ESTKL,X
        STA     ESTKL,X
        BCC     +
        INC     ESTKH,X
+       STA     ALTRDON
        LDY     IPY
        JMP     NEXTOP
ADDLW   INY                     ;+INC_IP
        LDA     (IP),Y
        STY     IPY
        TAY
        LDA     (IFP),Y
        CLC
        ADC     ESTKL,X
        STA     ESTKL,X
        INY
        LDA     (IFP),Y
        ADC     ESTKH,X
        STA     ESTKH,X
        LDY     IPY
        JMP     NEXTOP
ADDLWX  INY                     ;+INC_IP
        LDA     (IP),Y
        STY     IPY
        TAY
        STA     ALTRDOFF
        LDA     (IFP),Y
        CLC
        ADC     ESTKL,X
        STA     ESTKL,X
        INY
        LDA     (IFP),Y
        ADC     ESTKH,X
        STA     ESTKH,X
        STA     ALTRDON
        LDY     IPY
        JMP     NEXTOP
;*
;* INDEX VALUE FROM LOCAL FRAME OFFSET
;*
IDXLB   INY                     ;+INC_IP
        LDA     (IP),Y
        STY     IPY
        TAY
        LDA     (IFP),Y
        LDY     #$00
        ASL
        BCC     +
        INY
        CLC
+       ADC     ESTKL,X
        STA     ESTKL,X
        TYA
        ADC     ESTKH,X
        STA     ESTKH,X
        LDY     IPY
        JMP     NEXTOP
IDXLBX  INY                     ;+INC_IP
        LDA     (IP),Y
        STY     IPY
        TAY
        STA     ALTRDOFF
        LDA     (IFP),Y
        LDY     #$00
        ASL
        BCC     +
        INY
        CLC
+       ADC     ESTKL,X
        STA     ESTKL,X
        TYA
        ADC     ESTKH,X
        STA     ESTKH,X
        STA     ALTRDON
        LDY     IPY
        JMP     NEXTOP
IDXLW   INY                     ;+INC_IP
        LDA     (IP),Y
        STY     IPY
        TAY
        LDA     (IFP),Y
        ASL
        STA     TMPL
        INY
        LDA     (IFP),Y
        ROL
        STA     TMPH
        LDA     TMPL
        CLC
        ADC     ESTKL,X
        STA     ESTKL,X
        LDA     TMPH
        ADC     ESTKH,X
        STA     ESTKH,X
        LDY     IPY
        JMP     NEXTOP
IDXLWX  INY                     ;+INC_IP
        LDA     (IP),Y
        STY     IPY
        TAY
        STA     ALTRDOFF
        LDA     (IFP),Y
        ASL
        STA     TMPL
        INY
        LDA     (IFP),Y
        ROL
        STA     TMPH
        LDA     TMPL
        CLC
        ADC     ESTKL,X
        STA     ESTKL,X
        LDA     TMPH
        ADC     ESTKH,X
        STA     ESTKH,X
        STA     ALTRDON
        LDY     IPY
        JMP     NEXTOP
;*
;* LOAD VALUE FROM ABSOLUTE ADDRESS
;*
LAB     INY                     ;+INC_IP
        LDA     (IP),Y
        STA     ESTKH-2,X
        INY                     ;+INC_IP
        LDA     (IP),Y
        STA     ESTKH-1,X
        LDA     (ESTKH-2,X)
        DEX
        STA     ESTKL,X
        LDA     #$00
        STA     ESTKH,X
        JMP     NEXTOP
LAW     INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMPL
        INY                     ;+INC_IP
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
LABX    INY                     ;+INC_IP
        LDA     (IP),Y
        STA     ESTKH-2,X
        INY                     ;+INC_IP
        LDA     (IP),Y
        STA     ESTKH-1,X
        STA     ALTRDOFF
        LDA     (ESTKH-2,X)
        DEX
        STA     ESTKL,X
        LDA     #$00
        STA     ESTKH,X
        STA     ALTRDON
        JMP     NEXTOP
LAWX    INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMPL
        INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMPH
        STY     IPY
        STA     ALTRDOFF
        LDY     #$00
        LDA     (TMP),Y
        DEX
        STA     ESTKL,X
        INY
        LDA     (TMP),Y
        STA     ESTKH,X
        STA     ALTRDON
        LDY     IPY
        JMP     NEXTOP
;*
;* ADD VALUE FROM ABSOLUTE ADDRESS
;*
ADDAB   INY                     ;+INC_IP
        LDA     (IP),Y
        STA     ESTKH-2,X
        INY                     ;+INC_IP
        LDA     (IP),Y
        STA     ESTKH-1,X
        LDA     (ESTKH-2,X)
        CLC
        ADC     ESTKL,X
        STA     ESTKL,X
        BCC     +
        INC     ESTKH,X
+       JMP     NEXTOP
ADDABX  INY                     ;+INC_IP
        LDA     (IP),Y
        STA     ESTKH-2,X
        INY                     ;+INC_IP
        LDA     (IP),Y
        STA     ESTKH-1,X
        STA     ALTRDOFF
        LDA     (ESTKH-2,X)
        CLC
        ADC     ESTKL,X
        STA     ESTKL,X
        BCC     +
        INC     ESTKH,X
+       STA     ALTRDON
        JMP     NEXTOP
ADDAW   INY                     ;+INC_IP
        LDA     (IP),Y
        STA     SRCL
        INY                     ;+INC_IP
        LDA     (IP),Y
        STA     SRCH
        STY     IPY
        LDY     #$00
        LDA     (SRC),Y
        CLC
        ADC     ESTKL,X
        STA     ESTKL,X
        INY
        LDA     (SRC),Y
        ADC     ESTKH,X
        STA     ESTKH,X
        LDY     IPY
        JMP     NEXTOP
ADDAWX  INY                     ;+INC_IP
        LDA     (IP),Y
        STA     SRCL
        INY                     ;+INC_IP
        LDA     (IP),Y
        STA     SRCH
        STY     IPY
        STA     ALTRDOFF
        LDY     #$00
        LDA     (SRC),Y
        CLC
        ADC     ESTKL,X
        STA     ESTKL,X
        INY
        LDA     (SRC),Y
        ADC     ESTKH,X
        STA     ESTKH,X
        STA     ALTRDON
        LDY     IPY
        JMP     NEXTOP
;*
;* INDEX VALUE FROM ABSOLUTE ADDRESS
;*
IDXAB   INY                     ;+INC_IP
        LDA     (IP),Y
        STA     ESTKH-2,X
        INY                     ;+INC_IP
        LDA     (IP),Y
        STA     ESTKH-1,X
        LDA     (ESTKH-2,X)
        STY     IPY
        LDY     #$00
        ASL
        BCC     +
        INY
        CLC
+       ADC     ESTKL,X
        STA     ESTKL,X
        TYA
        ADC     ESTKH,X
        STA     ESTKH,X
        LDY     IPY
        JMP     NEXTOP
IDXABX  INY                     ;+INC_IP
        LDA     (IP),Y
        STA     ESTKH-2,X
        INY                     ;+INC_IP
        LDA     (IP),Y
        STA     ESTKH-1,X
        STA     ALTRDOFF
        LDA     (ESTKH-2,X)
        STY     IPY
        LDY     #$00
        ASL
        BCC     +
        INY
        CLC
+       ADC     ESTKL,X
        STA     ESTKL,X
        TYA
        ADC     ESTKH,X
        STA     ESTKH,X
        LDY     IPY
        STA     ALTRDON
        JMP     NEXTOP
IDXAW   INY                     ;+INC_IP
        LDA     (IP),Y
        STA     SRCL
        INY                     ;+INC_IP
        LDA     (IP),Y
        STA     SRCH
        STY     IPY
        LDY     #$00
        LDA     (SRC),Y
        ASL
        STA     TMPL
        INY
        LDA     (SRC),Y
        ROL
        STA     TMPH
        LDA     TMPL
        CLC
        ADC     ESTKL,X
        STA     ESTKL,X
        LDA     TMPH
        ADC     ESTKH,X
        STA     ESTKH,X
        LDY     IPY
        JMP     NEXTOP
IDXAWX  INY                     ;+INC_IP
        LDA     (IP),Y
        STA     SRCL
        INY                     ;+INC_IP
        LDA     (IP),Y
        STA     SRCH
        STY     IPY
        STA     ALTRDOFF
        LDY     #$00
        LDA     (SRC),Y
        ASL
        STA     TMPL
        INY
        LDA     (SRC),Y
        ROL
        STA     TMPH
        LDA     TMPL
        CLC
        ADC     ESTKL,X
        STA     ESTKL,X
        LDA     TMPH
        ADC     ESTKH,X
        STA     ESTKH,X
        STA     ALTRDON
        LDY     IPY
        JMP     NEXTOP
;*
;* STORE VALUE TO ADDRESS
;*
SB      LDA     ESTKL,X
        STA     ESTKH-1,X
        LDA     ESTKL+1,X
        STA     (ESTKH-1,X)
        INX
        JMP     DROP
SW      LDA     ESTKL,X
        STA     ESTKH-1,X
        LDA     ESTKL+1,X
        STA     (ESTKH-1,X)
        LDA     ESTKH+1,X
        INC     ESTKH-1,X
        BEQ     +
        STA     (ESTKH-1,X)
        INX
        JMP     DROP
+       INC     ESTKH,X
        STA     (ESTKH-1,X)
;*
;* DROP TOS, TOS-1
;*
DROP2   INX
        JMP     DROP
;*
;* STORE VALUE TO LOCAL FRAME OFFSET
;*
SLB     INY                     ;+INC_IP
        LDA     (IP),Y
        STY     IPY
        TAY
        LDA     ESTKL,X
        STA     (IFP),Y
        LDY     IPY
        BMI     FIXDROP
        JMP     DROP
SLW     INY                     ;+INC_IP
        LDA     (IP),Y
        STY     IPY
        TAY
        LDA     ESTKL,X
        STA     (IFP),Y
        INY
        LDA     ESTKH,X
        STA     (IFP),Y
        LDY     IPY
        BMI     FIXDROP
        JMP     DROP
FIXDROP TYA
        LDY     #$00
        CLC
        ADC     IPL
        STA     IPL
        BCC     +
        INC     IPH
+       JMP     DROP
;*
;* STORE VALUE TO LOCAL FRAME OFFSET WITHOUT POPPING STACK
;*
DLB     INY                     ;+INC_IP
        LDA     (IP),Y
        STY     IPY
        TAY
        LDA     ESTKL,X
        STA     (IFP),Y
        LDA     #$00
        STA     ESTKH,X
        LDY     IPY
        JMP     NEXTOP
DLW     INY                     ;+INC_IP
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
-       TYA                     ; RENORMALIZE IP
        CLC
        ADC     IPL
        STA     IPL
        BCC     +
        INC     IPH
+       LDY     #$FF
SAB     INY                     ;+INC_IP
        BMI     -
        LDA     (IP),Y
        STA     ESTKH-2,X
        INY                     ;+INC_IP
        LDA     (IP),Y
        STA     ESTKH-1,X
        LDA     ESTKL,X
        STA     (ESTKH-2,X)
        JMP     DROP
SAW     INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMPL
        INY                     ;+INC_IP
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
        BMI     +
        JMP     DROP
+       JMP     FIXDROP
;*
;* STORE VALUE TO ABSOLUTE ADDRESS WITHOUT POPPING STACK
;*
DAB     INY                     ;+INC_IP
        LDA     (IP),Y
        STA     ESTKH-2,X
        INY                     ;+INC_IP
        LDA     (IP),Y
        STA     ESTKH-1,X
        LDA     ESTKL,X
        STA     (ESTKH-2,X)
        LDA     #$00
        STA     ESTKH,X
        JMP     NEXTOP
DAW     INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMPL
        INY                     ;+INC_IP
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
ISGE    LDA     ESTKL+1,X
        CMP     ESTKL,X
        LDA     ESTKH+1,X
        SBC     ESTKH,X
        BVS     +
        BPL     ISTRU
        BMI     ISFLS
+
-       BPL     ISFLS
        BMI     ISTRU
ISLE    LDA     ESTKL,X
        CMP     ESTKL+1,X
        LDA     ESTKH,X
        SBC     ESTKH+1,X
        BVS     -
        BPL     ISTRU
        BMI     ISFLS
ISGT    LDA     ESTKL,X
        CMP     ESTKL+1,X
        LDA     ESTKH,X
        SBC     ESTKH+1,X
        BVS     +
        BMI     ISTRU
        BPL     ISFLS
+
-       BMI     ISFLS
        BPL     ISTRU
ISLT    LDA     ESTKL+1,X
        CMP     ESTKL,X
        LDA     ESTKH+1,X
        SBC     ESTKH,X
        BVS     -
        BMI     ISTRU
        BPL     ISFLS
;*
;* BRANCHES
;*
SEL     INX
        TYA                     ; FLATTEN IP
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
        LDA     (IP),Y
        STA     TMPL            ; CASE COUNT
        INC     IPL
        BNE     CASELP
        INC     IPH
CASELP  LDA     ESTKL-1,X
        CMP     (IP),Y
        BEQ     +
        LDA     ESTKH-1,X
        INY
        SBC     (IP),Y
        BMI     CASEEND
-       INY
        INY
        DEC     TMPL
        BEQ     FIXNEXT
        INY
        BNE     CASELP
        INC     IPH
        BNE     CASELP
+       LDA     ESTKH-1,X
        INY
        SBC     (IP),Y
        BEQ     BRNCH
        BPL     -
CASEEND LDA     #$00
        STA     TMPH
        DEC     TMPL
        LDA     TMPL
        ASL                 ; SKIP REMAINING CASES
        ROL     TMPH
        ASL
        ROL     TMPH
;       CLC
        ADC     IPL
        STA     IPL
        LDA     TMPH
        ADC     IPH
        STA     IPH
        INY
        INY
FIXNEXT TYA
        LDY     #$00
        SEC
        ADC     IPL
        STA     IPL
        BCC     +
        INC     IPH
+       JMP     FETCHOP
BRAND   LDA     ESTKL,X
        ORA     ESTKH,X
        BEQ     BRNCH
        INX                     ; DROP LEFT HALF OF AND
        BNE     NOBRNCH
BROR    LDA     ESTKL,X
        ORA     ESTKH,X
        BNE     BRNCH
        INX                     ; DROP LEFT HALF OF OR
        BNE     NOBRNCH
BREQ    INX
        INX
        LDA     ESTKL-2,X
        CMP     ESTKL-1,X
        BNE     NOBRNCH
        LDA     ESTKH-2,X
        CMP     ESTKH-1,X
        BEQ     BRNCH
        BNE     NOBRNCH
BRNE    INX
        INX
        LDA     ESTKL-2,X
        CMP     ESTKL-1,X
        BNE     BRNCH
        LDA     ESTKH-2,X
        CMP     ESTKH-1,X
        BNE     BRNCH
        BEQ     NOBRNCH
BRTRU   INX
        LDA     ESTKH-1,X
        ORA     ESTKL-1,X
        BNE     BRNCH
NOBRNCH INY                     ;+INC_IP
        INY
        BMI     FIXNEXT
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
;*
;* FOR LOOPS PUT TERMINAL VALUE AT ESTK+1 AND CURRENT COUNT ON ESTK
;*
BRGT    LDA     ESTKL+1,X
        CMP     ESTKL,X
        LDA     ESTKH+1,X
        SBC     ESTKH,X
        BVS     +
        BPL     NOBRNCH
        BMI     BRNCH
BRLT    LDA     ESTKL,X
        CMP     ESTKL+1,X
        LDA     ESTKH,X
        SBC     ESTKH+1,X
        BVS     +
        BPL     NOBRNCH
        BMI     BRNCH
+       BMI     NOBRNCH
        BPL     BRNCH
DECBRGE DEC     ESTKL,X
        LDA     ESTKL,X
        CMP     #$FF
        BNE     +
        DEC     ESTKH,X
_BRGE   LDA     ESTKL,X
+       CMP     ESTKL+1,X
        LDA     ESTKH,X
        SBC     ESTKH+1,X
        BVS     +
        BPL     BRNCH
        BMI     NOBRNCH
INCBRLE INC     ESTKL,X
        BNE     _BRLE
        INC     ESTKH,X
_BRLE   LDA     ESTKL+1,X
        CMP     ESTKL,X
        LDA     ESTKH+1,X
        SBC     ESTKH,X
        BVS     +
        BPL     BRNCH
        BMI     NOBRNCH
+       BMI     BRNCH
        BPL     NOBRNCH
SUBBRGE LDA     ESTKL+1,X
        SEC
        SBC     ESTKL,X
        STA     ESTKL+1,X
        LDA     ESTKH+1,X
        SBC     ESTKH,X
        STA     ESTKH+1,X
        INX
        BNE     _BRGE
ADDBRLE LDA     ESTKL,X
        CLC
        ADC     ESTKL+1,X
        STA     ESTKL+1,X
        LDA     ESTKH,X
        ADC     ESTKH+1,X
        STA     ESTKH+1,X
        INX
        BNE     _BRLE
;*
;* CALL INTO ABSOLUTE ADDRESS (NATIVE CODE)
;*
CALL    INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMPL
        INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMPH
        TYA
        SEC
        ADC     IPL
        PHA
        LDA     IPH
        ADC     #$00
        PHA
        JSR     JMPTMP
        PLA
        STA     IPH
        PLA
        STA     IPL
        LDA     #>OPTBL         ; MAKE SURE WE'RE INDEXING THE RIGHT TABLE
        STA     OPPAGE
        LDY     #$00
        JMP     FETCHOP
CALLX   INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMPL
        INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMPH
        STA     ALTRDOFF
        PLP                     ; RESTORE FLAGS
        TYA
        SEC
        ADC     IPL
        PHA
        LDA     IPH
        ADC     #$00
        PHA
        JSR     JMPTMP
        PLA
        STA     IPH
        PLA
        STA     IPL
        LDA     #>OPXTBL        ; MAKE SURE WE'RE INDEXING THE RIGHT TABLE
        STA     OPPAGE
        LDY     #$00
        PHP
        SEI
        STA     ALTRDON
        JMP     FETCHOP
;*
;* INDIRECT CALL TO ADDRESS (NATIVE CODE)
;*
ICAL    LDA     ESTKL,X
        STA     TMPL
        LDA     ESTKH,X
        STA     TMPH
        INX
        TYA
        SEC
        ADC     IPL
        PHA
        LDA     IPH
        ADC     #$00
        PHA
        JSR     JMPTMP
        PLA
        STA     IPH
        PLA
        STA     IPL
        LDA     #>OPTBL         ; MAKE SURE WE'RE INDEXING THE RIGHT TABLE
        STA     OPPAGE
        LDY     #$00
        JMP     FETCHOP
ICALX   STA     ALTRDOFF
        PLP                     ; RESTORE FLAGS
        LDA     ESTKL,X
        STA     TMPL
        LDA     ESTKH,X
        STA     TMPH
        INX
        TYA
        SEC
        ADC     IPL
        PHA
        LDA     IPH
        ADC     #$00
        PHA
        JSR     JMPTMP
        PLA
        STA     IPH
        PLA
        STA     IPL
        LDA     #>OPXTBL        ; MAKE SURE WE'RE INDEXING THE RIGHT TABLE
        STA     OPPAGE
        LDY     #$00
        PHP
        SEI
        STA     ALTRDON
        JMP     FETCHOP
;*
;* JUMP INDIRECT TRHOUGH TMP
;*
;JMPTMP  JMP     (TMP)
;*
;* ENTER FUNCTION WITH FRAME SIZE AND PARAM COUNT
;*
ENTERX  PLA                     ; KEEP PSR ON TOP OF STACK
        TAY
        LDA     IFPH
        PHA                     ; SAVE ON STACK FOR LEAVE
        LDA     IFPL
        PHA
        TYA
        PHA
        LDA     PPL             ; ALLOCATE FRAME
        LDY     #$01
        SEC
        SBC     (IP),Y
        STA     PPL
        STA     IFPL
        LDA     PPH
        SBC     #$00
        STA     PPH
        STA     IFPH
        INY
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
ENTER   LDA     IFPH
        PHA                     ; SAVE ON STACK FOR LEAVE
        LDA     IFPL
        PHA
        LDA     PPL             ; ALLOCATE FRAME
        INY
        SEC
        SBC     (IP),Y
        STA     PPL
        STA     IFPL
        LDA     PPH
        SBC     #$00
        STA     PPH
        STA     IFPH
        INY
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
LEAVEX  LDA     IFPL
        INY                     ;+INC_IP
        CLC
        ADC     (IP),Y
        STA     PPL
        LDA     IFPH
        ADC     #$00
        STA     PPH
        STA     ALTRDOFF
        PLP
        PLA                     ; RESTORE PREVIOUS FRAME
        STA     IFPL
        PLA
        STA     IFPH
        RTS
RETX    STA     ALTRDOFF
        PLP
        RTS
LEAVE   LDA     IFPL
        INY                     ;+INC_IP
        CLC
        ADC     (IP),Y
        STA     PPL
        LDA     IFPH
        ADC     #$00
        STA     PPH
        PLA                     ; RESTORE PREVIOUS FRAME
        STA     IFPL
        PLA
        STA     IFPH
RET     RTS
;*
;* RETURN TO NATIVE CODE
;*
NATV    TYA                     ; FLATTEN IP
        SEC
        ADC     IPL
        STA     IPL
        BCS     +
        JMP     (IP)
+       INC     IPH
        JMP     (IP)
VMEND   =       *
}
;***************************************
;*                                     *
;* 65C02 OPS TO OVERWRITE STANDARD OPS *
;*                                     *
;***************************************
C02OPS  LDA     #<DINTRP
        LDX     #>DINTRP
        LDY     #(CDINTRPEND-CDINTRP)
        JSR     OPCPY
CDINTRP PLY
        PLA
        INY
        BNE     +
        INC
+       STY     IPL
        STA     IPH
        LDY     #$00
        LDA     #>OPTBL
        STA     OPPAGE
        JMP     FETCHOP
CDINTRPEND
;
        LDA     #<CN
        LDX     #>CN
        LDY     #(CCNEND-CCN)
        JSR     OPCPY
CCN     DEX
        LSR
        STA     ESTKL,X
        STZ     ESTKH,X
        JMP     NEXTOP
CCNEND
;
        LDA     #<CB
        LDX     #>CB
        LDY     #(CCBEND-CCB)
        JSR     OPCPY
CCB     DEX
        STZ     ESTKH,X
        INY
        LDA     (IP),Y
        STA     ESTKL,X
        JMP     NEXTOP
CCBEND
;
        LDA     #<CS
        LDX     #>CS
        LDY     #(CCSEND-CCS)
        JSR     OPCPY
CCS     DEX
        ;INY                     ;+INC_IP
        TYA                     ; NORMALIZE IP AND SAVE STRING ADDR ON ESTK
        SEC
        ADC     IPL
        STA     IPL
        STA     ESTKL,X
        LDA     #$00
        ADC     IPH
        STA     IPH
        STA     ESTKH,X
        LDA     (IP)
        TAY
        JMP     NEXTOP
CCSEND
;
        LDA     #<SHL
        LDX     #>SHL
        LDY     #(CSHLEND-CSHL)
        JSR     OPCPY
CSHL    STY     IPY
        LDA     ESTKL,X
        CMP     #$08
        BCC     +
        LDY     ESTKL+1,X
        STY     ESTKH+1,X
        STZ     ESTKL+1,X
        SBC     #$08
+       TAY
        BEQ     +
        LDA     ESTKL+1,X
-       ASL
        ROL     ESTKH+1,X
        DEY
        BNE     -
        STA     ESTKL+1,X
+       LDY     IPY
        JMP     DROP
CSHLEND
;
        LDA     #<LB
        LDX     #>LB
        LDY     #(CLBEND-CLB)
        JSR     OPCPY
CLB     LDA     ESTKL,X
        STA     ESTKH-1,X
        LDA     (ESTKH-1,X)
        STA     ESTKL,X
        STZ     ESTKH,X
        JMP     NEXTOP
CLBEND
;
        LDA     #<LBX
        LDX     #>LBX
        LDY     #(CLBXEND-CLBX)
        JSR     OPCPY
CLBX    LDA     ESTKL,X
        STA     ESTKH-1,X
        STA     ALTRDOFF
        LDA     (ESTKH-1,X)
        STA     ESTKL,X
        STZ     ESTKH,X
        STA     ALTRDON
        JMP     NEXTOP
CLBXEND
;
        LDA     #<LLB
        LDX     #>LLB
        LDY     #(CLLBEND-CLLB)
        JSR     OPCPY
CLLB    INY                     ;+INC_IP
        LDA     (IP),Y
        STY     IPY
        TAY
        DEX
        LDA     (IFP),Y
        STA     ESTKL,X
        STZ     ESTKH,X
        LDY     IPY
        JMP     NEXTOP
CLLBEND
;
        LDA     #<LLBX
        LDX     #>LLBX
        LDY     #(CLLBXEND-CLLBX)
        JSR     OPCPY
CLLBX   INY                     ;+INC_IP
        LDA     (IP),Y
        STY     IPY
        TAY
        DEX
        STA     ALTRDOFF
        LDA     (IFP),Y
        STA     ESTKL,X
        STZ     ESTKH,X
        STA     ALTRDON
        LDY     IPY
        JMP     NEXTOP
CLLBXEND
;
        LDA     #<LAB
        LDX     #>LAB
        LDY     #(CLABEND-CLAB)
        JSR     OPCPY
CLAB    INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMPL
        INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMPH
        LDA     (TMP)
        DEX
        STA     ESTKL,X
        STZ     ESTKH,X
        JMP     NEXTOP
CLABEND
;
        LDA     #<LAW
        LDX     #>LAW
        LDY     #(CLAWEND-CLAW)
        JSR     OPCPY
CLAW    INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMPL
        INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMPH
        STY     IPY
        LDA     (TMP)
        DEX
        STA     ESTKL,X
        LDY     #$01
        LDA     (TMP),Y
        STA     ESTKH,X
        LDY     IPY
        JMP     NEXTOP
CLAWEND
;
        LDA     #<LABX
        LDX     #>LABX
        LDY     #(CLABXEND-CLABX)
        JSR     OPCPY
CLABX   INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMPL
        INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMPH
        STA     ALTRDOFF
        LDA     (TMP)
        DEX
        STA     ESTKL,X
        STZ     ESTKH,X
        STA     ALTRDON
        JMP     NEXTOP
CLABXEND
;
        LDA     #<LAWX
        LDX     #>LAWX
        LDY     #(CLAWXEND-CLAWX)
        JSR     OPCPY
CLAWX   INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMPL
        INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMPH
        STY     IPY
        STA     ALTRDOFF
        LDA     (TMP)
        DEX
        STA     ESTKL,X
        LDY     #$01
        LDA     (TMP),Y
        STA     ESTKH,X
        STA     ALTRDON
        LDY     IPY
        JMP     NEXTOP
CLAWXEND
;
        LDA     #<SAW
        LDX     #>SAW
        LDY     #(CSAWEND-CSAW)
        JSR     OPCPY
CSAW    INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMPL
        INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMPH
        STY     IPY
        LDA     ESTKL,X
        STA     (TMP)
        LDY     #$01
        LDA     ESTKH,X
        STA     (TMP),Y
        LDY     IPY
        BMI     +
        JMP     DROP
+       JMP     FIXDROP
CSAWEND
;
        LDA     #<DAW
        LDX     #>DAW
        LDY     #(CDAWEND-CDAW)
        JSR     OPCPY
CDAW    INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMPL
        INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMPH
        STY     IPY
        LDA     ESTKL,X
        STA     (TMP)
        LDY     #$01
        LDA     ESTKH,X
        STA     (TMP),Y
        LDY     IPY
        JMP     NEXTOP
CDAWEND
;
        LDA     #<DAB
        LDX     #>DAB
        LDY     #(CDABEND-CDAB)
        JSR     OPCPY
CDAB    INY                     ;+INC_IP
        LDA     (IP),Y
        STA     ESTKH-2,X
        INY                     ;+INC_IP
        LDA     (IP),Y
        STA     ESTKH-1,X
        LDA     ESTKL,X
        STA     (ESTKH-2,X)
        STZ     ESTKH,X
        JMP     NEXTOP
CDABEND
;
        LDA     #<DLB
        LDX     #>DLB
        LDY     #(CDLBEND-CDLB)
        JSR     OPCPY
CDLB    INY                     ;+INC_IP
        LDA     (IP),Y
        STY     IPY
        TAY
        LDA     ESTKL,X
        STA     (IFP),Y
        STZ     ESTKH,X
        LDY     IPY
        JMP     NEXTOP
CDLBEND
;
        LDA     #<ISFLS
        LDX     #>ISFLS
        LDY     #(CISFLSEND-CISFLS)
        JSR     OPCPY
CISFLS  STZ     ESTKL+1,X
        STZ     ESTKH+1,X
        JMP     DROP
CISFLSEND
;
        LDA     #<BRNCH
        LDX     #>BRNCH
        LDY     #(CBRNCHEND-CBRNCH)
        JSR     OPCPY
CBRNCH  TYA                     ; FLATTEN IP
        SEC
        ADC     IPL
        STA     TMPL
        LDA     #$00
        ADC     IPH
        STA     TMPH            ; ADD BRANCH OFFSET
        LDA     (TMP)
        ;CLC                    ; BETTER NOT CARRY OUT OF IP+Y
        ADC     TMPL
        STA     IPL
        LDY     #$01
        LDA     (TMP),Y
        ADC     TMPH
        STA     IPH
        DEY
        JMP     FETCHOP
CBRNCHEND
;
        LDA     #<ENTERX
        LDX     #>ENTERX
        LDY     #(CENTERXEND-CENTERX)
        JSR     OPCPY
CENTERX PLY                     ; KEEP PSR ON TOP OF STACK
        LDA     IFPH
        PHA                     ; SAVE ON STACK FOR LEAVE
        LDA     IFPL
        PHA
        PHY
        LDA     PPL             ; ALLOCATE FRAME
        LDY     #$01
        SEC
        SBC     (IP),Y
        STA     PPL
        STA     IFPL
        LDA     PPH
        SBC     #$00
        STA     PPH
        STA     IFPH
        INY
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
CENTERXEND
;
        RTS
;*
;* COPY OP TO VM
;*
OPCPY   STA     DST
        STX     DST+1
        PLA
        STA     SRC
        PLA
        STA     SRC+1
        TYA
        CLC
        ADC     SRC
        TAX
        LDA     #$00
        ADC     SRC+1
        PHA
        PHX
        INC     SRC
        BNE     +
        INC     SRC+1
+       DEY
-       LDA     (SRC),Y
        STA     (DST),Y
        DEY
        BPL     -
        RTS

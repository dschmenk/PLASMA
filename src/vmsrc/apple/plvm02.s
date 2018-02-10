;**********************************************************
;*
;*          APPLE ][ 64K/128K PLASMA INTERPRETER
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
PSR     =       TMP+2
DVSIGN  =       PSR+1
DROP    =       $EF
NEXTOP  =       $F0
FETCHOP =       NEXTOP+1
IP      =       FETCHOP+1
IPL     =       IP
IPH     =       IPL+1
OPIDX   =       FETCHOP+6
OPPAGE  =       OPIDX+1
STRBUF  =       $0280
INTERP  =       $03D0
;******************************
;*                            *
;* INTERPRETER INITIALIZATION *
;*                            *
;******************************
*       =       $2000
        LDX     #$FE
        TXS
        LDX    #$00
        STX    $01FF
;*
;* DISCONNECT /RAM
;*
        ;SEI                    ; DISABLE /RAM
        LDA     MACHID
        AND     #$30
        CMP     #$30
        BNE     RAMDONE
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
-       LDA     (SRC),Y         ; COPY VM+CMD INTO LANGUAGE CARD
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
        STY     SRCL
        LDA     #$D1
        STA     SRCH
-       LDA     (SRC),Y
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
;* SET 64K ENTER/LEAVE (NO NEED FOR STRING POOL)
;*
+       LDA     MACHID
        AND     #$30
        CMP     #$30
        BEQ     +
        LDA     #<ENTER64
        STA     OPTBL+$58
        LDA     #>ENTER64
        STA     OPTBL+$59
        LDA     #<LEAVE64
        STA     OPTBL+$5A
        LDA     #>LEAVE64
        STA     OPTBL+$5B
;*
;* SAVE DEFAULT COMMAND INTERPRETER PATH IN LC
;*
+       JSR     PRODOS          ; GET PREFIX
        !BYTE   $C7
        !WORD   GETPFXPARMS
        LDY     STRBUF          ; APPEND "CMD"
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
OPTBL   !WORD   ZERO,ADD,SUB,MUL,DIV,MOD,INCR,DECR              ; 00 02 04 06 08 0A 0C 0E
        !WORD   NEG,COMP,BAND,IOR,XOR,SHL,SHR,IDXW              ; 10 12 14 16 18 1A 1C 1E
        !WORD   LNOT,LOR,LAND,LA,LLA,CB,CW,CS                   ; 20 22 24 26 28 2A 2C 2E
        !WORD   DROP,DUP,NEXTOP,DIVMOD,BRGT,BRLT,BREQ,BRNE      ; 30 32 34 36 38 3A 3C 3E
        !WORD   ISEQ,ISNE,ISGT,ISLT,ISGE,ISLE,BRFLS,BRTRU       ; 40 42 44 46 48 4A 4C 4E
        !WORD   BRNCH,IBRNCH,CALL,ICAL,ENTER,LEAVE,RET,CFFB   ; 50 52 54 56 58 5A 5C 5E
        !WORD   LB,LW,LLB,LLW,LAB,LAW,DLB,DLW                   ; 60 62 64 66 68 6A 6C 6E
        !WORD   SB,SW,SLB,SLW,SAB,SAW,DAB,DAW                   ; 70 72 74 76 78 7A 7C 7E
;*
;* ENTER INTO BYTECODE INTERPRETER
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
IINTRP  PLA
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
+       LDA     #>OPTBL
        STA     OPPAGE
        JMP     FETCHOP
IINTRPX PHP
        PLA
        STA     PSR
        SEI
        PLA
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
; DEACTIVATE 80 COL CARDS
;
        BIT     ROMEN
        LDY     #4
-       LDA     DISABLE80,Y
        ORA     #$80
        JSR     $FDED
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
; INIT VM ENVIRONMENT STACK POINTERS
;
;       LDA     #$00
        STA     $01FF           ; CLEAR CMDLINE BUFF
        STA     PPL             ; INIT FRAME POINTER
        STA     IFPL
        LDA     #$BF
        STA     PPH
        STA     IFPH
        LDX     #$FE            ; INIT STACK POINTER (YES, $FE. SEE GETS)
        TXS
        LDX     #ESTKSZ/2       ; INIT EVAL STACK INDEX
;
; CHANGE CMD STRING TO SYSPATH STRING
;
        LDA     STRBUF
        SEC
        SBC     #$03
        STA     STRBUF
        JMP     $2000           ; JUMP TO LOADED SYSTEM COMMAND
;
; PRINT FAIL MESSAGE, WAIT FOR KEYPRESS, AND REBOOT
;
FAIL    INC     $3F4            ; INVALIDATE POWER-UP BYTE
        LDY     #33
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
FAILMSG !TEXT   "...TESER OT YEK YNA .DMC GNISSIM"
PAGE0    =      *
;******************************
;*                            *
;* INTERP BYTECODE INNER LOOP *
;*                            *
;******************************
        !PSEUDOPC       DROP {
        INX                     ; DROP @ $EF
        INY                     ; NEXTOP @ $F0
        LDA     $FFFF,Y         ; FETCHOP @ $F3, IP MAPS OVER $FFFF @ $F4
        STA     OPIDX
        JMP     (OPTBL)         ; OPIDX AND OPPAGE MAP OVER OPTBL
}
PAGE3   =       *
;*
;* PAGE 3 VECTORS INTO INTERPRETER
;*
        !PSEUDOPC       $03D0 {
        BIT     LCRDEN+LCBNK2   ; $03D0 - DIRECT INTERP ENTRY
        JMP     DINTRP
        BIT     LCRDEN+LCBNK2   ; $03D6 - INDIRECT INTERP ENTRY
        JMP     IINTRP
        BIT     LCRDEN+LCBNK2   ; $03DC - INDIRECT INTERPX ENTRY
        JMP     IINTRPX
}
DEFCMD  !FILL   28
ENDBYE  =       *
}
LCDEFCMD =      *-28            ; DEFCMD IN LC MEMORY
;*****************
;*               *
;* OPXCODE TABLE *
;*               *
;*****************
        !ALIGN  255,0
OPXTBL  !WORD   ZERO,ADD,SUB,MUL,DIV,MOD,INCR,DECR              ; 00 02 04 06 08 0A 0C 0E
        !WORD   NEG,COMP,BAND,IOR,XOR,SHL,SHR,IDXW              ; 10 12 14 16 18 1A 1C 1E
        !WORD   LNOT,LOR,LAND,LA,LLA,CB,CW,CSX                  ; 20 22 24 26 28 2A 2C 2E
        !WORD   DROP,DUP,NEXTOP,DIVMOD,BRGT,BRLT,BREQ,BRNE      ; 30 32 34 36 38 3A 3C 3E
        !WORD   ISEQ,ISNE,ISGT,ISLT,ISGE,ISLE,BRFLS,BRTRU       ; 40 42 44 46 48 4A 4C 4E
        !WORD   BRNCH,IBRNCH,CALLX,ICALX,ENTER,LEAVEX,RETX,CFFB; 50 52 54 56 58 5A 5C 5E
        !WORD   LBX,LWX,LLBX,LLWX,LABX,LAWX,DLB,DLW             ; 60 62 64 66 68 6A 6C 6E
        !WORD   SB,SW,SLB,SLW,SAB,SAW,DAB,DAW                   ; 70 72 74 76 78 7A 7C 7E
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
        LDA     DVSIGN          ; REMAINDER IS SIGN OF DIVIDEND
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
;* LOGICAL AND
;*
LAND    LDA     ESTKL+1,X
        ORA     ESTKH+1,X
        BEQ     ++
        LDA     ESTKL,X
        ORA     ESTKH,X
        BEQ     +
        LDA     #$FF
+       STA     ESTKL+1,X
        STA     ESTKH+1,X
++      JMP     DROP
;*
;* LOGICAL OR
;*
LOR     LDA     ESTKL,X
        ORA     ESTKH,X
        ORA     ESTKL+1,X
        ORA     ESTKH+1,X
        BEQ     +
        LDA     #$FF
        STA     ESTKL+1,X
        STA     ESTKH+1,X
+       JMP     DROP
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
;* LOGICAL NOT
;*
LNOT    LDA     ESTKL,X
        ORA     ESTKH,X
        BNE     +
        LDA     #$FF
        STA     ESTKL,X
        STA     ESTKH,X
        JMP     NEXTOP
;*
;* CONSTANT
;*
ZERO    DEX
+       LDA     #$00
        STA     ESTKL,X
        STA     ESTKH,X
        JMP     NEXTOP
CFFB    DEX
        LDA     #$FF
        STA     ESTKH,X
        INY                     ;+INC_IP
        LDA     (IP),Y
        STA     ESTKL,X
        JMP     NEXTOP
CB      DEX
        LDA     #$00
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
;
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
;
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
;
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
;
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
        INX
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
        JMP     FIXDROP
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
        BVS     +
        BPL     ISTRU
        BMI     ISFLS
+       BPL     ISFLS
        BMI     ISTRU
;
ISGT    LDA     ESTKL,X
        CMP     ESTKL+1,X
        LDA     ESTKH,X
        SBC     ESTKH+1,X
        BVS     +
        BMI     ISTRU
        BPL     ISFLS
+       BMI     ISFLS
        BPL     ISTRU
;
ISLE    LDA     ESTKL,X
        CMP     ESTKL+1,X
        LDA     ESTKH,X
        SBC     ESTKH+1,X
        BVS     +
        BPL     ISTRU
        BMI     ISFLS
+       BPL     ISFLS
        BMI     ISTRU
;
ISLT    LDA     ESTKL+1,X
        CMP     ESTKL,X
        LDA     ESTKH+1,X
        SBC     ESTKH,X
        BVS     +
        BMI     ISTRU
        BPL     ISFLS
+       BMI     ISFLS
        BPL     ISTRU
;*
;* BRANCHES
;*
BRTRU   INX
        LDA     ESTKH-1,X
        ORA     ESTKL-1,X
        BNE     BRNCH
NOBRNCH INY                     ;+INC_IP
        INY
        BMI     FIXNEXT
        JMP     NEXTOP
FIXNEXT TYA
        LDY     #$00
        CLC
        ADC     IPL
        STA     IPL
        BCC     +
        INC     IPH
+       JMP     NEXTOP
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
        BVS     +
        BPL     NOBRNCH
        BMI     BRNCH
+       BPL     BRNCH
        BMI     NOBRNCH
BRLT    INX
        LDA     ESTKL,X
        CMP     ESTKL-1,X
        LDA     ESTKH,X
        SBC     ESTKH-1,X
        BVS     +
        BPL     NOBRNCH
        BMI     BRNCH
+       BPL     BRNCH
        BMI     NOBRNCH
IBRNCH  TYA                     ; FLATTEN IP
        CLC
        ADC     IPL
        STA     TMPL
        LDA     #$00
        TAY
        ADC     IPH
        STA     TMPH            ; ADD BRANCH OFFSET
        LDA     TMPL
        ;CLC                    ; BETTER NOT CARRY OUT OF IP+Y
        ADC     ESTKL,X
        STA     IPL
        LDA     TMPH
        ADC     ESTKH,X
        STA     IPH
        JMP     DROP
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
        CLC
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
        LDY     #$01
        JMP     FETCHOP
;
CALLX   INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMPL
        INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMPH
        TYA
        CLC
        ADC     IPL
        PHA
        LDA     IPH
        ADC     #$00
        PHA
        STA     ALTRDOFF
        LDA     PSR
        PHA
        PLP
        JSR     JMPTMP
        PHP
        PLA
        STA     PSR
        SEI
        STA     ALTRDON
        PLA
        STA     IPH
        PLA
        STA     IPL
        LDA     #>OPXTBL        ; MAKE SURE WE'RE INDEXING THE RIGHT TABLE
        STA     OPPAGE
        LDY     #$01
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
        CLC
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
        LDY     #$01
        JMP     FETCHOP
;
ICALX   LDA     ESTKL,X
        STA     TMPL
        LDA     ESTKH,X
        STA     TMPH
        INX
        TYA
        CLC
        ADC     IPL
        PHA
        LDA     IPH
        ADC     #$00
        PHA
        STA     ALTRDOFF
        LDA     PSR
        PHA
        PLP
        JSR     JMPTMP
        PHP
        PLA
        STA     PSR
        STA     ALTRDON
        PLA
        STA     IPH
        PLA
        STA     IPL
        LDA     #>OPXTBL        ; MAKE SURE WE'RE INDEXING THE RIGHT TABLE
        STA     OPPAGE
        LDY     #$01
        JMP     FETCHOP
;*
;* JUMP INDIRECT TRHOUGH TMP
;*
;JMPTMP  JMP     (TMP)
;*
;* ENTER FUNCTION WITH FRAME SIZE AND PARAM COUNT
;*
ENTER64 INY
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
ENTER   LDA     IFPH
        PHA                     ; SAVE ON STACK FOR LEAVE
        LDA     IFPL
        PHA
        INY
        LDA     (IP),Y
        EOR     #$FF            ; ALLOCATE FRAME
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
LEAVEX  INY                     ;+INC_IP
        LDA     (IP),Y
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
RETX    STA     ALTRDOFF
        LDA     PSR
        PHA
        PLP
        RTS
LEAVE64 INY                     ;+INC_IP
        LDA     (IP),Y
        CLC
        ADC     IFPL
        STA     IFPL
        BCS     +
        RTS
+       INC     IFPH
        RTS
LEAVE   INY                     ;+INC_IP
        LDA     (IP),Y
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
RET     RTS
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
        LDA     #<ZERO
        LDX     #>ZERO
        LDY     #(CZEROEND-CZERO)
        JSR     OPCPY
CZERO   DEX
        STZ     ESTKL,X
        STZ     ESTKH,X
        JMP     NEXTOP
CZEROEND
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
        STA     ESTKH-2,X
        INY                     ;+INC_IP
        LDA     (IP),Y
        STA     ESTKH-1,X
        LDA     (ESTKH-2,X)
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
        STA     ESTKH-2,X
        INY                     ;+INC_IP
        LDA     (IP),Y
        STA     ESTKH-1,X
        STA     ALTRDOFF
        LDA     (ESTKH-2,X)
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
+
-       LDA     (SRC),Y
        STA     (DST),Y
        DEY
        BPL     -
        RTS

;**********************************************************
;*
;*          APPLE ][ 65802/65816 PLASMA INTERPRETER
;*
;*              SYSTEM ROUTINES AND LOCATIONS
;*
;**********************************************************
        !CPU    65816
DEBUG   =       0
;*
;* THE DEFAULT CPU MODE FOR EXECUTING OPCODES IS:
;*   16 BIT A/M
;*    8 BIT X/Y
;*
;* THE EVALUATION STACK WILL BE THE HARDWARE STACK UNTIL
;* A CALL IS MADE. THE 16 BIT PARAMETERS WILL BE COPIED
;* TO THE ZERO PAGE INTERLEAVED EVALUATION STACK.
;*
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
HWSP    =       PSR+1
DROP    =       $EF
NEXTOP  =       DROP+1
FETCHOP =       NEXTOP+1
IP      =       FETCHOP+1
IPL     =       IP
IPH     =       IPL+1
OPIDX   =       FETCHOP+4
OPPAGE  =       OPIDX+1
;
; BUFFER ADDRESSES
;
STRBUF  =       $0300
JITMOD  =       $02F0
INTERP  =       $03D0
JITCOMP =       $03E2
JITCODE =       $03E4
;*
;* HARDWARE STACK OFFSETS
;*
TOS     =       $01             ; TOS
NOS     =       $03             ; TOS-1
;*
;* ACCUM/MEM SIZE MACROS
;*
        !MACRO  ACCMEM8 {
        SEP     #$20            ; 8 BIT A/M
        !AS
        }
        !MACRO  ACCMEM16 {
        REP     #$20            ; 16 BIT A/M
        !AL
        }
        !MACRO  INDEX8 {
        SEP     #$10            ; 8 BIT X/Y
        !AS
        }
        !MACRO  INDEX16 {
        REP     #$10            ; 16 BIT X/Y
        !AL
        }
;******************************
;*                            *
;* INTERPRETER INITIALIZATION *
;*                            *
;******************************
*       =      $2000
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
        LDY     #ANYKEY-BADCPU
        BNE     +++
NEEDAUX !TEXT   "128K MEMORY REQUIRED.", 13, 0
;*
;* CHECK CPU TYPE
;*
++      CLC
        XCE                     ; SWITCH TO NATIVE MODE
        BCS     ++
        LDY     #$00            ; NOPE, NOT 65802/65816
-       LDA     BADCPU,Y
        BEQ     +
        ORA     #$80
        JSR     $FDED
        INY
+++     BNE     -
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
BADCPU  !TEXT   "65C802/65C816 CPU REQUIRED.", 13
ANYKEY  !TEXT   "PRESS ANY KEY...", 0
++      XCE                     ; SWITCH BACK TO EMULATED MODE

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
;* SAVE DEFAULT COMMAND INTERPRETER PATH IN LC
;*
        JSR     PRODOS          ; GET PREFIX
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
;* ENTER INTO BYTECODE INTERPRETER - IMMEDIATELY SWITCH TO NATIVE
;*
        !AS
DINTRP  PHP
        PLA
        STA     PSR
        SEI
        CLC                     ; SWITCH TO NATIVE MODE
        XCE
        +ACCMEM16               ; 16 BIT A/M
        PLA
        INC
        STA     IP
        STX     ESP
        TSX
        STX     HWSP
        LDX     #>OPTBL
!IF DEBUG {
        BRA     SETDBG
} ELSE {
        STX     OPPAGE
        LDY     #$00
        JMP     FETCHOP
}
        !AS
IINTRPX PHP
        PLA
        STA     PSR
        SEI
        CLC                     ; SWITCH TO NATIVE MODE
        XCE
        +ACCMEM16               ; 16 BIT A/M
        LDY     #$01
        LDA     (TOS,S),Y
        DEY
        STA     IP
        PLA
        STX     ESP
        TSX
        STX     HWSP
        STX     ALTRDON
        LDX     #>OPXTBL
!IF DEBUG {
SETDBG  LDY     LCRWEN+LCBNK2
        LDY     LCRWEN+LCBNK2
        STX     DBG_OP+2
        LDY     LCRDEN+LCBNK2
        LDX     #>DBGTBL
        LDY     #$00
}
        STX     OPPAGE
        JMP     FETCHOP
;************************************************************
;*                                                          *
;* 'BYE' PROCESSING - COPIED TO $1000 ON PRODOS BYE COMMAND *
;*                                                          *
;************************************************************
        !AS
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
!IF     DEBUG {
        LDA     #20             ; SET TEXT WINDOW ABOVE DEBUG OUTPUT
        STA     $23
}
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
        LDA     #$AF            ; FRAME POINTER AT $AF00, BELOW JIT BUFFER
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
JITDCI  !BYTE       'J'|$80,'I'|$80,'T'|$80,'1'|$80,'6'
FAILMSG !TEXT   ".DMC GNISSIM"
PAGE0    =      *
;******************************
;*                            *
;* INTERP BYTECODE INNER LOOP *
;*                            *
;******************************
        !PSEUDOPC       DROP {
        PLA                     ; DROP @ $EF
        INY                     ; NEXTOP @ $F0
        LDX     $FFFF,Y         ; FETCHOP @ $F1, IP MAPS OVER $FFFF @ $F2
        JMP     (OPTBL,X)       ; OPIDX AND OPPAGE MAP OVER OPTBL
}
PAGE3   =       *
;*
;* PAGE 3 VECTORS INTO INTERPRETER
;*
        !PSEUDOPC       $03D0 {
        BIT     LCRDEN+LCBNK2   ; $03D0 - DIRECT INTERP ENTRY
        JMP     DINTRP
        BIT     LCRDEN+LCBNK2   ; $03D6 - JIT INDIRECT INTERPX ENTRY
        JMP     JITINTRPX
        BIT     LCRDEN+LCBNK2   ; $03DC - INDIRECT INTERPX ENTRY
        JMP     IINTRPX
}
!IF     DEBUG {
DEFCMD  !FILL   23
} ELSE {
DEFCMD  !FILL   28
}
ENDBYE  =       *
}
!IF     DEBUG {
LCDEFCMD =      *-23 ;*-28      ; DEFCMD IN LC MEMORY
} ELSE {
LCDEFCMD =      *-28            ; DEFCMD IN LC MEMORY
}
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
        !WORD   BRNCH,SEL,CALLX,ICALX,ENTER,LEAVEX,RETX,CFFB            ; 50 52 54 56 58 5A 5C 5E
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
JITINTRPX PHP
        PLA
        STA     PSR
        SEI
        PLA
        SEC
        SBC     #$02            ; POINT TO DEF ENTRY
        STA     TMPL
        PLA
        SBC     #$00
        STA     TMPH
        LDY     #$05
        LDA     (TMP),Y         ; DEC JIT COUNT
        DEC
        STA     (TMP),Y
        BEQ     RUNJIT
        CLC                     ; SWITCH TO NATIVE MODE
        XCE
        +ACCMEM16               ; 16 BIT A/M
        LDY     #$3             ; INTERP BYTECODE AS USUAL
        LDA     (TMP),Y
        STA     IP
        STX     ESP
        TSX
        STX     HWSP
        STX     ALTRDON
        LDX     #>OPXTBL
!IF DEBUG {
        LDY     LCRWEN+LCBNK2
        LDY     LCRWEN+LCBNK2
        STX     DBG_OP+2
        LDY     LCRDEN+LCBNK2
        LDX     #>DBGTBL
}
        STX     OPPAGE
        LDY     #$00
        JMP     FETCHOP
;
        !AS
RUNJIT  DEX                     ; ADD PARAMETER TO DEF ENTRY
        LDA     TMPL
        PHA                     ; AND SAVE IT FOR LATER
        STA     ESTKL,X
        LDA     TMPH
        PHA
        STA     ESTKH,X
        CLC                     ; SWITCH TO NATIVE MODE
        XCE
        +ACCMEM16               ; 16 BIT A/M
        LDA     JITCOMP
        STA     SRC
        LDY     #$03
        LDA     (SRC),Y
        STA     IP
        STX     ESP
        TSX
        DEX                     ; TAKE INTO ACCOUNT JSR BELOW
        DEX
        STX     HWSP
        STX     ALTRDON
        LDX     #>OPXTBL
!IF DEBUG {
        LDY     LCRWEN+LCBNK2
        LDY     LCRWEN+LCBNK2
        STX     DBG_OP+2
        LDY     LCRDEN+LCBNK2
        LDX     #>DBGTBL
}
        STX     OPPAGE
        LDY     #$00
        JSR     FETCHOP         ; CALL JIT COMPILER
        !AS                     ; RETURN IN EMULATION MODE
        PLA
        STA     TMPH
        PLA
        STA     TMPL
        JMP     (TMP)           ; RE-CALL ORIGINAL DEF ENTRY
;*********************************************************************
;*
;*      CODE BELOW HERE DEFAULTS TO NATIVE 16 BIT A/M, 8 BIT X,Y
;*
;*********************************************************************
        !AL
;*
;* ADD TOS TO TOS-1
;*
ADD     PLA
        CLC
        ADC     TOS,S
        STA     TOS,S
        JMP     NEXTOP
;*
;* SUB TOS FROM TOS-1(NOS)
;*
SUB     LDA     NOS,S
        SEC
        SBC     TOS,S
        STA     NOS,S
        JMP     DROP
;*
;* SHIFT TOS LEFT BY 1, ADD TO TOS-1
;*
IDXW    PLA
        ASL
        CLC
        ADC     TOS,S
        STA     TOS,S
        JMP     NEXTOP
;*
;* MUL TOS-1 BY TOS
;*
MUL     LDX     #$10
        LDA     NOS,S
        EOR     #$FFFF
        STA     TMP
        LDA     #$0000
_MULLP  ASL
        ASL     TMP             ; MULTPLR
        BCS     +
        ADC     TOS,S           ; MULTPLD
+       DEX
        BNE     _MULLP
        STA     NOS,S           ; PROD
        JMP     DROP
;*
;* INTERNAL DIVIDE ALGORITHM
;*
_DIV    STY     IPY
        LDY     #$11            ; #BITS+1
        LDX     #$00
        LDA     NOS+2,S         ; WE JSR'ED HERE SO OFFSET ACCORDINGLY
        BPL     +
        LDX     #$81
        EOR     #$FFFF
        INC
+       STA     TMP             ; NOS,S
        BEQ     _DIVEX
        LDA     TOS+2,S
        BPL     +
        INX
        EOR     #$FFFF
        INC
        STA     TOS+2,S
+       LDA     TMP
_DIV1   ASL                     ; DVDND
        DEY
        BCC     _DIV1
        STA     TMP             ; NOS,S           ; DVDND
        LDA     #$0000          ; REMNDR
_DIVLP  ROL                     ; REMNDR
        CMP     TOS+2,S         ; DVSR
        BCC     +
        SBC     TOS+2,S         ; DVSR
        SEC
+       ROL     TMP             ; NOS,S           ; DVDND
        DEY
        BNE     _DIVLP
_DIVEX  LDY     IPY
        RTS
;*
;* DIV TOS-1 BY TOS
;*
DIV     JSR     _DIV
        LDA     TMP
        STA     NOS,S
        PLA
        TXA                     ; DIVSGN
        LSR                     ; SIGN(RESULT) = (SIGN(DIVIDEND) + SIGN(DIVISOR)) & 1
        BCS     NEG
        JMP     NEXTOP
;*
;* MOD TOS-1 BY TOS
;*
MOD     JSR     _DIV
        STA     NOS,S           ; REMNDR
        PLA
        CPX     #$80            ; DIVSGN
        BCS     NEG             ; REMAINDER IS SIGN OF DIVIDEND
        JMP     NEXTOP
;*
;* DIVMOD TOS-1 BY TOS - !!!HACK!!! MUST COPY ESTK TO HW STACK
;*
DIVMOD  +ACCMEM8
        LDX     ESP
        LDA     ESTKH+1,X
        PHA
        LDA     ESTKL+1,X
        PHA
        LDA     ESTKH,X
        PHA
        LDA     ESTKL,X
        PHA
        +ACCMEM16
        JSR     _DIV
        CPX     #$80            ; DIVSGN
        BCC     +               ; REMAINDER IS SIGN OF DIVIDEND
        EOR     #$FFFF
        INC
+       STA     TOS,S           ; REMNDR
        TXA                     ; DIVSGN
        LSR                     ; SIGN(RESULT) = (SIGN(DIVIDEND) + SIGN(DIVISOR)) & 1
        LDA     TMP
        BCC     +
        EOR     #$FFFF
        INC
+       STA     NOS,S           ; DVDND
        +ACCMEM8
        LDX     ESP
        PLA
        STA     ESTKL,X
        PLA
        STA     ESTKH,X
        PLA
        STA     ESTKL+1,X
        PLA
        STA     ESTKH+1,X
        +ACCMEM16
        JMP     NEXTOP
;*
;* NEGATE TOS
;*
NEG     PLA
        EOR     #$FFFF
        INC
        PHA
        JMP     NEXTOP
;*
;* INCREMENT TOS
;*
INCR    PLA
        INC
        PHA
        JMP     NEXTOP
;*
;* DECREMENT TOS
;*
DECR    PLA
        DEC
        PHA
        JMP     NEXTOP
;*
;* BITWISE COMPLIMENT TOS
;*
COMP    PLA
        EOR     #$FFFF
        PHA
        JMP     NEXTOP
;*
;* BITWISE AND TOS TO TOS-1
;*
BAND    PLA
        AND     TOS,S
        STA     TOS,S
        JMP     NEXTOP
;*
;* INCLUSIVE OR TOS TO TOS-1
;*
IOR     PLA
        ORA     TOS,S
        STA     TOS,S
        JMP     NEXTOP
;*
;* EXLUSIVE OR TOS TO TOS-1
;*
XOR     PLA
        EOR     TOS,S
        STA     TOS,S
        JMP     NEXTOP
;*
;* SHIFT TOS-1 LEFT BY TOS
;*
SHL     PLA
        TAX
        BEQ     +
        PLA
-       ASL
        DEX
        BNE     -
        PHA
+       JMP     NEXTOP
;*
;* SHIFT TOS-1 RIGHT BY TOS
;*
SHR     PLA
        TAX
        BEQ     +
        PLA
-       CMP     #$8000
        ROR
        DEX
        BNE     -
        PHA
+       JMP     NEXTOP
;*
;* DUPLICATE TOS
;*
DUP     LDA     TOS,S
        PHA
        JMP     NEXTOP
;*
;* ADD IMMEDIATE TO TOS
;*
ADDI    INY                     ;+INC_IP
        LDA     (IP),Y
        AND     #$00FF
        CLC
        ADC     TOS,S
        STA     TOS,S
        JMP     NEXTOP
;*
;* SUB IMMEDIATE FROM TOS
;*
SUBI    INY                     ;+INC_IP
        LDA     (IP),Y
        AND     #$00FF
        EOR     #$FFFF
        SEC
        ADC     TOS,S
        STA     TOS,S
        JMP     NEXTOP
;*
;* AND IMMEDIATE TO TOS
;*
ANDI    INY                     ;+INC_IP
        LDA     (IP),Y
        AND     #$00FF
        AND     TOS,S
        STA     TOS,S
        JMP     NEXTOP
;*
;* IOR IMMEDIATE TO TOS
;*
ORI     INY                     ;+INC_IP
        LDA     (IP),Y
        AND     #$00FF
        ORA     TOS,S
        STA     TOS,S
        JMP     NEXTOP
;*
;* LOGICAL NOT
;*
LNOT    PLA
        BNE     ZERO
;*
;* CONSTANT -1, ZERO, NYBBLE, BYTE, $FF BYTE, WORD (BELOW)
;*
MINUS1  PEA     $FFFF
        JMP     NEXTOP
ZERO    PEA     $0000
        JMP     NEXTOP
CN      TXA
        LSR                     ; A = CONST * 2
        PHA
        JMP     NEXTOP
CB      INY                     ;+INC_IP
        LDA     (IP),Y
        AND     #$00FF
        PHA
        JMP     NEXTOP
CFFB    INY                     ;+INC_IP
        LDA     (IP),Y
        ORA     #$FF00
        PHA
        JMP     NEXTOP
;*
;* LOAD ADDRESS & LOAD CONSTANT WORD (SAME THING, WITH OR WITHOUT FIXUP)
;*
LA      INY                     ;+INC_IP
        LDA     (IP),Y
        PHA
        INY
        BMI     +
        JMP     NEXTOP
+       JMP     FIXNEXT
CW      INY                     ;+INC_IP
        LDA     (IP),Y
        PHA
        INY                     ;+INC_IP
        JMP     NEXTOP
;*
;* CONSTANT STRING
;*
CS      ;INY                     ;+INC_IP
        TYA                     ; NORMALIZE IP AND SAVE STRING ADDR ON ESTK
        SEC
        ADC     IP
        STA     IP
        PHA
        LDA     (IP)
        TAY
        JMP     NEXTOP
CSX     ;INY                     ;+INC_IP
        TYA                     ; NORMALIZE IP
        SEC
        ADC     IP
        STA     IP
        LDA     PP              ; SCAN POOL FOR STRING ALREADY THERE
_CMPPSX STA     TMP
        CMP     IFP             ; CHECK FOR END OF POOL
        BCS     _CPYSX          ; AT OR BEYOND END OF POOL, COPY STRING OVER
_CMPSX  +ACCMEM8                ; 8 BIT A/M
        STX     ALTRDOFF        ; CHECK FOR MATCHING STRING
        LDA     (TMP)           ; COMPARE STRINGS FROM AUX MEM TO STRINGS IN MAIN MEM
        STX     ALTRDON
        CMP     (IP)            ; COMPARE STRING LENGTHS
        BNE     _CNXTSX1
        TAY
-       STX     ALTRDOFF
        LDA     (TMP),Y         ; COMPARE STRING CHARS FROM END
        STX     ALTRDON
        CMP     (IP),Y
        BNE     _CNXTSX
        DEY
        BNE     -
        LDA     TMPH            ; MATCH - SAVE EXISTING ADDR ON ESTK AND MOVE ON
        PHA
        LDA     TMPL
        PHA
        BRA     _CEXSX
_CNXTSX STX     ALTRDOFF
        LDA     (TMP)
        STX     ALTRDON
_CNXTSX1 +ACCMEM16              ; 16 BIT A/M
        AND     #$00FF
        SEC                     ; SKIP OVER STRING+LEN BYTE
        ADC     TMP
        BRA     _CMPPSX
_CPYSX  LDA     (IP)            ; COPY STRING FROM AUX TO MAIN MEM POOL
        TAY                     ; MAKE ROOM IN POOL AND SAVE ADDR ON ESTK
        AND     #$00FF
        EOR     #$FFFF
        CLC
        ADC     PP
        STA     PP
        PHA                     ; SAVE ADDRESS ON ESTK
        +ACCMEM8                ; 8 BIT A/M
-       LDA     (IP),Y          ; ALTRD IS ON,  NO NEED TO CHANGE IT HERE
        STA     (PP),Y          ; ALTWR IS OFF, NO NEED TO CHANGE IT HERE
        DEY
        CPY     #$FF
        BNE     -
_CEXSX  LDA     (IP)            ; SKIP TO NEXT OP ADDR AFTER STRING
        TAY
        +ACCMEM16               ; 16 BIT A/M
        JMP     NEXTOP
;*
;* LOAD VALUE FROM ADDRESS TAG
;*
LB      TYX
        LDY     #$00
        TYA                     ; QUICKY CLEAR OUT MSB
        +ACCMEM8                ; 8 BIT A/M
        LDA     (TOS,S),Y
        +ACCMEM16               ; 16 BIT A/M
        STA     TOS,S
        TXY
        JMP     NEXTOP
LW      TYX
        LDY     #$00
        LDA     (TOS,S),Y
        STA     TOS,S
        TXY
        JMP     NEXTOP
LBX     TYX
        LDY     #$00
        TYA                     ; QUICKY CLEAR OUT MSB
        STX     ALTRDOFF
        +ACCMEM8                ; 8 BIT A/M
        LDA     (TOS,S),Y
        +ACCMEM16               ; 16 BIT A/M
        STX     ALTRDON
        STA     TOS,S
        TXY
        JMP     NEXTOP
LWX     TYX
        LDY     #$00
        STX     ALTRDOFF
        LDA     (TOS,S),Y
        STX     ALTRDON
        STA     TOS,S
        TXY
        JMP     NEXTOP
;*
;* LOAD ADDRESS OF LOCAL FRAME OFFSET
;*
-       TYA
        CLC
        ADC     IP
        STA     IP
        LDY     #$FF
LLA     INY                     ;+INC_IP
        BMI     -
        LDA     (IP),Y
        AND     #$00FF
        CLC
        ADC     IFP
        PHA
        JMP     NEXTOP
;*
;* LOAD VALUE FROM LOCAL FRAME OFFSET
;*
LLB     INY                     ;+INC_IP
        TYX
        LDA     (IP),Y
        TAY
        LDA     (IFP),Y
        AND     #$00FF
        PHA
        TXY
        JMP     NEXTOP
LLW     INY                     ;+INC_IP
        TYX
        LDA     (IP),Y
        TAY
        LDA     (IFP),Y
        PHA
        TXY
        JMP     NEXTOP
LLBX    INY                     ;+INC_IP
        TYX
        LDA     (IP),Y
        TAY
        STX     ALTRDOFF
        LDA     (IFP),Y
        STX     ALTRDON
        AND     #$00FF
        PHA
        TXY
        JMP     NEXTOP
LLWX    INY                     ;+INC_IP
        TYX
        LDA     (IP),Y
        TAY
        STX     ALTRDOFF
        LDA     (IFP),Y
        STX     ALTRDON
        PHA
        TXY
        JMP     NEXTOP
;*
;* ADD VALUE FROM LOCAL FRAME OFFSET
;*
ADDLB   INY                     ;+INC_IP
        TYX
        LDA     (IP),Y
        TAY
        LDA     (IFP),Y
        AND     #$00FF
        TXY
        CLC
        ADC     TOS,S
        STA     TOS,S
        JMP     NEXTOP
ADDLBX  INY                     ;+INC_IP
        TYX
        LDA     (IP),Y
        TAY
        STX     ALTRDOFF
        LDA     (IFP),Y
        STX     ALTRDON
        AND     #$00FF
        TXY
        CLC
        ADC     TOS,S
        STA     TOS,S
        JMP     NEXTOP
ADDLW   INY                     ;+INC_IP
        TYX
        LDA     (IP),Y
        TAY
        LDA     (IFP),Y
        TXY
        CLC
        ADC     TOS,S
        STA     TOS,S
        JMP     NEXTOP
ADDLWX  INY                     ;+INC_IP
        TYX
        LDA     (IP),Y
        TAY
        STX     ALTRDOFF
        LDA     (IFP),Y
        STX     ALTRDON
        TXY
        CLC
        ADC     TOS,S
        STA     TOS,S
        JMP     NEXTOP
;*
;* INDEX VALUE FROM LOCAL FRAME OFFSET
;*
IDXLB   INY                     ;+INC_IP
        TYX
        LDA     (IP),Y
        TAY
        LDA     (IFP),Y
        AND     #$00FF
        TXY
        ASL
        CLC
        ADC     TOS,S
        STA     TOS,S
        JMP     NEXTOP
IDXLBX  INY                     ;+INC_IP
        TYX
        LDA     (IP),Y
        TAY
        STX     ALTRDOFF
        LDA     (IFP),Y
        STX     ALTRDON
        AND     #$00FF
        TXY
        ASL
        CLC
        ADC     TOS,S
        STA     TOS,S
        JMP     NEXTOP
IDXLW   INY                     ;+INC_IP
        TYX
        LDA     (IP),Y
        TAY
        LDA     (IFP),Y
        TXY
        ASL
        CLC
        ADC     TOS,S
        STA     TOS,S
        JMP     NEXTOP
IDXLWX  INY                     ;+INC_IP
        TYX
        LDA     (IP),Y
        TAY
        STX     ALTRDOFF
        LDA     (IFP),Y
        STX     ALTRDON
        TXY
        ASL
        CLC
        ADC     TOS,S
        STA     TOS,S
        JMP     NEXTOP
;*
;* LOAD VALUE FROM ABSOLUTE ADDRESS
;*
LAB     INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMP
        TYA                     ; QUICKY CLEAR OUT MSB
        +ACCMEM8                ; 8 BIT A/M
        LDA     (TMP)
        +ACCMEM16               ; 16 BIT A/M
        PHA
        INY                     ;+INC_IP
        JMP     NEXTOP
LAW     INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMP
        LDA     (TMP)
        PHA
        INY                     ;+INC_IP
        JMP     NEXTOP
LABX    INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMP
        TYA                     ; QUICKY CLEAR OUT MSB
        STX     ALTRDOFF
        +ACCMEM8                ; 8 BIT A/M
        LDA     (TMP)
        +ACCMEM16               ; 16 BIT A/M
        STX     ALTRDON
        PHA
        INY                     ;+INC_IP
        JMP     NEXTOP
LAWX    INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMP
        STX     ALTRDOFF
        LDA     (TMP)
        STX     ALTRDON
        PHA
        INY                     ;+INC_IP
        JMP     NEXTOP
;*
;* ADD VALUE FROM ABSOLUTE ADDRESS
;*
ADDAB   INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMP
        TYA                     ; QUICKY CLEAR OUT MSB
        +ACCMEM8                ; 8 BIT A/M
        LDA     (TMP)
        +ACCMEM16               ; 16 BIT A/M
        INY                     ;+INC_IP
        CLC
        ADC     TOS,S
        STA     TOS,S
        JMP     NEXTOP
ADDABX  INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMP
        TYA                     ; QUICKY CLEAR OUT MSB
        STX     ALTRDOFF
        +ACCMEM8                ; 8 BIT A/M
        LDA     (TMP)
        +ACCMEM16               ; 16 BIT A/M
        STX     ALTRDON
        INY                     ;+INC_IP
        CLC
        ADC     TOS,S
        STA     TOS,S
        JMP     NEXTOP
ADDAW   INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMP
        LDA     (TMP)
        INY                     ;+INC_IP
        CLC
        ADC     TOS,S
        STA     TOS,S
        JMP     NEXTOP
ADDAWX  INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMP
        STX     ALTRDOFF
        LDA     (TMP)
        STX     ALTRDON
        INY                     ;+INC_IP
        CLC
        ADC     TOS,S
        STA     TOS,S
        JMP     NEXTOP
;*
;* INDEX VALUE FROM ABSOLUTE ADDRESS
;*
IDXAB   INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMP
        TYA                     ; QUICKY CLEAR OUT MSB
        +ACCMEM8                ; 8 BIT A/M
        LDA     (TMP)
        +ACCMEM16               ; 16 BIT A/M
        INY                     ;+INC_IP
        ASL
        CLC
        ADC     TOS,S
        STA     TOS,S
        JMP     NEXTOP
IDXABX  INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMP
        TYA                     ; QUICKY CLEAR OUT MSB
        STX     ALTRDOFF
        +ACCMEM8                ; 8 BIT A/M
        LDA     (TMP)
        +ACCMEM16               ; 16 BIT A/M
        STX     ALTRDON
        INY                     ;+INC_IP
        ASL
        CLC
        ADC     TOS,S
        STA     TOS,S
        JMP     NEXTOP
IDXAW   INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMP
        LDA     (TMP)
        INY                     ;+INC_IP
        ASL
        CLC
        ADC     TOS,S
        STA     TOS,S
        JMP     NEXTOP
IDXAWX  INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMP
        STX     ALTRDOFF
        LDA     (TMP)
        STX     ALTRDON
        INY                     ;+INC_IP
        ASL
        CLC
        ADC     TOS,S
        STA     TOS,S
        JMP     NEXTOP
;*
;* STORE VALUE TO ADDRESS
;*
SB      TYX
        LDY     #$00
        +ACCMEM8                ; 8 BIT A/M
        LDA     NOS,S
        STA     (TOS,S),Y
        +ACCMEM16               ; 16 BIT A/M
        TXY
        PLA
        JMP     DROP
SW      TYX
        LDY     #$00
        LDA     NOS,S
        STA     (TOS,S),Y
        TXY
;*
;* DROP TOS, TOS-1
;*
DROP2   PLA
        JMP     DROP
;*
;* STORE VALUE TO LOCAL FRAME OFFSET
;*
SLB     INY                     ;+INC_IP
        TYX
        LDA     (IP),Y
        TAY
        PLA
        +ACCMEM8                ; 8 BIT A/M
        STA     (IFP),Y
        +ACCMEM16               ; 16 BIT A/M
        TXY
        BMI     +
        JMP     NEXTOP
SLW     INY                     ;+INC_IP
        LDA     (IP),Y
        TYX
        TAY
        PLA
        STA     (IFP),Y
        TXY
        BMI     +
        JMP     NEXTOP
+       JMP     FIXNEXT
;*
;* STORE VALUE TO LOCAL FRAME OFFSET WITHOUT POPPING STACK
;*
DLB     INY                     ;+INC_IP
        TYX
        +ACCMEM8                ; 8 BIT A/M
        LDA     (IP),Y
        TAY
        LDA     TOS,S
        STA     (IFP),Y
        +ACCMEM16               ; 16 BIT A/M
        AND     #$00FF
        STA     TOS,S
        TXY
        JMP     NEXTOP
DLW     INY                     ;+INC_IP
        LDA     (IP),Y
        TYX
        TAY
        LDA     TOS,S
        STA     (IFP),Y
        TXY
        JMP     NEXTOP
;*
;* STORE VALUE TO ABSOLUTE ADDRESS
;*
SAB     INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMP
        PLA
        +ACCMEM8                ; 8 BIT A/M
        STA     (TMP)
        +ACCMEM16               ; 16 BIT A/M
        INY
        BMI     +
        JMP     NEXTOP
SAW     INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMP
        PLA
        STA     (TMP)
        INY
        BMI     +
        JMP     NEXTOP
+       JMP     FIXNEXT
;*
;* STORE VALUE TO ABSOLUTE ADDRESS WITHOUT POPPING STACK
;*
DAB     INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMP
        +ACCMEM8                ; 8 BIT A/M
        LDA     TOS,S
        STA     (TMP)
        +ACCMEM16               ; 16 BIT A/M
        AND     #$00FF
        STA     TOS,S
        INY                     ;+INC_IP
        JMP     NEXTOP
DAW     INY                     ;+INC_IP
        LDA     (IP),Y
        STA     TMP
        LDA     TOS,S
        STA     (TMP)
        INY                     ;+INC_IP
        JMP     NEXTOP
;*
;* COMPARES
;*
ISEQ    PLA
        CMP     TOS,S
        BNE     ISFLS
ISTRU   LDA     #$FFFF
        STA     TOS,S
        JMP     NEXTOP
ISNE    PLA
        CMP     TOS,S
        BNE     ISTRU
ISFLS   LDA     #$0000
        STA     TOS,S
        JMP     NEXTOP
ISGE    PLA
        SEC
        SBC     TOS,S
        BVS     +
        BMI     ISTRU
        BEQ     ISTRU
        BPL     ISFLS
+       BMI     ISFLS
        BEQ     ISFLS
        BPL     ISTRU
ISGT    PLA
        SEC
        SBC     TOS,S
        BVS     +
        BMI     ISTRU
        BPL     ISFLS
+       BMI     ISFLS
        BPL     ISTRU
ISLE    PLA
        SEC
        SBC     TOS,S
        BVS     +
        BPL     ISTRU
        BMI     ISFLS
+       BPL     ISFLS
        BMI     ISTRU
ISLT    PLA
        SEC
        SBC     TOS,S
        BVS     +
        BMI     ISFLS
        BEQ     ISFLS
        BPL     ISTRU
+       BMI     ISTRU
        BEQ     ISTRU
        BPL     ISFLS
;*
;* BRANCHES
;*
SEL     TYA                     ; FLATTEN IP
        SEC
        ADC     IP
        INY                     ;+INC_IP
        ;CLC                    ; ADD BRANCH OFFSET (BETTER NOT CARRY OUT OF IP+Y)
        ADC     (IP),Y
        STA     IP
        LDY     #$00
        LDA     (IP),Y
        TAX                     ; CASE COUNT
        PLA
        INC     IP
CASELP  CMP     (IP),Y
        BEQ     +
        BMI     CASEEND         ; CASE VALS IN ASCENDING ORDER, EXIT WHEN LESS
        INY
        INY
        INY
        DEX
        BEQ     FIXNEXT
        INY
        BNE     CASELP
        +ACCMEM8                ; 8 BIT A/M
        INC     IPH
        +ACCMEM16               ; 16 BIT A/M
        BRA     CASELP
+       INY
        BRA     BRNCH
CASEEND TXA                     ; SKIP REMAINING CASES
        ASL
        ASL
        DEC
;        CLC
        ADC     IP
        STA     IP
FIXNEXT TYA
        LDY     #$00
        SEC
        ADC     IP
        STA     IP
        JMP     FETCHOP
BRAND   LDA     TOS,S
        BEQ     BRNCH
        PLA                     ; DROP LEFT HALF OF AND
        BRA     NOBRNCH
BROR    LDA     TOS,S
        BNE     BRNCH
        PLA                     ; DROP LEFT HALF OF OR
        BRA     NOBRNCH
BREQ    PLA
        CMP     TOS,S
        BNE     +
        PLA
        BRA     BRNCH
BRNE    PLA
        CMP     TOS,S
        BEQ     +
        PLA
        BRA     BRNCH
+       PLA
        BRA     NOBRNCH
BRTRU   PLA
        BNE     BRNCH
NOBRNCH INY                     ;+INC_IP
        INY
        BMI     FIXNEXT
        JMP     NEXTOP
BRFLS   PLA
        BNE     NOBRNCH
BRNCH   TYA                     ; FLATTEN IP
        SEC
        ADC     IP
        INY                     ;+INC_IP
        ;CLC                    ; ADD BRANCH OFFSET (BETTER NOT CARRY OUT OF IP+Y)
        ADC     (IP),Y
        STA     IP
        LDY     #$00
        JMP     FETCHOP
;*
;* FOR LOOPS PUT TERMINAL VALUE AT ESTK+1 AND CURRENT COUNT ON ESTK
;*
BRGT    LDA     NOS,S
        SEC
        SBC     TOS,S
        BVS     +
        BPL     NOBRNCH
        BMI     BRNCH
BRLT    LDA     TOS,S
        SEC
        SBC     NOS,S
        BVS     +
        BPL     NOBRNCH
        BMI     BRNCH
+       BMI     NOBRNCH
        BPL     BRNCH
DECBRGE PLA
        DEC
        PHA
_BRGE   LDA     TOS,S
        SEC
        SBC     NOS,S
        BVS     +
        BPL     BRNCH
        BMI     NOBRNCH
INCBRLE PLA
        INC
        PHA
_BRLE   LDA     NOS,S
        SEC
        SBC     TOS,S
        BVS     +
        BPL     BRNCH
        BMI     NOBRNCH
+       BMI     BRNCH
        BPL     NOBRNCH
SUBBRGE LDA     NOS,S
        SEC
        SBC     TOS,S
        STA     NOS,S
        PLA
        BRA     _BRGE
ADDBRLE PLA
        CLC
        ADC     TOS,S
        STA     TOS,S
        BRA     _BRLE
;*
;* INDIRECT CALL TO ADDRESS (NATIVE CODE)
;*
ICAL    PLA
        BRA     EMUSTK
;*
;* CALL INTO ABSOLUTE ADDRESS (NATIVE CODE)
;*
CALL    INY                     ;+INC_IP
        LDA     (IP),Y
        INY
EMUSTK  STA     TMP
        TYA                     ; FLATTEN IP
        SEC
        ADC     IP
        STA     IP
        SEC                     ; SWITCH TO EMULATED MODE
        XCE
        !AS
        TSC                     ; MOVE HW EVAL STACK TO ZP EVAL STACK
        EOR     #$FF
        SEC
        ADC     HWSP            ; STACK DEPTH = (HWSP - SP)/2
        LSR
!IF     DEBUG {
        PHA
        CLC
        ADC     #$80+'0'
        STA     $7D0+31
        PLA
}
        EOR     #$FF
        SEC
        ADC     ESP             ; ESP - STACK DEPTH
        TAY
        TAX
        CPX     ESP
        BEQ     +
-       PLA
        STA     ESTKL,X
        PLA
        STA     ESTKH,X
        INX
        CPX     ESP
        BNE     -
+
!IF     DEBUG {
        TXA
        TSX
        CPX     HWSP
        BEQ     +
        LDX     #$80+'C'
        STX     $7D0+30
-       LDX    $C000
        BPL     -
        LDX     $C010
+       TAX
}
        PEI     (IP)            ; SAVE INSTRUCTION POINTER
        PHX                     ; SAVE BASELINE ESP
        TYX
        LDA     PSR
        PHA
        PLP
        JSR     JMPTMP
        PHP
        PLA
        STA     PSR
        SEI
        PLY                     ; MOVE RETURN VALUES TO HW EVAL STACK
        STY     ESP             ; RESTORE BASELINE ESP
        PLA
        STA     IPL
        PLA
        STA     IPH
!IF     DEBUG {
        TXA
        EOR     #$FF
        SEC
        ADC     ESP
        CLC
        ADC     #$80+'0'
        STA     $7D0+32
}
        STX     TMPL
        TSX                     ; RESTORE BASELINE HWSP
        STX     HWSP
        CPY     TMPL
        BEQ     +
        TYX
-       DEX
        LDA     ESTKH,X
        PHA
        LDA     ESTKL,X
        PHA
        CPX     TMPL
        BNE     -
+       CLC                     ; SWITCH BACK TO NATIVE MODE
        XCE
        +ACCMEM16               ; 16 BIT A/M
        LDX     #>OPTBL         ; MAKE SURE WE'RE INDEXING THE RIGHT TABLE
!IF DEBUG {
        LDY     LCRWEN+LCBNK2
        LDY     LCRWEN+LCBNK2
        STX     DBG_OP+2
        LDY     LCRDEN+LCBNK2
        LDX     #>DBGTBL
}
        STX     OPPAGE
        LDY     #$00
        JMP     FETCHOP
;*
;* INDIRECT CALL TO ADDRESS (NATIVE CODE)
;*
ICALX   PLA
        BRA     EMUSTKX
;*
;* CALL INTO ABSOLUTE ADDRESS (NATIVE CODE)
;*
CALLX   INY                     ;+INC_IP
        LDA     (IP),Y
        INY
EMUSTKX STA     TMP
        TYA                     ; FLATTEN IP
        SEC
        ADC     IP
        STA     IP
        SEC                     ; SWITCH TO EMULATION MODE
        XCE
        !AS
        TSC                     ; MOVE HW EVAL STACK TO ZP EVAL STACK
        EOR     #$FF
        SEC
        ADC     HWSP            ; STACK DEPTH = (HWSP - SP)/2
        LSR
!IF     DEBUG {
        PHA
        CLC
        ADC     #$80+'0'
        STA     $7D0+31
        PLA
}
        EOR     #$FF
        SEC
        ADC     ESP             ; ESP - STACK DEPTH
        TAY
        TAX
        CPX     ESP
        BEQ     +
-       PLA
        STA     ESTKL,X
        PLA
        STA     ESTKH,X
        INX
        CPX     ESP
        BNE     -
+
!IF     DEBUG {
        TXA
        TSX
        CPX     HWSP
        BEQ     +
        LDX     #$80+'X'
        STX     $7D0+30
-       LDX     $C000
        BPL     -
        LDX     $C010
+       TAX
}
        PEI     (IP)            ; SAVE INSTRUCTION POINTER
        PHX                     ; SAVE BASELINE ESP
        TYX
        STX     ALTRDOFF
        LDA     PSR
        PHA
        PLP
        JSR     JMPTMP
        PHP
        PLA
        STA     PSR
        SEI
        STX     ALTRDON
        PLY                     ; MOVE RETURN VALUES TO HW EVAL STACK
        STY     ESP             ; RESTORE BASELINE ESP
        PLA
        STA     IPL
        PLA
        STA     IPH
!IF     DEBUG {
        TXA
        EOR     #$FF
        SEC
        ADC     ESP
        CLC
        ADC     #$80+'0'
        STA     $7D0+32
}
        STX     TMPL
        TSX                     ; RESTORE BASELINE HWSP
        STX     HWSP
        CPY     TMPL
        BEQ     +
        TYX
-       DEX
        LDA     ESTKH,X
        PHA
        LDA     ESTKL,X
        PHA
        CPX     TMPL
        BNE     -
+       CLC                     ; SWITCH BACK TO NATIVE MODE
        XCE
        +ACCMEM16               ; 16 BIT A/M
        LDX     #>OPXTBL        ; MAKE SURE WE'RE INDEXING THE RIGHT TABLE
!IF DEBUG {
        LDY     LCRWEN+LCBNK2
        LDY     LCRWEN+LCBNK2
        STX     DBG_OP+2
        LDY     LCRDEN+LCBNK2
        LDX     #>DBGTBL
}
        STX     OPPAGE
        LDY     #$00
        JMP     FETCHOP
;*
;* JUMP INDIRECT THROUGH TMP
;*
;JMPTMP  JMP     (TMP)
;*
;* ENTER FUNCTION WITH FRAME SIZE AND PARAM COUNT
;*
ENTER   PEI     (IFP)           ; SAVE ON STACK FOR LEAVE
        TSX                     ; REFLECT SP IN SAVED HWSP
        STX     HWSP
        INY
        LDA     (IP),Y
        AND     #$00FF
!IF     DEBUG {
        +ACCMEM8                ; 8 BIT A/M
        PHA
        CLC
        ADC     #$80+'0'
        STA     $7D0+31
        PLA
        +ACCMEM16               ; 16 BIT A/M
}
        EOR     #$FFFF          ; ALLOCATE FRAME
        SEC
        ADC     PP
        STA     PP
        STA     IFP
        +ACCMEM8                ; 8 BIT A/M
        INY
        LDA     (IP),Y
        BEQ     +
        ASL
        TAY
        LDX     ESP             ; MOVE PARAMETERS TO CALL FRAME
-       LDA     ESTKH,X
        DEY
        STA     (IFP),Y
        LDA     ESTKL,X
        INX
        DEY
        STA     (IFP),Y
        BNE -
        STX     ESP
+       +ACCMEM16               ; 16 BIT A/M
        LDY     #$03
        JMP     FETCHOP
;*
;* LEAVE FUNCTION
;*
LEAVE   INY                     ;+INC_IP
        +ACCMEM8                ; 8 BIT A/M
        LDA     (IP),Y          ; DEALLOCATE POOL + FRAME
        BRA     +
LEAVEX  INY                     ;+INC_IP
        +ACCMEM8                ; 8 BIT A/M
        LDA     (IP),Y          ; DEALLOCATE POOL + FRAME
        STA     ALTRDOFF
+       STA     TMPL
        TSC                     ; MOVE HW EVAL STACK TO ZP EVAL STACK
        EOR     #$FF
        SEC
        ADC     HWSP            ; STACK DEPTH = (HWSP - SP)/2
        LSR
!IF     DEBUG {
        PHA
        CLC
        ADC     #$80+'0'
        STA     $7D0+31
        PLA
}
        EOR     #$FF
        SEC
        ADC     ESP             ; ESP - STACK DEPTH
        TAX
        CPX     ESP
        BEQ     ++
        TAY
-       PLA
        STA     ESTKL,X
        PLA
        STA     ESTKH,X
        INX
        CPX     ESP
        BNE     -
!IF     DEBUG {
        TSX
        CPX     HWSP
        BEQ     +
        LDX     #$80+'L'
        STX     $7D0+30
-       LDX     $C000
        BPL     -
        LDX     $C010
+
}
        TYX                     ; RESTORE NEW ESP
++      +ACCMEM16               ; 16 BIT A/M
        LDY     TMPL            ; DEALLOCATE POOL + FRAME
        TYA
        CLC
        ADC     IFP
        STA     PP
        PLA                     ; RESTORE PREVIOUS FRAME
        STA     IFP
        SEC                     ; SWITCH TO EMULATION MODE
        XCE
        !AS
        LDA     PSR
        PHA
        PLP
        RTS
        !AL
RETX    STX     ALTRDOFF
RET     SEC                     ; SWITCH TO EMULATION MODE
        XCE
        !AS
        ;+ACCMEM8                ; 8 BIT A/M
        TSC                     ; MOVE HW EVAL STACK TO ZP EVAL STACK
        EOR     #$FF
        SEC
        ADC     HWSP            ; STACK DEPTH = (HWSP - SP)/2
        LSR
!IF     DEBUG {
        PHA
        CLC
        ADC     #$80+'0'
        STA     $7D0+31
        PLA
}
        EOR     #$FF
        SEC
        ADC     ESP             ; ESP - STACK DEPTH
        TAX
        CPX     ESP
        BEQ     ++
        TAY
-       PLA
        STA     ESTKL,X
        PLA
        STA     ESTKH,X
        INX
        CPX     ESP
        BNE     -
!IF     DEBUG {
        TSX
        CPX     HWSP
        BEQ     +
        LDX     #$80+'X'
        STX     $7D0+30
-       LDX     $C000
        BPL     -
        LDX     $C010
+
}
        TYX
++      LDA     PSR
        PHA
        PLP
        RTS
;*
;* RETURN TO NATIVE CODE
;*
NATV    TYA                     ; FLATTEN IP
        SEC
        ADC     IP
        STA     IP
        +INDEX16                ; SET 16 BIT X/Y
        JMP     (IP)
!IF     DEBUG {
;*****************
;*               *
;*  DEBUG TABLE  *
;*               *
;*****************
        !ALIGN  255,0
DBGTBL  !WORD   STEP,STEP,STEP,STEP,STEP,STEP,STEP,STEP         ; 00 02 04 06 08 0A 0C 0E
        !WORD   STEP,STEP,STEP,STEP,STEP,STEP,STEP,STEP         ; 10 12 14 16 18 1A 1C 1E
        !WORD   STEP,STEP,STEP,STEP,STEP,STEP,STEP,STEP         ; 20 22 24 26 28 2A 2C 2E
        !WORD   STEP,STEP,STEP,STEP,STEP,STEP,STEP,STEP         ; 30 32 34 36 38 3A 3C 3E
        !WORD   STEP,STEP,STEP,STEP,STEP,STEP,STEP,STEP         ; 40 42 44 46 48 4A 4C 4E
        !WORD   STEP,STEP,STEP,STEP,STEP,STEP,STEP,STEP         ; 50 52 54 56 58 5A 5C 5E
        !WORD   STEP,STEP,STEP,STEP,STEP,STEP,STEP,STEP         ; 60 62 64 66 68 6A 6C 6E
        !WORD   STEP,STEP,STEP,STEP,STEP,STEP,STEP,STEP         ; 70 72 74 76 78 7A 7C 7E
        !WORD   STEP,STEP,STEP,STEP,STEP,STEP,STEP,STEP         ; 80 82 84 86 88 8A 8C 8E
        !WORD   STEP,STEP,STEP,STEP,STEP,STEP,STEP,STEP         ; 90 92 94 96 98 9A 9C 9E
        !WORD   STEP,STEP,STEP,STEP,STEP,STEP,STEP,STEP         ; A0 A2 A4 A6 A8 AA AC AE
        !WORD   STEP,STEP,STEP,STEP,STEP,STEP,STEP,STEP         ; B0 B2 B4 B6 B8 BA BC BE
        !WORD   STEP                                            ; C0
;*
;* DEBUG PRINT ROUTINES
;*
        !AS
PRHEX   PHA
        LSR
        LSR
        LSR
        LSR
        CLC
        ADC     #'0'
        CMP     #':'
        BCC     +
        ADC     #6
+       ORA     #$80
        STA     $7D0,X
        INX
        PLA
        AND     #$0F
        ADC     #'0'
        CMP     #':'
        BCC     +
        ADC     #6
+       ORA     #$80
        STA     $7D0,X
        INX
        RTS
PRCHR   ORA     #$80
        STA     $7D0,X
        INX
        RTS
PRBYTE  PHA
        LDA     #'$'
        JSR     PRCHR
        PLA
        JMP     PRHEX
PRWORD  PHA
        LDA     #'$'
        JSR     PRCHR
        XBA
        JSR     PRHEX
        PLA
        JMP     PRHEX
        !AL
; MESSAGES
BADEMU  !TEXT   "IN EMULATION MODE!", 0
BADMODE !TEXT   "IN 8 BIT A/M MODE!", 0
;*
;* DEBUG STEP ROUTINE
;*
STEP    STX     TMPL
        PHP                     ; VALIDATE MODES
        SEC                     ; SWITCH TO EMULATED MODE
        XCE
        +ACCMEM8                ; 8 BIT A/M
        BCC     +
        LDX     #$00
--      LDA     BADEMU,X
-       BEQ     -
        JSR     PRCHR
        BRA     --
+       XCE                     ; SWITCH BACK TO NATIVE MODE
        PLX
        TXA
        AND     #$20
        BEQ     ++
        LDX     #$00
-       LDA     BADMODE,X
        BNE     +
        JMP     DBGKEY
+       JSR     PRCHR
        BRA     -
++      LDX     $C013           ; SAVE RAMRD
        STX     $C002
        STX     TMPH
        LDX     #39             ; SCROLL PREVIOUS LINES UP
-       LDA     $6D0,X
        STA     $650,X
        LDA     $750,X
        STA     $6D0,X
        LDA     $7D0,X
        STA     $750,X
        DEX
        BPL     -
        LDA     #' '
        BIT     TMPH            ; RAMRD SET?
        BPL     +
        STX     $C003
        LDA     #'X'
+       LDX     #$00
        JSR     PRCHR
        +ACCMEM16               ; 16 BIT A/M
        TYA
        CLC
        ADC     IP
        +ACCMEM8                ; 8 BIT A/M
        JSR     PRWORD
        LDA     #':'
        JSR     PRCHR
        LDA     (IP),Y
        JSR     PRBYTE
        INX
        +ACCMEM16               ; 16 BIT A/M
        TSC
        +ACCMEM8                ; 8 BIT A/M
        JSR     PRWORD
        LDA     #$80+'['
        JSR     PRCHR
        STX     TMPH
        TSX
        TXA
        EOR     #$FF
        SEC
        ADC     HWSP
        LSR
        CLC
        ADC     #$80+'0'
        LDX     TMPH
        JSR     PRCHR
        LDA     #$80+']'
        JSR     PRCHR
        LDA     #':'
        JSR     PRCHR
        STX     TMPH
        TSX
        CPX     HWSP
        BEQ     ++
        BCC     +
        LDX     TMPH
        LDA     #' '
        JSR     PRCHR
        LDA     #'<'            ; STACK UNDERFLOW!
        JSR     PRCHR
        JSR     PRCHR
        JSR     PRCHR
        JSR     PRCHR
        BRA     DBGKEY
+       LDA     $102,X
        XBA
        LDA     $101,X
        LDX     TMPH
        JSR     PRWORD
        BRA     +++
++      LDX     TMPH
        LDA     #' '
        JSR     PRCHR
        LDA     #'-'
        JSR     PRCHR
        JSR     PRCHR
        JSR     PRCHR
        JSR     PRCHR
+++     LDA     #' '
-       JSR     PRCHR
        CPX     #40
        BNE     -
        TSX
        CMP     #$10
        BCC     DBGKEY
        LDX     TMPL
;        CPX     #$00            ; FORCE PAUSE AT 'ZERO'
;        BEQ     DBGKEY
-       LDX     $C000
        CPX     #$9B
        BNE     +
DBGKEY  STX     $C010
-       LDX     $C000
        BPL     -
        CPX     #$9B
        BEQ     +
        STX     $C010
        CPX     #$80+'Q'
        BNE     +
        SEC                     ; SWITCH TO EMU MODE
        XCE
        BIT     $C054           ; SET TEXT MODE
        BIT     $C051
        BIT     $C05F
        LDA     #20             ; SET TEXT WINDOW ABOVE DEBUG OUTPUT
        STA     $23
        STZ     $20
        STZ     $22
        STZ     $24
        STZ     $25
        STZ     $28
        LDA     #$04
        STA     $29
        BRK
+       +ACCMEM16               ; 16 BIT A/M
        LDX     TMPL
DBG_OP  JMP     (OPTBL,X)
}

VMEND   =       *
}

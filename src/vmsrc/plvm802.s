;**********************************************************
;*
;*          APPLE ][ 65802/65816 PLASMA INTERPRETER
;*
;*              SYSTEM ROUTINES AND LOCATIONS
;*
;**********************************************************
        !CPU    65816
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
DROP16  =       $EE
NEXTOP16=       $EF
FETCHOP16=      NEXTOP16+3
IP16    =       $F3
IP16L   =       $F3
IP16H   =       IP16L+1
OP16IDX =       FETCHOP16+4
OP16PAGE=       OP16IDX+1
HWSP    =       TMPH+1
;
; BUFFER ADDRESSES
;
STRBUF  =       $0280
INTERP  =       $03D0
;*
;* HARDWARE STACK OFFSETS
;*
TOS     =       $01             ; TOS
NOS     =       $03             ; TOS-1
;*
;* INTERPRETER INSTRUCTION POINTER INCREMENT MACRO
;*
        !MACRO  INC_IP  {
        INY
        BNE     * + 8
        SEP     #$20            ; SET 8 BIT MODE
        INC     IP16H
        REP     #$20            ; SET 16 BIT MODE
        }
;******************************
;*                            *
;* INTERPRETER INITIALIZATION *
;*                            *
;******************************
*        =      $2000
        LDX     #$FE
        TXS
        LDX     #$00
        STX     $01FF
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
;* SAVE DEFAULT COMMAND INTERPRETER PATH IN LC
;*
        JSR     PRODOS          ; GET PREFIX
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
        !WORD   DROP,DUP,PUSHEP,PULLEP,BRGT,BRLT,BREQ,BRNE      ; 30 32 34 36 38 3A 3C 3E
        !WORD   ISEQ,ISNE,ISGT,ISLT,ISGE,ISLE,BRFLS,BRTRU       ; 40 42 44 46 48 4A 4C 4E
        !WORD   BRNCH,IBRNCH,CALL,ICAL,ENTER,LEAVE,RET,CFFB     ; 50 52 54 56 58 5A 5C 5E
        !WORD   LB,LW,LLB,LLW,LAB,LAW,DLB,DLW                   ; 60 62 64 66 68 6A 6C 6E
        !WORD   SB,SW,SLB,SLW,SAB,SAW,DAB,DAW                   ; 70 72 74 76 78 7A 7C 7E
;*
;* ENTER INTO BYTECODE INTERPRETER - IMMEDIATELY SWITCH TO NATIVE
;*
        !AL
DINTRP  CLC                     ; SWITCH TO NATIVE MODE
        XCE
        REP     #$20            ; 16 BIT A/M
        SEP     #$10            ; 8 BIT X,Y
        PLA
        INC
        STA     IP16
        LDA     IFP
        PHA                     ; SAVE ON STACK FOR LEAVE/RET
        LDA     PP              ; SET FP TO PP
        STA     IFP
        LDY     #$00
        STX     ESP
        TSX
        STX     HWSP
        LDX     #>OPTBL
        STX     OP16PAGE
        JMP     FETCHOP16
IINTRP  CLC                     ; SWITCH TO NATIVE MODE
        XCE
        REP     #$20            ; 16 BIT A/M
        SEP     #$10            ; 8 BIT X,Y
        PLA
        STA     TMP
        LDY     #$01
        LDA     (TMP),Y
        DEY
        STA     IP16
        LDA     IFP
        PHA                     ; SAVE ON STACK FOR LEAVE/RET
        LDA     PP              ; SET FP TO PP
        STA     IFP
+       STX     ESP
        TSX
        STX     HWSP
        LDX     #>OPTBL
        STX     OP16PAGE
        JMP     FETCHOP16
IINTRPX CLC                     ; SWITCH TO NATIVE MODE
        XCE
        REP     #$20            ; 16 BIT A/M
        SEP     #$10            ; 8 BIT X,Y
        PLA
        STA     TMP
        LDY     #$0
        LDA     (TMP),Y
        STA     IP16
        DEY
        LDA     IFP
        PHA                     ; SAVE ON STACK FOR LEAVE/RET
        LDA     PP             ; SET FP TO PP
        STA     IFP
        STX     ESP
        TSX
        STX     HWSP
        LDX     #>OPXTBL
        STX     OP16PAGE
        ;SEI UNTIL I KNOW WHAT TO DO WITH THE UNENHANCED IIE
        STX     ALTRDON
        JMP     FETCHOP16
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
        INY                     ; CLEAR CMDLINE BUFF
        STY     $01FF
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
        LDY     #$11
-       LDA     PAGE0,Y
        STA     DROP16,Y
        DEY
        BPL     -
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
;        LDA #$00               ; INIT FRAME POINTER
        STA     PPL
        STA     IFPL
        LDA     #$BF
        STA     PPH
        STA     IFPH
        LDX     #$FE            ; INIT STACK POINTER (YES, $FE. SEE GETS)
        TXS
        LDX     #ESTKSZ/2       ; INIT EVAL STACK INDEX
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
        !PSEUDOPC       $00EE {
        PLA                     ; DROP16 @ $EE
        INY                     ; NEXTOP16 @ $EF
        BEQ     NEXTOPH
        LDX     $FFFF,Y         ; FETCHOP16 @ $F2, IP16 MAPS OVER $FFFF @ $F3
        JMP     (OPTBL,X)       ; OP16IDX AND OP16PAGE MAP OVER OPTBL
NEXTOPH SEP     #$20            ; SET 8 BIT MODE
        INC     IP16H
        REP     #$20            ; SET 16 BIT MODE
        BRA     FETCHOP16
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
        !WORD   LNOT,LOR,LAND,LA,LLA,CB,CW,CS                   ; 20 22 24 26 28 2A 2C 2E
        !WORD   DROP,DUP,PUSHEP,PULLEP,BRGT,BRLT,BREQ,BRNE      ; 30 32 34 36 38 3A 3C 3E
        !WORD   ISEQ,ISNE,ISGT,ISLT,ISGE,ISLE,BRFLS,BRTRU       ; 40 42 44 46 48 4A 4C 4E
        !WORD   BRNCH,IBRNCH,CALLX,ICALX,ENTER,LEAVEX,RETX,CFFB ; 50 52 54 56 58 5A 5C 5E
        !WORD   LBX,LWX,LLBX,LLWX,LABX,LAWX,DLB,DLW             ; 60 62 64 66 68 6A 6C 6E
        !WORD   SB,SW,SLB,SLW,SAB,SAW,DAB,DAW                   ; 70 72 74 76 78 7A 7C 7E
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
        JMP     NEXTOP16
;*
;* SUB TOS FROM TOS-1(NOS)
;*
SUB     LDA     NOS,S
        SEC
        SBC     TOS,S
        STA     NOS,X
        JMP     DROP16
;*
;* SHIFT TOS LEFT BY 1, ADD TO TOS-1
;*
IDXW    PLA
        ASL
        CLC
        ADC     TOS,S
        STA     TOS,S
        JMP     NEXTOP16
;*
;* MUL TOS-1 BY TOS
;*
MUL     ;STY     IPY
        ;LDY     #$10
        LDA     NOS,S
        EOR     #$FFFF
        STA     TMP
        LDA     #$0000
MULLP   ASL     TMP             ;LSR     TMP             ; MULTPLR
        BCS     +
        ADC     TOS,S           ; MULTPLD
+       ASL                     ;ASL     TOS,S           ; MULTPLD
        ;DEY
        BNE     MULLP
        STA     NOS,S           ; PROD
        ;LDY     IPY
        JMP     DROP16
;*
;* INTERNAL DIVIDE ALGORITHM
;*
_NEG    EOR     #$FFFF
        INC
        SEC
        SBC     TOS,S
        STA     TOS,S
        RTS
_DIV    STY     IPY
        LDY     #$11            ; #BITS+1
        LDX     #$00
        LDA     TOS,S
        BPL     +
        LDX     #$81
        EOR     #$FFFF
        INC
        STA     TOS,S
+       LDA     NOS,S
        BPL     +
        INX
        EOR     #$FFFF
        INC
        STA     TMP             ; NOS,S
+       BEQ     _DIVEX
_DIV1   ASL                     ; DVDND
        DEY
        BCC     _DIV1
        STA     TMP             ;NOS,S           ; DVDND
        LDA     #$0000          ; REMNDR
_DIVLP  ROL                     ; REMNDR
        CMP     TOS,S           ; DVSR
        BCC     +
        SBC     TOS,S           ; DVSR
        SEC
+       ROL     TMP             ;NOS,S           ; DVDND
        DEY
        BNE     _DIVLP
_DIVEX  ;STA     TMP             ; REMNDR
        LDY     IPY
        RTS
;*
;* NEGATE TOS
;*
NEG     LDA     #$0000
        SEC
        SBC     TOS,S
        STA     TOS,S
        JMP     NEXTOP16
;*
;* DIV TOS-1 BY TOS
;*
DIV     JSR     _DIV
        LDA     TMP
        STA     NOS,S
        PLA
        TXA
        LSR                     ; SIGN(RESULT) = (SIGN(DIVIDEND) + SIGN(DIVISOR)) & 1
        BCS     NEG
        JMP     NEXTOP16
;*
;* MOD TOS-1 BY TOS
;*
MOD     JSR     _DIV
        STA     NOS,S           ; REMNDR
        PLA
        TXA
        AND     #$0080          ; REMAINDER IS SIGN OF DIVIDEND
        BNE     NEG
        JMP     NEXTOP16
;*
;* INCREMENT TOS
;*
INCR    LDA     TOS,S
        INC
        STA     TOS,S
        JMP     NEXTOP16
;*
;* DECREMENT TOS
;*
DECR    LDA     TOS,S
        DEC
        STA     TOS,S
        JMP     NEXTOP16
;*
;* BITWISE COMPLIMENT TOS
;*
COMP    LDA     #$FFFF
        EOR     TOS,S
        STA     TOS,S
        JMP     NEXTOP16
;*
;* BITWISE AND TOS TO TOS-1
;*
BAND    PLA
        AND     TOS,S
        STA     TOS,S
        JMP     NEXTOP16
;*
;* INCLUSIVE OR TOS TO TOS-1
;*
IOR     PLA
        ORA     TOS,S
        STA     TOS,S
        JMP     NEXTOP16
;*
;* EXLUSIVE OR TOS TO TOS-1
;*
XOR     PLA
        EOR     TOS,S
        STA     TOS,S
        JMP     NEXTOP16
;*
;* SHIFT TOS-1 LEFT BY TOS
;*
SHL     PLA
        TAX
        BEQ     SHL2
        LDA     TOS,S
SHL1    ASL
        DEX
        BNE     SHL1
        STA     TOS,S
SHL2    JMP     NEXTOP16
;*
;* SHIFT TOS-1 RIGHT BY TOS
;*
SHR     PLA
        TAX
        BEQ     SHR2
        LDA     TOS,S
SHR1    CMP     #$8000
        ROR
        DEX
        BNE     SHR1
        STA     TOS,S
SHR2    JMP     NEXTOP16
;*
;* LOGICAL NOT
;*
LNOT    LDA     TOS,S
        BEQ     LNOT1
        LDA     #$0001
LNOT1   DEC
        STA     TOS,S
        JMP     NEXTOP16
;*
;* LOGICAL AND
;*
LAND    PLA
        BEQ     LAND1
        LDA     TOS,S
        BEQ     LAND2
        LDA     #$FFFF
LAND1   STA     TOS,S
LAND2   JMP     NEXTOP16
;*
;* LOGICAL OR
;*
LOR     PLA
        ORA     TOS,S
        BEQ     LOR1
        LDA     #$FFFF
        STA     TOS,S
LOR1    JMP     NEXTOP16
;*
;* DUPLICATE TOS
;*
DUP     LDA     TOS,S
        PHA
        JMP     NEXTOP16
;*
;* PUSH EVAL STACK POINTER TO CALL STACK
;*
PUSHEP  TXA
        PHA                     ; MAKE SURE WE PUSH 16 BITS
        JMP     NEXTOP16
;*
;* PULL EVAL STACK POINTER FROM CALL STACK
;*
PULLEP  PLA                     ; MAKE SURE WE PULL 16 BITS
        TAX
        JMP     NEXTOP16
;*
;* CONSTANT
;*
ZERO    LDA     #$0000
        PHA
        JMP     NEXTOP16
CFFB    +INC_IP
        LDA     (IP16),Y
        ORA     #$FF00
        PHA
        JMP     NEXTOP16
CB      +INC_IP
        LDA     (IP16),Y
        AND     #$00FF
        PHA
        JMP     NEXTOP16
;*
;* LOAD ADDRESS & LOAD CONSTANT WORD (SAME THING, WITH OR WITHOUT FIXUP)
;*
LA      =       *
CW      DEX
        +INC_IP
        LDA     (IP16),Y
        PHA
        +INC_IP
        JMP     NEXTOP16
;*
;* CONSTANT STRING
;*
CS      +INC_IP
        TYA                     ; NORMALIZE IP AND SAVE STRING ADDR ON ESTK
        CLC
        ADC     IP16
        STA     IP16
        PHA
        LDA     (IP16),Y
        TAY
        JMP     NEXTOP16
;
CSX     +INC_IP
        TYA                     ; NORMALIZE IP
        CLC
        ADC     IP16
        STA     IP16
        LDY     #$00
        LDA     PP              ; SCAN POOL FOR STRING ALREADY THERE
        STA     TMP
_CMPPSX CMP     IFP             ; CHECK FOR END OF POOL
        BCC     _CMPSX          ; CHECK FOR MATCHING STRING
        BCS     _CPYSX          ; AT OR BEYOND END OF POOL, COPY STRING OVER
_CMPSX  SEP     #$20            ; 8 BIT A/M
        !AS
        STX     ALTRDOFF
        LDA     (TMP),Y         ; COMPARE STRINGS FROM AUX MEM TO STRINGS IN MAIN MEM
        STX     ALTRDON
        CMP     (IP16),Y        ; COMPARE STRING LENGTHS
        BNE     _CNXTSX1
        TAY
_CMPCSX STX     ALTRDOFF
        LDA     (TMP),Y         ; COMPARE STRING CHARS FROM END
        STX     ALTRDON
        CMP     (IP16),Y
        BNE     _CNXTSX
        DEY
        BNE     _CMPCSX
        LDA     TMPH            ; MATCH - SAVE EXISTING ADDR ON ESTK AND MOVE ON
        PHA
        LDA     TMPL
        PHA
        BRA     _CEXSX
_CNXTSX LDY     #$00
        STX     ALTRDOFF
        LDA     (TMP),Y
        STX     ALTRDON
_CNXTSX1 REP     #$20            ; 16 BIT A/M
        !AL
        AND     #$00FF
        SEC
        ADC     TMP
        STA     TMP
        BNE     _CMPPSX
        !AS
_CPYSX  LDA     (IP16),Y        ; COPY STRING FROM AUX TO MAIN MEM POOL
        TAY                     ; MAKE ROOM IN POOL AND SAVE ADDR ON ESTK
        EOR     #$FF
        CLC
        ADC     PPL
        STA     PPL
        TAX
        LDA     #$FF
        ADC     PPH
        STA     PPH
        PHA                     ; SAVE ADDRESS ON ESTK
        PHX
_CPYSX1 LDA     (IP16),Y        ; ALTRD IS ON,  NO NEED TO CHANGE IT HERE
        STA     (PP),Y          ; ALTWR IS OFF, NO NEED TO CHANGE IT HERE
        DEY
        CPY     #$FF
        BNE     _CPYSX1
        INY
_CEXSX  LDA     (IP16),Y        ; SKIP TO NEXT OP ADDR AFTER STRING
        TAY
        REP     #$20            ; 16 BIT A/M
        !AL
        JMP     NEXTOP16
;*
;* LOAD VALUE FROM ADDRESS TAG
;*
LB      TYX
        LDY     #$00
        SEP     #$20            ; 8 BIT A/M
        !AS
        LDA     (TOS,S),Y
        REP     #$20            ; 16 BIT A/M
        !AL
        AND     #$00FF
        STA     TOS,S
        TXY
        JMP     NEXTOP16
LW      TYX
        LDY     #$00
        LDA     (TOS,S),Y
        STA     TOS,S
        TXY
        JMP     NEXTOP16
;
LBX     TYX
        LDY     #$00
        STX     ALTRDOFF
        SEP     #$20            ; 8 BIT A/M
        !AS
        LDA     (TOS,S),Y
        REP     #$20            ; 16 BIT A/M
        !AL
        STX     ALTRDON
        AND     #$00FF
        STA     TOS,S
        TXY
        JMP     NEXTOP16
LWX     TYX
        LDY     #$00
        STX     ALTRDOFF
        LDA     (TOS,S),Y
        STX     ALTRDON
        STA     TOS,S
        TXY
        JMP     NEXTOP16
;*
;* LOAD ADDRESS OF LOCAL FRAME OFFSET
;*
LLA     +INC_IP
        LDA     (IP16),Y
        AND     #$00FF
        CLC
        ADC     IFP
        PHA
        JMP     NEXTOP16
;*
;* LOAD VALUE FROM LOCAL FRAME OFFSET
;*
LLB     +INC_IP
        LDA     (IP16),Y
        TYX
        TAY
        LDA     (IFP),Y
        AND     #$00FF
        PHA
        TXY
        JMP     NEXTOP16
LLW     +INC_IP
        LDA     (IP16),Y
        TYX
        TAY
        LDA     (IFP),Y
        PHA
        TXY
        JMP     NEXTOP16
;
LLBX    +INC_IP
        LDA     (IP16),Y
        TYX
        TAY
        STX     ALTRDOFF
        LDA     (IFP),Y
        STX     ALTRDON
        AND     #$00FF
        PHA
        TXY
        JMP     NEXTOP16
LLWX    +INC_IP
        LDA     (IP16),Y
        TYX
        TAY
        STX     ALTRDOFF
        LDA     (IFP),Y
        STX     ALTRDON
        PHA
        TXY
        JMP     NEXTOP16
;*
;* LOAD VALUE FROM ABSOLUTE ADDRESS
;*
LAB     +INC_IP
        LDA     (IP16),Y
        +INC_IP
        STA     TMP
        SEP     #$20            ; 8 BIT A/M
        !AS
        LDA     (TMP)
        REP     #$20            ; 16 BIT A/M
        !AL
        AND     #$00FF
        PHA
        JMP     NEXTOP16
LAW     +INC_IP
        LDA     (IP16),Y
        +INC_IP
        STA     TMP
        LDA     (TMP)
        PHA
        JMP     NEXTOP16
;
LABX    +INC_IP
        LDA     (IP16),Y
        +INC_IP
        STA     TMP
        SEP     #$20            ; 8 BIT A/M
        !AS
        STX     ALTRDOFF
        LDA     (TMP)
        STX     ALTRDON
        REP     #$20            ; 16 BIT A/M
        !AL
        AND     #$00FF
        PHA
        JMP     NEXTOP16
LAWX    +INC_IP
        LDA     (IP16),Y
        +INC_IP
        STA     TMP
        STX     ALTRDOFF
        LDA     (TMP)
        STX     ALTRDON
        PHA
        JMP     NEXTOP16
;
;*
;* STORE VALUE TO ADDRESS
;*
SB      TYX
        LDY     #$00
        SEP     #$20            ; 8 BIT A/M
        !AS
        LDA     NOS,S
        STA     (TOS,S),Y
        REP     #$20            ; 16 BIT A/M
        !AL
        TXY
        PLA
        JMP     DROP16
SW      TYX
        LDY     #$00
        LDA     NOS,S
        STA     (TOS,S),Y
        TXY
        PLA
        JMP     DROP16
;*
;* STORE VALUE TO LOCAL FRAME OFFSET
;*
SLB     +INC_IP
        LDA     (IP16),Y
        TYX
        TAY
        PLA
        SEP     #$20            ; 8 BIT A/M
        !AS
        STA     (IFP),Y
        REP     #$20            ; 16 BIT A/M
        !AL
        TXY
        JMP     NEXTOP16
SLW     +INC_IP
        LDA     (IP16),Y
        TYX
        TAY
        PLA
        STA     (IFP),Y
        TXY
        JMP     NEXTOP16
;*
;* STORE VALUE TO LOCAL FRAME OFFSET WITHOUT POPPING STACK
;*
DLB     +INC_IP
        LDA     (IP16),Y
        TYX
        TAY
        LDA     TOS,S
        SEP     #$20            ; 8 BIT A/M
        !AS
        STA     (IFP),Y
        REP     #$20            ; 16 BIT A/M
        !AL
        TXY
        JMP     NEXTOP16
DLW     +INC_IP
        LDA     (IP16),Y
        TYX
        TAY
        LDA     TOS,S
        STA     (IFP),Y
        TXY
        JMP     NEXTOP16
;*
;* STORE VALUE TO ABSOLUTE ADDRESS
;*
SAB     +INC_IP
        LDA     (IP16),Y
        +INC_IP
        STA     TMP
        PLA
        SEP     #$20            ; 8 BIT A/M
        !AS
        STA     (TMP)
        REP     #$20            ; 16 BIT A/M
        !AL
        JMP     NEXTOP16
SAW     +INC_IP
        LDA     (IP16),Y
        +INC_IP
        STA     TMP
        PLA
        STA     (TMP)
        JMP     NEXTOP16
;*
;* STORE VALUE TO ABSOLUTE ADDRESS WITHOUT POPPING STACK
;*
DAB     +INC_IP
        LDA     (IP16),Y
        +INC_IP
        STA     TMP
        SEP     #$20            ; 8 BIT A/M
        !AS
        LDA     TOS,S
        STA     (TMP)
        REP     #$20            ; 16 BIT A/M
        !AL
        JMP     NEXTOP16
DAW     +INC_IP
        LDA     (IP16),Y
        +INC_IP
        STA     TMP
        LDA     TOS,S
        STA     (TMP)
        JMP     NEXTOP16
;*
;* COMPARES
;*
ISEQ    PLA
        CMP     TOS,S
        BNE     ISFLS
ISTRU   LDA     #$FFFF
        STA     TOS,S
        JMP     NEXTOP16
;
ISNE    PLA
        CMP     TOS,S
        BNE     ISTRU
ISFLS   LDA     #$0000
        STA     TOS,S
        JMP     NEXTOP16
;
ISGE    PLA
        CMP     TOS,S
        BVS     +
        BMI     ISTRU
        BEQ     ISTRU
        BPL     ISFLS
+       BMI     ISFLS
        BEQ     ISFLS
        BPL     ISTRU
;
ISGT    PLA
        CMP     TOS,S
        BVS     +
        BMI     ISTRU
        BPL     ISFLS
+       BMI     ISFLS
        BPL     ISTRU
;
ISLE    PLA
        CMP     TOS,S
        BVS     +
        BPL     ISTRU
        BMI     ISFLS
+       BPL     ISFLS
        BMI     ISTRU
;
ISLT    PLA
        CMP     TOS,S
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
BRTRU   PLA
        BNE     BRNCH
NOBRNCH +INC_IP
        +INC_IP
        JMP     NEXTOP16
BRFLS   PLA
        BNE     NOBRNCH
BRNCH   LDA     IP16
        +INC_IP
        CLC
        ADC     (IP16),Y
        STA     IP16
        DEY
        JMP     NEXTOP16
BREQ    PLA
        CMP     TOS,S
        BNE     NOBRNCH
        BEQ     BRNCH
BRNE    PLA
        CMP     TOS,S
        BNE     BRNCH
        BEQ     NOBRNCH
BRGT    PLA
        CMP     TOS,S
        BVS     +
        BMI     BRNCH
        BPL     NOBRNCH
+       BMI     NOBRNCH
        BPL     BRNCH
BRLT    PLA
        CMP     TOS,S
        BVS     +
        BMI     NOBRNCH
        BEQ     NOBRNCH
        BPL     BRNCH
+       BMI     BRNCH
        BEQ     BRNCH
        BPL     NOBRNCH
IBRNCH  PLA
        CLC
        ADC     IP16
        STA     IP16
        JMP     NEXTOP16
;*
;* CALL INTO ABSOLUTE ADDRESS (NATIVE CODE)
;*
CALL    +INC_IP
        LDA     (IP16),Y
        STA     TMP
        +INC_IP
EMUSTK  STY     IPY
        SEC                     ; SWITCH TO EMULATED MODE
        XCE
        !AS
        TSX                     ; COPY HW EVAL STACK TO ZP EVAL STACK
        TXA
        SEC
        SBC     HWSP
        LSR
        LDX     ESP
        TAY
        BEQ     +
-       DEX
        PLA
        STA     ESTKL,X
        PLA
        STA     ESTKH,X
        DEY
        BNE     -
+       LDA     IP16H
        PHA
        LDA     IP16L
        PHA
        LDA     IPY
        PHA
        PHX
        JSR     JMPTMP
        PLY                     ; COPY RETURN VALUES TO HW EVAL STACK
        STY     ESP
        PLY
        PLA
        STA     IP16L
        PLA
        STA     IP16H
        CPX     ESP
        BEQ     +
-       LDA     ESTKH,X
        PHA
        LDA     ESTKL,X
        PHA
        INX
        CPX     ESP
        BNE     -
+       CLC                     ; SWITCH BACK TO NATIVE MODE
        XCE
        REP     #$20            ; 16 BIT A/M
        SEP     #$10            ; 8 BIT X,Y
        !AL
        LDX     #>OPTBL         ; MAKE SURE WE'RE INDEXING THE RIGHT TABLE
        STX     OP16PAGE
        JMP     NEXTOP16
;
CALLX   +INC_IP
        LDA     (IP16),Y
        STA     TMP
        +INC_IP
EMUSTKX STY     IPY
        SEC                     ; SWITCH TO EMULATED MODE
        XCE
        !AS
        TSX                     ; COPY HW EVAL STACK TO ZP EVAL STACK
        TXA
        SEC
        SBC     HWSP
        LSR
        LDX     ESP
        TAY
        BEQ     +
-       DEX
        PLA
        STA     ESTKL,X
        PLA
        STA     ESTKH,X
        DEY
        BNE     -
+       LDA     IP16H
        PHA
        LDA     IP16L
        PHA
        LDA     IPY
        PHA
        PHX
        STX     ALTRDOFF
        ;CLI UNTIL I KNOW WHAT TO DO WITH THE UNENHANCED IIE
        JSR     JMPTMP
        ;SEI UNTIL I KNOW WHAT TO DO WITH THE UNENHANCED IIE
        STX     ALTRDON
        PLY                     ; COPY RETURN VALUES TO HW EVAL STACK
        STY     ESP
        PLY
        PLA
        STA     IP16L
        PLA
        STA     IP16H
        CPX     ESP
        BEQ     +
-       LDA     ESTKH,X
        PHA
        LDA     ESTKL,X
        PHA
        INX
        CPX     ESP
        BNE     -
+       CLC                     ; SWITCH BACK TO NATIVE MODE
        XCE
        REP     #$20            ; 16 BIT A/M
        SEP     #$10            ; 8 BIT X,Y
        !AL
        LDX     #>OPXTBL         ; MAKE SURE WE'RE INDEXING THE RIGHT TABLE
        STX     OP16PAGE
        JMP     NEXTOP16

;*
;* INDIRECT CALL TO ADDRESS (NATIVE CODE)
;*
ICAL    PLA
        STA     TMP
        JMP     EMUSTK
;
ICALX   PLA
        STA     TMP
        JMP     EMUSTKX
;*
;* JUMP INDIRECT THROUGH TMP
;*
JMPTMP  JMP     (TMP)
;*
;* ENTER FUNCTION WITH FRAME SIZE AND PARAM COUNT
;*
ENTER   INY
        SEP     #$20            ; 8 BIT A/M
        !AS
        LDA     (IP16),Y
        PHA                     ; SAVE ON STACK FOR LEAVE
        REP     #$20            ; 16 BIT A/M
        !AL
        AND     #$00FF
        EOR     #$FFFF          ; ALLOCATE FRAME
        SEC
        ADC     PP
        STA     PP
        STA     IFP
        SEP     #$20            ; 8 BIT A/M
        !AS
        INY
        LDA     (IP16),Y
        ASL
        TAY
        BEQ     +
        LDX     ESP
-       LDA     ESTKH,X
        DEY
        STA     (IFP),Y
        LDA     ESTKL,X
        INX
        DEY
        STA     (IFP),Y
        BNE -
        STX     ESP
+       LDY     #$02
        REP     #$20            ; 16 BIT A/M
        !AL
        JMP     NEXTOP16
;*
;* LEAVE FUNCTION
;*
LEAVEX  STX     ALTRDOFF
        ;CLI UNTIL I KNOW WHAT TO DO WITH THE UNENHANCED IIE
LEAVE   SEP     #$20            ; 8 BIT A/M
        !AS
        TSX                     ; COPY HW EVAL STACK TO ZP EVAL STACK
        TXA
        LDX     ESP
        SEC
        SBC     HWSP
        LSR
        BEQ     +
        TAY
-       DEX
        PLA
        STA     ESTKL,X
        PLA
        STA     ESTKH,X
        DEY
        BNE     -
+       PLA                     ; DEALLOCATE POOL + FRAME
        REP     #$20            ; 16 BIT A/M
        !AL
        AND     #$00FF
        CLC
        ADC     IFP
        STA     PP
        PLA                     ; RESTORE PREVIOUS FRAME
        STA     IFP
        SEC                     ; SWITCH TO EMULATED MODE
        XCE
        RTS
;
RETX    STX     ALTRDOFF
        ;CLI UNTIL I KNOW WHAT TO DO WITH THE UNENHANCED IIE
RET     SEP     #$20            ; 8 BIT A/M
        !AS
        TSX                     ; COPY HW EVAL STACK TO ZP EVAL STACK
        TXA
        LDX     ESP
        SEC
        SBC     HWSP
        LSR
        BEQ     +
        TAY
-       DEX
        PLA
        STA     ESTKL,X
        PLA
        STA     ESTKH,X
        DEY
        BNE     -
+       REP     #$20            ; 16 BIT A/M
        !AL
        LDA     IFP             ; DEALLOCATE POOL
        STA     PP
        PLA                     ; RESTORE PREVIOUS FRAME
        STA     IFP
        SEC                     ; SWITCH TO EMULATED MODE
        XCE
        !AS
        RTS
VMEND   =       *
}

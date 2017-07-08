;****************************************************************
;*
;*                  PWM SOUND ROUTINES
;*
;****************************************************************
;*
;* PWM ZERO PAGE LOCATIONS
;*
SPEAKER =       $C030
HFO     =       $08
LFO     =       $09
LFOINDEX=       $0A             ; IF LFOUSRH == 0
LFOUSRL =       $0A
LFOUSRH =       $0B
ATK     =       $0C
DCY     =       $0D
SUS     =       $0E
RLS     =       $0F
ATKINCL =       $10
ATKINCH =       $11
DCYINCL =       $12
DCYINCH =       $13
RLSINCL =       $14
RLSINCH =       $15
ADSRL   =       $16
ADSRH   =       $17
ADSRINCL=       $18
ADSRINCH=       $19
TONELEN =       $1B
LPCNT   =       $1C
HFOCNT  =       $1D
LFOPOSL =       $1E
LFOPOSH =       $1F
LFOPTR  =       $00
LFOPTRL =       LFOPTR
LFOPTRH =       LFOPTRL+1
;*
;* PWM ENTRY POINT
;*
HILOPWM LDA     LFOUSRL
        LDX     LFOUSRH
        BNE     +               ; USER SUPPLIED WAVEFORM
        LDX     #>LFOTBL
        ASL
        ASL
        ASL
        ASL
        ASL
+       STA     LFOPTRL
        STX     LFOPTRH
        PHP
        SEI
        LDY     #$00
        STY     LFOPOSL
;        STY     LFOPOSH
        STY     LPCNT
        STY     ADSRL
;        STY     ADSRH
        LDA     #$02
        STA     HFOCNT
ATTACK  LDX     #$0F
        LDA     ATK
        BEQ     DECAY
        LDX     #$00
        STA     TONELEN
        LDA     ATKINCL
        STA     ADSRINCL
        LDA     ATKINCH
        STA     ADSRINCH
        JSR     HILOSND
DECAY   LDA     DCY
        BEQ     SUSTAIN
        STA     TONELEN
        LDA     #$00            ; REVERSE ATTACK RATE
        SEC
        SBC     DCYINCL
        STA     ADSRINCL
        LDA     #$00
        SBC     DCYINCH
        STA     ADSRINCH
        JSR     HILOSND
SUSTAIN LDA     SUS
        BEQ     RELEASE
        STA     TONELEN
        LDA     #$00            ; SUSTAIN DOESN'T ALTER VOLUME
        STA     ADSRINCL
        STA     ADSRINCH
        JSR     HILOSND
RELEASE LDA     RLS
        BEQ     PWMEXIT
        STA     TONELEN
        LDA     #$00            ; REVERSE RELEASE RATE
        SEC
        SBC     RLSINCL
        STA     ADSRINCL
        LDA     #$00
        SBC     RLSINCH
        STA     ADSRINCH
        JSR     HILOSND
PWMEXIT PLP
        RTS
PWMSND  CLC                     ; 1, 2
        LDA     ADSRL           ; 2, 3
        ADC     ADSRINCL        ; 2, 3
        STA     TMP             ; 2, 3
        TXA                     ; 1, 2
        ADC     ADSRINCH        ; 2, 3
        DEC     LPCNT           ; 2, 5
                                ;------
                                ;12,21

        BNE     HILOSND         ; 2, 2
        AND     #$0F            ; 2, 2
        TAX                     ; 1, 2
        LDA     TMP             ; 2, 3
        STA     ADSRL           ; 2, 3
        DEC     TONELEN         ; 2, 5
        BEQ     PWMRET          ; 2, 2
        DEC     HFOCNT          ; 2, 5
        BEQ     SPKRON          ; 2, 2
        CLC                     ; 1, 2
        LDA     LFOPOSL         ; 2, 3
        ADC     LFO             ; 2, 3
        STA     LFOPOSL         ; 2, 3
        TYA                     ; 1, 2
        ADC     #$00            ; 2, 2
        AND     #$1F            ; 2, 2
        TAY                     ; 1, 2
        TXA                     ; 1, 2
        ORA     (LFOPTR),Y      ; 2, 5
        STA     *+4             ; 3, 4
        LDA     MUL4X4          ; 3, 4
        ASL                     ; 1, 2
        STA     SPKRPWM+1       ; 3, 4
        LDA     #>PWM1          ; 2, 2
        ADC     #$00            ; 2, 2
        STA     SPKRPWM+2       ; 3, 4
        NOP                     ; 1, 2
        JMP     PWMSND          ; 3, 3
                                ;------
                                ;55,79

;       BNE     HILOSND         ;  , 3
HILOSND DEC     HFOCNT          ; 2, 5
        BNE     +               ; 2, 2
SPKRON  BIT     SPEAKER         ; 3, 4
SPKRPWM JMP     PWM1            ; 3, 3+62
                                ;------
                                ;10,79

;       BNE     HILOSND         ;  , 3
;       DEC     HFOCNT          ;  , 5
;       BNE     +               ;  , 3
+       BNE     ++              ; 2, 3
++      NOP                     ; 1, 2
        NOP                     ; 1, 2
        NOP                     ; 1, 2
        NOP                     ; 1, 2
        NOP                     ; 1, 2
        NOP                     ; 1, 2
        NOP                     ; 1, 2
        CLC                     ; 1, 2
        LDA     LFOPOSL         ; 2, 3
        ADC     LFO             ; 2, 3
        STA     LFOPOSL         ; 2, 3
        TYA                     ; 1, 2
        ADC     #$00            ; 2, 2
        AND     #$1F            ; 2, 2
        TAY                     ; 1, 2
        TXA                     ; 1, 2
        ORA     (LFOPTR),Y      ; 2, 5
        STA     *+4             ; 3, 4
        LDA     MUL4X4          ; 3, 4
        ASL                     ; 1, 2
        STA     SPKRPWM+1       ; 3, 4
        LDA     #>PWM1          ; 2, 2
        ADC     #$00            ; 2, 2
        STA     SPKRPWM+2       ; 3, 4
        JMP     PWMSND          ; 3, 3
                                ;------
                                ;44,79
PWMRET  RTS
;*
;* 4 BIT x 4 BIT TO 3.5 BIT MULTIPLY TABLE
;*
        !ALIGN  255,0
MUL4X4  !BYTE   $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00
        !BYTE   $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00
        !BYTE   $00, $00, $00, $00, $00, $00, $00, $00, $20, $20, $20, $20, $20, $20, $20, $20
        !BYTE   $00, $00, $00, $00, $00, $00, $20, $20, $20, $20, $20, $20, $20, $20, $20, $20
        !BYTE   $00, $00, $00, $00, $20, $20, $20, $20, $20, $20, $20, $20, $40, $40, $40, $40
        !BYTE   $00, $00, $00, $00, $20, $20, $20, $20, $20, $20, $40, $40, $40, $40, $40, $40
        !BYTE   $00, $00, $00, $20, $20, $20, $20, $20, $40, $40, $40, $40, $40, $40, $60, $60
        !BYTE   $00, $00, $00, $20, $20, $20, $20, $40, $40, $40, $40, $40, $60, $60, $60, $60
        !BYTE   $00, $00, $20, $20, $20, $20, $40, $40, $40, $40, $60, $60, $60, $60, $80, $80
        !BYTE   $00, $00, $20, $20, $20, $20, $40, $40, $40, $60, $60, $60, $60, $80, $80, $80
        !BYTE   $00, $00, $20, $20, $20, $40, $40, $40, $60, $60, $60, $60, $80, $80, $80, $A0
        !BYTE   $00, $00, $20, $20, $20, $40, $40, $40, $60, $60, $60, $80, $80, $80, $A0, $A0
        !BYTE   $00, $00, $20, $20, $40, $40, $40, $60, $60, $60, $80, $80, $A0, $A0, $A0, $C0
        !BYTE   $00, $00, $20, $20, $40, $40, $40, $60, $60, $80, $80, $80, $A0, $A0, $C0, $C0
        !BYTE   $00, $00, $20, $20, $40, $40, $60, $60, $80, $80, $80, $A0, $A0, $C0, $C0, $E0
        !BYTE   $00, $00, $20, $20, $40, $40, $60, $60, $80, $80, $A0, $A0, $C0, $C0, $E0, $E0
LFOTBL  !SOURCE "lfotbl.s"
        !ALIGN  63,0
PWM1    BIT     SPEAKER         ; 3, 4
        CLC                     ; 1, 2
        LDA     LFOPOSL         ; 2, 3
        ADC     LFO             ; 2, 3
        STA     LFOPOSL         ; 2, 3
        TYA                     ; 1, 2
        ADC     #$00            ; 2, 2
        AND     #$1F            ; 2, 2
        TAY                     ; 1, 2
        TXA                     ; 1, 2
        ORA     (LFOPTR),Y      ; 2, 5
        STA     *+4             ; 3, 4
        LDA     MUL4X4          ; 3, 4
        ASL                     ; 1, 2
        STA     SPKRPWM+1       ; 3, 4
        LDA     #>PWM1          ; 2, 2
        ADC     #$00            ; 2, 2
        STA     SPKRPWM+2       ; 3, 4
        LDA     HFO             ; 2, 3
        STA     HFOCNT          ; 2, 3
        JMP     PWMSND          ; 3, 3
                                ;------
                                ;43,61
        !ALIGN  63,0
PWM2    CLC                     ; 1, 2
        LDA     LFOPOSL         ; 2, 3
        BIT     SPEAKER         ; 3, 4
        ADC     LFO             ; 2, 3
        STA     LFOPOSL         ; 2, 3
        TYA                     ; 1, 2
        ADC     #$00            ; 2, 2
        AND     #$1F            ; 2, 2
        TAY                     ; 1, 2
        TXA                     ; 1, 2
        ORA     (LFOPTR),Y      ; 2, 5
        STA     *+4             ; 3, 4
        LDA     MUL4X4          ; 3, 4
        ASL                     ; 1, 2
        STA     SPKRPWM+1       ; 3, 4
        LDA     #>PWM1          ; 2, 2
        ADC     #$00            ; 2, 2
        STA     SPKRPWM+2       ; 3, 4
        LDA     HFO             ; 2, 3
        STA     HFOCNT          ; 2, 3
        JMP     PWMSND          ; 3, 3
                                ;------
                                ;43,62
        !ALIGN  63,0
PWM3    CLC                     ; 1, 2
        LDA     LFOPOSL         ; 2, 3
        ADC     LFO             ; 2, 3
        STA     LFOPOSL         ; 2, 3
        BIT     SPEAKER         ; 3, 4
        TYA                     ; 1, 2
        ADC     #$00            ; 2, 2
        AND     #$1F            ; 2, 2
        TAY                     ; 1, 2
        TXA                     ; 1, 2
        ORA     (LFOPTR),Y      ; 2, 5
        STA     *+4             ; 3, 4
        LDA     MUL4X4          ; 3, 4
        ASL                     ; 1, 2
        STA     SPKRPWM+1       ; 3, 4
        LDA     #>PWM1          ; 2, 2
        ADC     #$00            ; 2, 2
        STA     SPKRPWM+2       ; 3, 4
        LDA     HFO             ; 2, 3
        STA     HFOCNT          ; 2, 3
        JMP     PWMSND          ; 3, 3
                                ;------
                                ;43,61
        !ALIGN  63,0
PWM4    CLC                     ; 1, 2
        LDA     LFOPOSL         ; 2, 3
        ADC     LFO             ; 2, 3
        STA     LFOPOSL         ; 2, 3
        TYA                     ; 1, 2
        ADC     #$00            ; 2, 2
        AND     #$1F            ; 2, 2
        TAY                     ; 1, 2
        BIT     SPEAKER         ; 3, 4
        TXA                     ; 1, 2
        ORA     (LFOPTR),Y      ; 2, 5
        STA     *+4             ; 3, 4
        LDA     MUL4X4          ; 3, 4
        ASL                     ; 1, 2
        STA     SPKRPWM+1       ; 3, 4
        LDA     #>PWM1          ; 2, 2
        ADC     #$00            ; 2, 2
        STA     SPKRPWM+2       ; 3, 4
        LDA     HFO             ; 2, 3
        STA     HFOCNT          ; 2, 3
        JMP     PWMSND          ; 3, 3
                                ;------
                                ;43,61
        !ALIGN  63,0
PWM5    CLC                     ; 1, 2
        LDA     LFOPOSL         ; 2, 3
        ADC     LFO             ; 2, 3
        STA     LFOPOSL         ; 2, 3
        TYA                     ; 1, 2
        ADC     #$00            ; 2, 2
        AND     #$1F            ; 2, 2
        TAY                     ; 1, 2
        TXA                     ; 1, 2
        ORA     (LFOPTR),Y      ; 2, 5
        BIT     SPEAKER         ; 3, 4
        STA     *+4             ; 3, 4
        LDA     MUL4X4          ; 3, 4
        ASL                     ; 1, 2
        STA     SPKRPWM+1       ; 3, 4
        LDA     #>PWM1          ; 2, 2
        ADC     #$00            ; 2, 2
        STA     SPKRPWM+2       ; 3, 4
        LDA     HFO             ; 2, 3
        STA     HFOCNT          ; 2, 3
        JMP     PWMSND          ; 3, 3
                                ;------
                                ;43,61
        !ALIGN  63,0
PWM6    CLC                     ; 1, 2
        LDA     LFOPOSL         ; 2, 3
        ADC     LFO             ; 2, 3
        STA     LFOPOSL         ; 2, 3
        TYA                     ; 1, 2
        ADC     #$00            ; 2, 2
        AND     #$1F            ; 2, 2
        TAY                     ; 1, 2
        TXA                     ; 1, 2
        ORA     (LFOPTR),Y      ; 2, 5
        STA     *+4             ; 3, 4
        LDA     MUL4X4          ; 3, 4
        BIT     SPEAKER         ; 3, 4
        ASL                     ; 1, 2
        STA     SPKRPWM+1       ; 3, 4
        LDA     #>PWM1          ; 2, 2
        ADC     #$00            ; 2, 2
        STA     SPKRPWM+2       ; 3, 4
        LDA     HFO             ; 2, 3
        STA     HFOCNT          ; 2, 3
        JMP     PWMSND          ; 3, 3
                                ;------
                                ;43,61
        !ALIGN  63,0
PWM7    CLC                     ; 1, 2
        LDA     LFOPOSL         ; 2, 3
        ADC     LFO             ; 2, 3
        STA     LFOPOSL         ; 2, 3
        TYA                     ; 1, 2
        ADC     #$00            ; 2, 2
        AND     #$1F            ; 2, 2
        TAY                     ; 1, 2
        TXA                     ; 1, 2
        ORA     (LFOPTR),Y      ; 2, 5
        STA     *+4             ; 3, 4
        LDA     MUL4X4          ; 3, 4
        ASL                     ; 1, 2
        STA     SPKRPWM+1       ; 3, 4
        LDA     #>PWM1          ; 2, 2
        ADC     #$00            ; 2, 2
        BIT     SPEAKER         ; 3, 4
        STA     SPKRPWM+2       ; 3, 4
        LDA     HFO             ; 2, 3
        STA     HFOCNT          ; 2, 3
        JMP     PWMSND          ; 3, 3
                                ;------
                                ;43,61
        !ALIGN  63,0
PWM8    CLC                     ; 1, 2
        LDA     LFOPOSL         ; 2, 3
        ADC     LFO             ; 2, 3
        STA     LFOPOSL         ; 2, 3
        TYA                     ; 1, 2
        ADC     #$00            ; 2, 2
        AND     #$1F            ; 2, 2
        TAY                     ; 1, 2
        TXA                     ; 1, 2
        ORA     (LFOPTR),Y      ; 2, 5
        STA     *+4             ; 3, 4
        LDA     MUL4X4          ; 3, 4
        ASL                     ; 1, 2
        STA     SPKRPWM+1       ; 3, 4
        LDA     #>PWM1          ; 2, 2
        ADC     #$00            ; 2, 2
        STA     SPKRPWM+2       ; 3, 4
        LDA     HFO             ; 2, 3
        STA     HFOCNT          ; 2, 3
        BIT     SPEAKER         ; 3, 4
        JMP     PWMSND          ; 3, 3
                                ;------
                                ;43,61

SRC" PLASMA.4TH"
SRC" CONIO.4TH"

0 VARIABLE K
0 VARIABLE W
0 VARIABLE FMI
0 VARIABLE FMK

: DOROD
  BEGIN
    51 3 DO ( for w = 3 to 50 )
      I W !
      20 1 DO ( for i = 1 to 19 )
        20 0 DO ( for j = 0 to 19 )
          ( Note: i -> J, j -> I )
          I 3 * J 3 + / J W @ * 12 / +
          COLOR
          J I + K !
          40 J - FMI !
          40 K @ - FMK !
          J K @ PLOT
          K @ J PLOT
          FMI @ FMK @ PLOT
          FMK @ FMI @ PLOT
          K @ FMI @ PLOT
          FMI @ K @ PLOT
          J FMK @ PLOT
          FMK @ J PLOT
          ?TERMINAL IF ( if keypressed )
            KEY DROP
            R> DROP R> DROP ( clean up DO-OKIE )
            R> DROP R> DROP
            R> DROP R> DROP
            EXIT ( return )
          THEN
        LOOP
      LOOP
    LOOP
  AGAIN
;
: ROD
  GR
    DOROD
  TEXT
;

ROD
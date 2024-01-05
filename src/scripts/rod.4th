: ?PLASMA
  " IFACE" FIND
  SWAP DROP
  0= IF
    " PLASMA.4TH" SRC
  THEN
;

?PLASMA ( Load PLASMA if not already )

: ?CONIO
  " CONIOAPI" FIND
  SWAP DROP
  0= IF
    " CONIO.4TH" SRC
  THEN
;

?CONIO ( Load CONIO if not already )

0 VARIABLE K
0 VARIABLE W
0 VARIABLE FMI
0 VARIABLE FMK

: RODINNER
  20 1 DO                                 ( for i = 1 to 19 )
    20 0 DO                               ( for j = 0 to 19 )
      ( Note: i -> J, j -> I )
      I 3 * J 3 + / J W @ * 12 / +        ( color = {j * 3} / {i + 3} + i * w / 12 )
      COLOR                               ( grcolor{color} )
      J I + K !                           ( k = i + j )
      40 J - FMI !                        ( fmi = 40 - i )
      40 K @ - FMK !                      ( fmk = 40 - k )
      J K @ PLOT                          ( grplot{i, k} )
      K @ J PLOT                          ( grplot{k, i} )
      FMI @ FMK @ PLOT                    ( grplot{fmi, fmk} )
      FMK @ FMI @ PLOT                    ( grplot{fmk, fmi} )
      K @ FMI @ PLOT                      ( grplot{k, fmi} )
      FMI @ K @ PLOT                      ( grplot{fmi, k} )
      J FMK @ PLOT                        ( grplot{i, fmk} )
      FMK @ J PLOT                        ( grplot{fmk, i} )
    LOOP                                  ( next )
    ?TERMINAL IF                          ( if keypressed )
      LEAVE                               ( return )
    THEN                                  ( fin )
  LOOP                                    ( next )
;
: ROD
  GR
  BEGIN
    51 3 DO                               ( for w = 3 to 50 )
      I W !
      RODINNER
      ?TERMINAL IF LEAVE THEN
    LOOP                                  ( next )
  ?TERMINAL
  UNTIL
  KEY DROP
  TEXT
;

ROD
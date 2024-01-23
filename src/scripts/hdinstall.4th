SRC" plasma.4th"
SRC" conio.4th"
: RESUME ;
: ?EXEC
  NOT IF
    1 >R
    BEGIN
      BL WORD FIND IF
        CASE
          ' RESUME OF
            R> 1- -DUP 0= IF EXIT THEN
            >R
            ENDOF
          ' ?EXEC OF
            R> 1+ >R
            ENDOF
        ENDCASE
      ELSE DROP
      THEN
    AGAIN
  THEN
;

HOME
." Copying system files to /RAM4..." CR
  COPY" -R PLVM16 CMD128 SYS /RAM4"
." Copy demos (Y/N)?"
KEY CR TOUPPER CHAR Y = ?EXEC
  COPY" -R DEMOS /RAM4"
RESUME
." Copy build tools (Y/N)?"
KEY CR TOUPPER CHAR Y = ?EXEC
  NEWDIR" /RAM4/BLD
  COPY" BLD/PLASM BLD/CODEOPT /RAM4/BLD"
  COPY" -R BLD/INC /RAM4/BLD"
  ." Copy sample code (Y/N)?"
  KEY CR TOUPPER CHAR Y = ?EXEC
    COPY" -R BLD/SAMPLES /RAM4/BLD"
  RESUME
RESUME
." Done" CR
BYE

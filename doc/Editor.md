WELCOME TO THE PLASMA EDITOR!
=============================

FIRST THINGS FIRST:
TO NAVIGATE, USE THE ARROW KEYS.

ON THE APPLE ][:

    CTRL-K = UP
    CTRL-J = DOWN.

TO JUMP AROUND THE TEXT FILE USE:

    CTRL-W = JUMP UP
    CTRL-Z = JUMP DOWN
    CTRL-A = JUMP LEFT
    CTRL-S = JUMP RIGHT

    CTRL-Q = JUMP BEGINNING
    CTRL-E = JUMP END

THE 'ESCAPE' KEY WILL PUT YOU IN
COMMAND MODE.  FROM THERE YOU CAN
EXIT BY ENTERING 'Q' AND 'RETURN'.
YOU CAN ALSO RETURN TO THE EDITOR BY
JUST PRESSING 'RETURN'.

-------------------------------------------------------------------------

THE PLASMA EDITOR IS A SIMPLE TEXT EDITOR FOR ENTERING AND MANIPULATING
TEXT AND SOURCE CODE FILES. THE EDITOR SUPPORTS FROM 40 TO 128 COLUMN MODE
WITH LINES THAT CAN BE UP TO 127 CHARACTERS LONG. THE SCREEN WILL SCROLL
HORIZONTALLY AS THE CURSOR MOVES, IF NEDED. THERE IS ABOUT 16K TO 30K OF
MEMORY FOR THE TEXT BUFFER, DEPENDING ON YOUR CONFIGURATION.

IT HAS TWO MODES, COMMAND AND EDIT.

EDIT COMMANDS:

    LEFT ARROW   = MOVE CHAR LEFT
    RIGHT ARROW  = MOVE CHAR RIGHT
    UP ARROW     = MOVE LINE UP
    DOWN ARROW   = MOVE LINE DOWN
    CTRL-K       = MOVE LINE UP
    CTRL-J       = MOVE LINE DOWN
    CTRL-A       = JUMP LEFT
    CTRL-S       = JUMP RIGHT
    CTRL-W       = JUMP UP
    CTRL-Z       = JUMP DOWN
    CTRL-Q       = JUMP BEGIN
    CTRL-E       = JUMP END
    CTRL-D       = DELETE CHAR
    CTRL-B       = BEGIN SELECTION
    CTRL-C       = COPY SELECTION INTO CLIPBOARD
    CTRL-X       = CUT SELECTION INTO CLIPBOARD
    CTRL-V       = PASTE CLIPBOARD
    CTRL-O       = OPEN NEW LINE
    CTRL-F       = OPEN A FOLLOWING NEW LINE
    CTRL-Y       = JOIN LINES
    CTRL-T       = TOGGLE INSERT/OVERWRITE
    TAB/CTRL-I   = INSERT SPACES TO NEXT TAB
                 = INDENT SELECTION IF INSERT MODE
                 = UNDENT SELECTION IF OVERWITE MODE
    ESCAPE       = SWITCH TO COMMAND MODE
    DELETE       = DELETE CHAR LEFT

  APPLE ][ FEATURES:
  ------------------

    SHIFT-M      = ]
    CTRL-N       = [
    SHIFT-CTRL-N = ~
    CTRL-P       = \
    SHIFT-CTRL-P = |
    CTRL-G       = _
    CTRL-L       = SHIFT LOCK
    SHIFT-LEFT ARROW = DELETE (SHIFT-MOD)

  WITH THE SHIFT-KEY MOD ON AN APPLE ][, UPPER AND LOWER CASE ENTRY WORKS
  AS EXPECTED.

    ESC T C      = FORCE LOWER-CASE CHARS

  If you have a lower-case character generator installed, you can force
  lower-case display.  Otherwise, upper case will be displayed normally
  but lower-case will be displayed in inverse.  This is the default.

  Apple //e AND //c FEATURES:
  ---------------------------

  The 'SOLID-APPLE' key will modify these keys:

    SA-RETURN      = OPEN FOLLOWING LINE
    SA-LEFT ARROW  = JUMP LEFT
    SA-RIGHT ARROW = JUMP RIGHT
    SA-UP ARROR    = JUMP UP
    SA-DOWN ARROW  = JUMP DOWN
    SA-TAB         = DETAB

  Apple /// FEATURES:
  -------------------

  The 'OPEN-APPLE' key will modify these keys:

    OA-\           = DELETE CHAR LEFT
    OA-RETURN      = OPEN FOLLOWING LINE
    OA-LEFT ARROW  = JUMP LEFT
    OA-RIGHT ARROW = JUMP RIGHT
    OA-UP ARROR    = JUMP UP
    OA-DOWN ARROW  = JUMP DOWN
    OA-TAB         = DETAB

  Apple //e Platinum AND /// FEATURES:
  ------------------------------------

  On the keypad, 'OPTION' on //e Platinum or 'OPEN-APPLE' on the ///
  allows the keys for navigation and misc operations:

    OA-4           = MOVE CHAR LEFT
    OA-6           = MOVE CHAR RIGHT
    OA-8           = MOVE LINE UP
    OA-2           = MOVE LINE DOWN
    OA-9           = JUMP UP
    OA-3           = JUMP DOWN
    OA-7           = JUMP BEGIN
    OA-1           = JUMP END
    OA-5           = SELECT/DESELECT
    OA--           = CUT SELECTION INTO CLIPBOARD
    OA-+           = COPY SELECTION INTO CLIPBOARD
    OA-0           = DELETE CHARACTER
    OA-/           = INSERT/OVERWRITE
    OA-*           = OPEN FOLLOWING LINE
    OA-ENTER       = OPEN FOLLOWING LINE
    OA-.           = PASTE CLIPBOARD
    OA-=           = NUMLOCK (NUMBER KEYS ACT LIKE OA/OPTION PRESSED)

COMMAND MODE:

    <REQUIRED PARAMETER>
    [OPTIONAL PARAMETER]

    Q            = QUIT
    R <FILENAME> = READ FILE
    W [FILENAME] = WRITE FILE (OPTIONAL FILENAME)
    A [FILENAME] = APPEND FILE
    C [PREFIX]   = CATALOG FILES
    P <PREFIX>   = SET PREFIX
    H [SLOT]     = HARDCOPY TO DEVICE IN SLOT (DEFAULT 1)
    N            = CLEAR TEXT IN MEMORY
    T G          = TOGGLE GUTTER VIEW
    T C          = TOGGLE LOWER-CASE SUPPORT (APPLE ][)
    F [STRING]   = FIND STRING
    E            = EDIT MODE
    'RETURN'     = EDIT MODE
    1..999       = GO TO LINE #
    
: IFACE 2 * + @ ;

LOOKUP CMDSYS 3 IFACE PLASMA   EXECMOD
LOOKUP CMDSYS 2 IFACE CONSTANT CMDLINE
LOOKUP STRCPY PLASMA STRCPY
LOOKUP STRCAT PLASMA STRCAT

: CPYCMD
    CMDLINE " . "  STRCPY DROP ( Need a dummy value for the module name )
    34 WORD CMDLINE SWAP STRCAT DROP ( Quote deliminted string )
;

: CMDEXEC
    CPYCMD
    EXECMOD 0< IF ." Failed to exec module" CR THEN
;

: LOADMOD"
    34 WORD ( Quote deliminted string )
    EXECMOD 0< IF ." Failed to load module" CR THEN
;

: EDIT"
    CPYCMD
    " ED" EXECMOD 0< IF ." Failed to run ED" CR ABORT THEN
;

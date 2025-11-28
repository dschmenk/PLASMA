# FORTH PLASMA + PLFORTH !

PLFORTH represents a REPL and scripting language for the PLASMA environment. Or, what I did over the Holiday break.

The goals of PLFORTH are pretty straight forward:
**Interactivity** and **debugging**.

PLFORTH is a PLASMA module written in PLASMA itself. As a first class citizen of the PLASMA environment, it has instant access to all the PLASMA modules, from floating point to high-res graphics libraries and everything in between.

## Missing words in PLFORTH

There are quite a few missing word that a standard FORTH would have. Mostly due to deliberately keeping PLFORTH as minimal as possible to reduce the memory footpring and load time. Most of the missing words can be synthesized using existing PLASMA modules and some glue words. The double word have mostly been made avialable through PLASMA's 32 bit integer module, `INT32` by way of the `int32.4th` script. You can always petition to get your favorite FORTH word included in the default vocabulary. Speaking of `VOCABULARY`, PLFORTH only has one.

## PLFORTH built-in words

    Compile only:  LITERAL ; EXIT POSTPONE COMPILE, [ DOES> REPEAT WHILE UNTIL
    AGAIN BEGIN J I +LOOP LOOP UNLOOP LEAVE DO ENDCASE ENDOF OF CASE THEN ELSE IF
    RECURSE

    Interpret only:  PBC ITC BRKOFF BRKON SEE CONT ] IS DEFER :NONAME : FORGET
    CONSTANT VARIABLE PLASMA

    Both:  WORDS BRK STEPOFF STEPON TROFF TRON .RS .S BYE ( COLD ?ABORT" (?ABORT")
    ?ABORT QUIT ENDSRC ?ENDSRC SRC" SRC .( ." (.") " CHAR BL TYPE SPACES SPACE CR
    EMIT C%. %. C$. $. . ? COMPARE -TRAILING NUM? WORD ACCEPT KEY KEY? ' FIND COUNT
    IMMEDIATE INTERPONLY COMPONLY STATE C, , CREATE ALLOT PAD HERE LATEST FILL
    CMOVE LOOKUP R@ R> >R EXECUTE @ C@ +! ! C! MAX MIN ABS 0<> 0= 0> 0< U< U> < >
    <> = RSHIFT LSHIFT NOT COMPLEMENT XOR OR AND NEGATE MOD /MOD / * - 2- 1- 2+ 1+
    + ROT OVER ?DUP DUP2 DUP SWAP DROP2 DROP OK

## PLFORTH specific words

### Words for looking at internal structures:

`.RS`: Displays the return stack. Note: PLFORTH uses a software defined return stack, this is not the hardware stack

### Words for tracing and single stepping execution:

`TRON`: Turn tracing on

`TROFF`: Turn tracing off (will also turn off single stepping if enabled)

`STEPON`: Turn single stepping on

`STEPOFF`: Turn single stepping off

While running code, `<CTRL-T>` will toggle tracing on and off as well

### Words for breakpoints:

`BRK`: Used inside compiled word to effect a runtime break

`BRKON xxxx`: Enable breakpoint whenever word `xxxx` is executed

`BRKOFF`: Disable the breakpoint. Note: only one breakpoint is currently supported

While running code, `<CTRL-C>` will break out and return to the interpreter.

`CONT`: Continue running from the last break point

### Words for PLASMA linkage:

`LOOKUP yyyy`: Lookup symbol `yyyy` in PLASMA symbol table and return its address

`PLASMA zzzz`: Create word `zzzz` with code address from `LOOKUP`

### Words to run a script:

`SRC`: Source filename on stack as input. Can be nested

`SRC" ssss"`: Source file `ssss` as input. Can be nested

`?ENDSRC`: End sourcing file as input if stack flag non-zero

`ENDSRC`: End sourcing file as input

### Words for compiler modes:

`PBC`: Compile into PLASMA Byte Code

`ITC`: Compile into Indirect Threaded Code

### Word for converting string to number:

`NUM?`: Convert string and length to number, returning number and valid flag

Numbers entered with a preceeding `$` will be interpreted as hex values

Numbers entered with a preceeding `%` will be interpreted as binary values

### Words for displaying hex numbers

`$.`: Display TOS word in hex with leading `$`

`C$.`: Display TOS byte in hex with leading `$`

Hex numbers can be entered when preceded by `$`

### Words for displaying binary numbers

`%.`: Display TOS word in binary with leading `%`

`C%.`: Display TOS byte in binary with leading `%`

Binary numbers can be entered when preceded by `%`

## Debugging vs Performance

PLFORTH defaults to compiling using PLASMA byte code (PBC) to greatly improve performance, but most of the debugging tools are lost. However, the compiler can easily switch to ITC (Indirect Threaded Code). ITC can be set as the initial mode by passing a `-D` flag on the command line. This supports a list of inspection and debugging features while developing programs and scripts. ITC compiled words and PBC compiled words can be intermingled and call each other seemlessly. PLASMA Byte Code is a direct match to many low-level FORTH constructs.

## Graphics
Due to the way the Apple II implements Hi-Res, Lo-Res and Double Lo-Res graphics, a stub loader is required to reserve the pages used.

`HRFORTH`: Reserve HGR page 1 before launching PLFORTH

`HR2FORTH`: Reserve HGR pages 1 and 2 before launching PLFORTH

`TX2FORTH`: Reserve GR and DGR pages 1 and 2 before launching PLFORTH

## Scripts

There are a few useful scripts located in the `scripts` directory. By far the most useful is `plasma.4th`

### plasma.4th useful words

`CAT`: Display files in current ProDOS directory

`CAT" pppp"`: Display files in `pppp` ProDOS directory

`PFX" pppp"`: Set current ProDOS prefix to `pppp`

`PFX.`: Display current ProDOS prefix

`EDIT" ssss"`: Edit file `ssss`

## Command line options

`-F`: Fast flag (like `PCB` as first command)

`-T`: Trace flag (like `TRON` as first command)

`SCRIPT NAME`: Soure filename to execute

## Links

Here is a (worse than usual) video running through some examples. This was a preliminary release so the final is a bit different: https://youtu.be/picPyXAk77I?si=Td2En5Z3oxVTzh0z

A pre-configured ProDOS floppy image able to run  and a few utilities is available here: https://github.com/dschmenk/PLASMA/blob/master/images/apple/PLFORTH.po

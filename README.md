# 4/29/2018 PLASMA 1.2 Available!
[Download and read the Release Notes](https://github.com/dschmenk/PLASMA/blob/master/doc/Version%201.2.md)

# The PLASMA Programming Language

![Luc Viatour](https://upload.wikimedia.org/wikipedia/commons/thumb/2/26/Plasma-lamp_2.jpg/1200px-Plasma-lamp_2.jpg)
image credit: Luc Viatour / www.Lucnix.be

PLASMA: **P**roto **L**anguage **A**s**S**e**M**bler for **A**ll

PLASMA is a medium level programming language targeting the 8-bit 6502 processor. Historically, there were simple languages developed in the early years of computers that improved on the tedium of assembly language programming while still being low level enough for system coding. Languages like B, FORTH, and PLASMA fall into this category.

PLASMA is a combination of operating environment, virtual machine, and assembler/compiler matched closely to the 6502 architecture.  It is an attempt to satisfy a few challenges surrounding code size, efficient execution, small runtime and flexible code location.  By architecting a unique bytecode that maps nearly one-to-one to the higher-level representation, the compiler can be very simple and execute quickly on the Apple II for a self-hosted environment.  A modular approach provides for incremental development and code reuse. The syntax of the language is heavily influenced by assembly, Pascal, and C. The design philosophy was to be as simple as feasible while retaining flexibility and semantic clarity. You won't find any unnecessary or redundant syntax in PLASMA.

Different projects have led to the architecture of PLASMA, most notably Apple Pascal, FORTH, and my own Java VM for the 6502: VM02. Each has tried to map a generic VM to the 6502 with varying levels of success.  Apple Pascal, based on the USCD Pascal using the p-code interpreter, was a very powerful system and ran fast enough on the Apple II to be interactive but didn't win any speed contests. FORTH was the poster child for efficiency and obtuse syntax. Commonly referred to as a write only language, it was difficult to come up to speed as a developer, especially when using others' code. My own project in creating a Java VM for the Apple II uncovered the folly of shoehorning a large, 32-bit virtual memory environment into 8-bit, 64K hardware.

## Contents

<!-- TOC depthFrom:1 depthTo:6 withLinks:1 updateOnSave:1 orderedList:0 -->

- [Build Environment](#build-environment)
    - [acme Cross-Assembler](#acme-cross-assembler)
    - [PLASMA Source](#plasma-source)
        - [Portable VM](#portable-vm)
        - [Target VM](#target-vm)
- [Tutorial](#tutorial)
    - [PLASMA Compiler/Assembler](#plasma-compilerassembler)
    - [PLASMA Modules](#plasma-modules)
    - [Data Types](#data-types)
    - [Obligatory 'Hello World'](#obligatory-hello-world)
    - [Character Case](#character-case)
    - [Comments](#comments)
    - [Numbers](#numbers)
    - [Characters](#characters)
    - [Strings](#strings)
    - [Organization of a PLASMA Source File](#organization-of-a-plasma-source-file)
        - [Module Dependencies](#module-dependencies)
        - [File Inclusion](#file-inclusion)
        - [Predefined Functions](#predefined-functions)
        - [Constant Declarations](#constant-declarations)
        - [Structure Declarations](#structure-declarations)
        - [Global Data & Variables Declarations](#global-data-variables-declarations)
        - [Function Definitions](#function-definitions)
            - [Statements and Expressions](#statements-and-expressions)
        - [Exported Declarations](#exported-declarations)
        - [Module Main Initialization Function](#module-main-initialization-function)
        - [Module Done](#module-done)
    - [Runtime](#runtime)
- [Reference](#reference)
    - [Decimal and Hexadecimal Numbers](#decimal-and-hexadecimal-numbers)
    - [Character and String Literals](#character-and-string-literals)
        - [In-line String Literals](#in-line-string-literals)
    - [Words](#words)
    - [Bytes](#bytes)
    - [Addresses](#addresses)
        - [Arrays](#arrays)
            - [Type Overrides](#type-overrides)
            - [Multi-Dimensional Arrays](#multi-dimensional-arrays)
        - [Offsets (Structure Elements)](#offsets-structure-elements)
        - [Defining Structures](#defining-structures)
        - [Pointers](#pointers)
        - [Pointer Dereferencing](#pointer-dereferencing)
        - [Addresses of Data/Code](#addresses-of-datacode)
            - [Function Pointers](#function-pointers)
    - [Function Definitions](#function-definitions)
        - [Expressions and Statements](#expressions-and-statements)
            - [Address Operators](#address-operators)
            - [Arithmetic, Bitwise, and Logical Operators](#arithmetic-bitwise-and-logical-operators)
        - [Assignment](#assignment)
            - [Empty Assignments](#empty-assignments)
        - [Increment and Decrement](#increment-and-decrement)
        - [Lambda (Anonymous) Functions](#lambda-functions)
        - [Control Flow](#control-flow)
            - [CALL](#call)
            - [RETURN](#return)
            - [IF/[ELSIF]/[ELSE]/FIN](#ifelsifelsefin)
            - [WHEN/IS/[OTHERWISE]/WEND](#whenisotherwisewend)
            - [FOR \<TO,DOWNTO\> [STEP]/NEXT](#for-todownto-stepnext)
            - [WHILE/LOOP](#whileloop)
            - [REPEAT/UNTIL](#repeatuntil)
            - [CONTINUE](#continue)
            - [BREAK](#break)
- [Advanced Topics](#advanced-topics)
    - [Code Optimizations](#code-optimizations)
        - [Functions Without Parameters Or Local Variables](#functions-without-parameters-or-local-variables)
        - [Return Values](#return-values)
    - [Native Assembly Functions](#native-assembly-functions)
- [Libraries and Sample Code](https://github.com/dschmenk/PLASMA/wiki)
- [Implementation](#implementation)
    - [A New Approach](#a-new-approach)
    - [The Virtual Machine](#the-virtual-machine)
        - [The Stacks](#the-stacks)
            - [Evaluation Stack](#evaluation-stack)
            - [Call Stack](#call-stack)
            - [Local Frame Stack](#local-frame-stack)
            - [Local String Pool](#local-string-pool)
        - [The Bytecodes](https://github.com/dschmenk/PLASMA/wiki/PLASMA-Byte-Codes)
    - [Apple 1 PLASMA](#apple-1-plasma)
    - [Apple II PLASMA](#apple-ii-plasma)
    - [Apple /// PLASMA](#apple--plasma)
- [Links](#links)

<!-- /TOC -->

# Build Environment

## PLASMA Cross-Compiler

The first step in writing PLASMA code is to get a build environment working. If you have Unix-like environment, then this is a fairly easy exercise. Windows users may want to install the [Linux Subsystem for Windows](https://docs.microsoft.com/en-us/windows/wsl/install-win10) or the [Cygwin](https://www.cygwin.com/) environment to replicate a Unix-like environment under Windows. When installing Cygwin, make sure **gcc-core**, **make**, and **git** are installed under the **Devel** packages. Mac OS X users may have to install the **Xcode** from the App Store. Linux users should make sure the development packages are installed.

Launch the command-line/terminal application for your environment to download and build PLASMA. Create a source code directory and change the working directory to it, something like:

```
mkdir Src
cd Src
```

### acme Cross-Assembler

There are two source projects you need to download: the first is a nice cross-platform 6502 assembler called [acme](http://sourceforge.net/p/acme-crossass/code-0/6/tree/trunk/docs/QuickRef.txt). Download, build, and install the acme assembler by typing:

```
git clone https://github.com/meonwax/acme
cd acme/src
make
cp acme /usr/local/bin
cd ../..
```

Under Unix that `cp` command may have to be preceded by `sudo` to elevate the privileges to copy into `/usr/local/bin`.

### PLASMA Source

Now, to download PLASMA and build it, type:

```
git clone https://github.com/dschmenk/PLASMA
cd PLASMA/src
make
```

#### Portable VM

To see if everything built correctly, type:

```
make hello
```

and you should be rewarded with the classic `Hello, world.` being printed out to the terminal from the portable PLASMA VM, which is able to directly execute simple PLASMA modules. Now, enter:

```
make test
```

There should be a screenful of gibberish. This is a test suite to run through a large chunk of PLASMA's functionality for a quick sanity check, including dynamic module loading: `TESTLIB` in this case.

### Target VM

You will notice the name of the `HELLO` module shows up as `HELLO#FE1000` in the directory listing. This follows the naming scheme used by the [CiderPress](https://github.com/fadden/ciderpress) program used to transfer files into and out of Apple II disk images. The `#` character separates the base filename from the metadata used for the file type and auxiliary information. In order to run the HELLO module on a real or emulated Apple II requires copying the `PLASMA.SYSTEM#FF2000`, `CMD#FF2000`, `HELLO#FE1000`, `TEST#FE1000`, and `TESTLIB#FE1000` to a ProDOS disk image. You can find the ProDOS 1.9 system in the `PLASMA/sysfiles/PRODOS#FF0000` file; this is a convenience for building a bootable disk image from scratch. On the real or emulated Apple II, boot the ProDOS disk image. You will see a PLASMA introduction, then a command prompt. For this example, type:

```
+HELLO
```

to run the module. You will be rewarded with `Hello, world.` printed to the screen, or `HELLO, WORLD.` on an uppercase-only Apple ][. Finally, enter:

```
+TEST
```

and you should see the same screenful of gibberish you saw from the portable VM, but on the Apple II this time. Both VMs are running the exact same module binaries. To view the source of these modules refer to `PLASMA/src/samplesrc/hello.pla`, `PLASMA/src/samplesrc/test.pla`, and `PLASMA/src/samplesrc/testlib.pla`. To get even more insight into the compiled source, view the corresponding `.a` files.

## PLASMA Target Hosted Compiler

The PLASMA compiler is also self-hosted on the Apple II and III. The PLASMA system and development disks can be run on a real or emulated machine. It is recommended to copy the files to a hard disk, or similar mass storage device. Boot the PLASMA system and change the prefix to the development disk/directory. The 'HELLO.PLA' source file should be there. To compile the module, type:

```
+PLASM HELLO.PLA
```

After the compiler loads (which can take some time on an un-accelerated machine), you will see the compiler banner message. The complilation process prints out a `.` once in awhile. When compilation is complete, the module will be written to disk, and the prompt will return. To execute the module, type:

```
+HELLO
```

and just like with the cross-compiled module, you will get the `Hello, word.` message printed to the screen.

# Tutorial

I've created a YouTube playlist of PLASMA tutorial videos. Best viewed when you follow along on a live Apple II or emulator. They are brief segments to highlight one feature of PLASMA at a time: [Modern Retrogramming with PLASMA](https://www.youtube.com/playlist?list=PLlPKgUMQbJ79VJvZRfv1CJQf4SP2Gw3yU)

During KansasFest 2015, I gave a PLASMA introduction using the Apple II PLASMA sandbox IDE. You can play along using your favorite Apple II emulator, or one that runs directly in your browser: [Apple II Emulator in Javascript](https://www.scullinsteel.com/apple/e). Download [SANDBOX.PO](https://github.com/dschmenk/PLASMA/blob/master/SANDBOX.PO?raw=true) and load it into Drive 1 of the emulator. Start the [KansasFest PLASMA Code-along video](https://www.youtube.com/watch?v=RrR79WVHwJo?t=11m24s) and follow along.

To use this tutorial, make sure you have a working PLASMA installation as described in the previous section.

## PLASMA Compiler/Assembler

Although the low-level PLASMA VM operations could easily by coded by hand, they were chosen to be an easy target for a simple compiler. Think along the lines of an advanced assembler or stripped down C compiler ( C-- ).  Taking concepts from BASIC, Pascal, C and assembler, the PLASMA compiler is simple yet expressive. The syntax is line oriented; generally there is one statement per line. However, a semicolon, `;`, can separate multiple statements on a single line. This tutorial will focus on the cross-compiler running under an UNIX-like environment.

## PLASMA Modules

PLASMA programs are built up around modules: small, self contained, dynamically loaded and linked software components that provide a well defined interface to other modules. The module format extends the .REL file type originally defined by the EDASM assembler from the DOS/ProDOS Toolkit from Apple Computer, Inc. PLASMA extends the file format through a backwards compatible extension that the PLASMA loader recognizes to locate the PLASMA bytecode and provide for advanced dynamic loading of module dependencies. Modules are first-class citizens in PLASMA: an imported module is assigned to a variable which can be accessed like any other.

## Data Types

PLASMA only defines two data types: `char`(or `byte`) and `var`(or `word`). All operations take place on word-sized quantities, with the exception of loads and stores to byte sized addresses. The interpretation of a value can be an integer, an address, or anything that fits in 16 bits. There are a number of address operators to identify how an address value is to be interpreted.

## Obligatory 'Hello World'

To start things off, here is the standard introductory program:

```
include "inc/cmdsys.plh"

puts("Hello, world.\n")
done
```

Three tools are required to build and run this program: **acme**, **plasm**, and **plvm**. The PLASMA compiler, **plasm**, will convert the PLASMA source code (usually with an extension of .pla) into an assembly language source file. **acme** will convert the assembly source into a binary ready for loading. To execute the module, the PLASMA portable VM, **plvm**, can load and interpret the bytecode. The same binary can be loaded onto the target platform and run there with the appropriate VM. On Linux/Unix from PLASMA/src, the steps would be entered as:

```
./plasm -AM < hello.pla > hello.a
acme --setpc 4094 -o HELLO#FE1000 hello.a
./plvm HELLO
```

The computer will respond with:

```
Load module HELLO
Hello, world.
```

To build **acme** compatible module source, the `-AM` flags must be passed in. The **acme** assembler needs the `--setpc 4094` to assemble the module at the proper address `($1000 - 2)`, and the `-o` option sets the output file. The makefile in the PLASMA/src directory has automated this process. Enter:

```
make hello
```

for the **make** program to build all the dependencies and run the module.

## Character Case

All identifiers are case sensitive. Reserved words can be all upper or lower case. Imported and exported symbols are always promoted to upper case when resolved. Because some Apple IIs only work easily with uppercase, this eases the chance of mismatched symbol names.

## Comments

Comments are allowed throughout a PLASMA source file. The format follows that of C and C++: they begin with a `//` and comment out the rest of the line:

```
// This is a comment, the rest of this line is ignored
```

## Numbers

Decimal numbers (using digits 0 through 9) are written just as you would expect them. The number **42** would be written as `42`. PLASMA only knows about integer numbers; no decimal points. Negative numbers are preceded by a `-`. Hexadecimal constants are preceded with a `$` to identify them, such as `$C030`.

## Characters

Characters are byte values represented by a character surrounded by `'`(single quotation mark). The letter **A** would be encoded as `'A'`.

## Strings

Strings, sequences of characters, are represented by the list of characters surrounded by `"`(double quotation mark). The string **Hello** would be encoded as `"Hello"`.

## Organization of a PLASMA Source File

The source code of a PLASMA module first defines imports, constants, variables and data, then functions.  Constants must be initialized with a value. Variables can have sizes associated with them to declare storage space. Data can be declared with or without a label associated with it. Arrays, tables, strings and any predeclared data can be created and accessed in multiple ways. Arrays can be defined with a size to reserve a minimum storage amount, and the brackets can be after the type declaration or after the identifier.

### Module Dependencies

Module dependencies will direct the loader to make sure these modules are loaded first, thus resolving any outstanding references.  A module dependency is declared with the `import` statement block with predefined function and data definitions. The `import` block is completed with an `end`. An example:

```
import cmdsys
    const reshgr1 = $0004
    predef putc, puts, getc, gets, cls, gotoxy
end

import testlib
    predef puti
    char testdata, teststring
    var testarray
end
```

The `predef` pre-defines functions that can be called throughout the module.  The data declarations, `byte` and `word` will refer to data in those modules. `const` can appear in an `import` block, although not required. It does keep values associated with the imported module in a well-contained block for readability and useful with pre-processor file inclusion. Case is not significant for either the module name or the pre-defined function/data labels. They are all converted to uppercase with 16 characters significant when the loader resolves them.

### File Inclusion

Other files containing PLASMA code can be inserted directly into the code with the `include` statement. This statement is usually how external module information is referenced, without having to add it directly into the source code. The above code could be written as:

```
include "inc/cmdsys.plh"
include "inc/testlib.plh
```

### Predefined Functions

Sometimes a function needs to be referenced before it is defined. The `predef` declaration reserves the label for a function. The `import` declaration block also uses the `predef` declaration to reserve an external function. Outside of an `import` block, `predef` will only predefine a function that must be declared later in the source file, otherwise an error will occur.

```
predef exec_file(str), mydef(var)
```

### Constant Declarations

Constants help with the readability of source code where hard-coded numbers might not be very descriptive.

```
const MACHID   = $BF98
const speaker  = $C030
const bufflen  = 2048
const exec_cmd = 'X'
```

These constants can be used in expressions just like a variable name.

### Structure Declarations

There is a shortcut for defining constant offsets into structures:

```
struc t_entry
  var id
  char[32] name
  var next_entry
end
```

is equivalent to:

```
const t_entry    = 36 // size of the structure
const id         = 0  // offset to id element
const name       = 2  // offset to name element
const next_entry = 34 // offset to next_entry element
```

### Global Data & Variables Declarations

One of the most powerful features in PLASMA is the flexible data declaration. Data must be defined after all the `import` declarations and before any function definitions, `asm` or `def`. Global labels and data can be defined in multiple ways, and exported for inclusion in other modules. Data can be initialized with constant values, addresses, calculated values (must resolve to a constant), and addresses from imported modules.

```
//
// Import standard library functions.
//
include "inc/cmdsys.plh"
//
// Constants used for hardware and flags
//
const speaker     = $C030
const changed     = 1
const insmode     = 2
//
// Global variables
//
byte  flags       = 0
word  numlines    = 0
byte  cursx, cursy
word  cursrow, scrntop, cursptr
//
// Array declaration of screen row addresses. All variations are allowed.
//
word  txtscrn[]   = $0400,$0480,$0500,$0580,$0600,$0680,$0700,$0780
word[]            = $0428,$04A8,$0528,$05A8,$0628,$06A8,$0728,$07A8
word              = $0450,$04D0,$0550,$05D0,$0650,$06D0,$0750,$07D0
word txt2scrn[8]  = $0800,$0880,$0900,$0980,$0A00,$0A80,$0B00,$0B80
word[8] txt2scrna = $0828,$08A8,$0928,$09A8,$0A28,$0AA8,$0B28,$0BA8
word txt2scrnb    = $0850,$08D0,$0950,$09D0,$0A50,$0AD0,$0B50,$0BD0
```

Variables can have optional brackets; empty brackets don't reserve any space for the variable but are useful as a label for data that is defined following the variable.  Brackets with a constant inside define a minimum size reserved for the variable.  Any data following the variable will take at least the amount of reserved space, but potentially more.

Strings are defined like Pascal strings, a length byte followed by the string characters so they can be a maximum of 255 characters long.

```
//
// An initialized string of 64 characters
//
char[64] txtfile = "UNTITLED"
```

### Function Definitions

Functions are defined after all constants, variables and data. Function definitions can be `export`ed for inclusion in other modules and can be forward declared with a `predef` type in the constant and variable declarations. Functions can take parameters passed on the evaluation stack, then copied to the local frame for easy access. They can have their own variable declarations, however, unlike the global declarations, no data can be predeclared - only storage space.  A local frame is built for every function invocation and there is also a limit of 254 bytes of local storage.  Each parameter takes two bytes of local storage, plus two bytes for the previous frame pointer.  If a function has no parameters or local variables, no local frame will be created, improving performance.  Functions return a single value by default.
```
def myfunc(a, b) // Two parameters and defaults to one returned value
```
The number of values to return can be set by appending the number of values after the function definition with the '#' syntax, such as:

```
def myfuncA(a, b)#3 // Two parameters and three returned values
```
A definition with no parameters but with return values can be written as:
```
def myfuncB#2 //  No parameters and two returned values
```
A pre-defined definition should include the same number of parameters and return values as the definition:
```
predef myfuncA(a, b)#3
```
A value used as a function pointer doesn't have the parameter/return value count associated with it. It can be overridden in-line:
```
word funcptr = @myfuncA
funcptr(2, 4)#3
```
If fewer values are returned, the remaining values will be padded with zero. It is an error to return more values than specified. Definitions returning zero values are ok and can save some stack clean-up if the definitions are called stand-alone (i.e. as a procedure).

After functions are defined, the main code for the module follows. The main code will be executed as soon as the module is loaded.  For library modules, this is a good place to do any runtime initialization, before any of the exported functions are called. The last statement in the module must be done, or else a compile error is issued.

#### Statements and Expressions

Expressions are algebraic.  Data is free form, but all operations on the evaluation stack use 16 bits of precision with the exception of byte load and stores.  A stand-alone expression will be evaluated and read from or called.  This allows for easy access to the Apple’s soft switches and other memory mapped hardware. The value of the expression is dropped.

```
const speaker=$C030

^speaker // click speaker
```

Calling a function without looking at the result silently drops it:

```
close(refnum) // return value ignored
```

Statements are built up from expressions and control flow keywords.  Simplicity of syntax took precedence over flexibility and complexity.  The simplest statement is the basic assignment using `=`.

```
byte numchars
numchars = 0
```

Multi-value assignments are written with lvalues separated by commas, and the same number of rvalues separated by commas:
```
a, b, c = 2, 4, 6
```
Definitions can return values that contribute to the rvalue count:
```
def myfuncC(p1, p2)#2
    return p1+p2, p1-p2
end

a, b, c = 2, myfuncC(6, 7)  // Note: myfuncC returns 2 values
```
A quick way to swap variables could be written:
```
a, b = b, a
```

Expressions can be built up with constants, variables, function calls, addresses, and pointers/arrays.  Comparison operators evaluate to 0 or -1 instead of the more traditional 0 or 1.  The use of -1 allows binary operations to be applied to other non-zero values and still retain a non-zero result.  Any conditional tests check only for zero and non-zero values.

Operators on values can be unary: `-myvar`, binary: `var1 * var2`, and ternary: `testvar > 0 ?? trueValue :: falseValue`. The unary and binary operators are algebraic and referencing in nature. The ternary operator is logical, like an inline `if-then-else` clause (it works just like the ternary `? :` operator in C like languages).

There are four basic types of data that can be manipulated: constants, variables, addresses, and functions.  Memory can only be read or written as either a byte or a word.  Bytes are unsigned 8-bit quantities, words are signed 16-bit quantities.  Everything on the evaluation stack is treated as a word.  Other than that, any value can be treated as a pointer, address, function, character, integer, etc.  There are convenience operations in PLASMA to easily manipulate addresses and expressions as pointers, arrays, structures, functions, or combinations thereof.  If a variable is declared as a byte, it can be accessed as a simple, single dimension byte array by using brackets to indicate the offset.  Any expression can calculate the indexed offset.  A word variable can be accessed as a word array in the same fashion.  In order to access expressions or constants as arrays, a type identifier has to be inserted before the brackets.  a `.` character denotes a byte type, a `:` character denotes a word type.  Along with brackets to calculate an indexed offset, a constant can be used after the `.` or `:` and will be added to the base address.  The constant can be a defined const to allow for structure style syntax.  If the offset is a known constant, using the constant offset is a much more efficient way to address the elements over an array index.  Multidimensional arrays are treated as arrays of array pointers.

```
word hgrscan[] = $2000,$2400,$2800,$2C00,$3000,$3400,$3800,$3C00
word           = $2080,$2480,$2880,$2C80,$3080,$3480,$3880,$3C80

hgrscan.[ypos, xpos] = fillval
```

Values can be treated as pointers by preceding them with a `^` for byte pointers, and `*` for word pointers. Addresses of variables and functions can be taken with a proceeding `@`, the address-of operator. Parenthesis can surround an expression to be used as a pointer, but not address-of.


```
char[] hellostr = "Hello"
var srcstr, strlen

srcstr = @hellostr // srcstr points to address of hellostr
strlen = ^srcstr // the first byte srcstr points to is the string length
```

Functions with parameters or expressions to be used as a function address to call must use parenthesis, even if empty.

```
predef keyin2plus
var keyin
char key

keyin = @keyin2plus // address-of keyin2plus function
key   = keyin()
```

Lambda functions are anonymous functions that can be used to return a value (or multiple values). They can be used as function pointers to routines that need a quick and dirty expression. They are written as '&' (a poor man's lambda symbol) followed by parameters in parentheses, and the resultant expression. There are no local variables or statements allowed, only expressions. The ternary operator can be used to provide a bit of `if-then-else` control inside the lambda function.

```
word result

def eval_op(x, y, op)
    result = result + op(x, y)
    return result
end

def keyin(x, y, key)
    if key == '+'
        eval_op(x, y, &(x, y) x + y)
    fin
end
````
Control statements affect the flow of control through the program.  There are conditional and looping constructs.  The most widely used conditional is probably the `if`/`elsif`/`else`/`fin` construct.

```
if ^pushbttn3 < 128
    if key == $C0
        key = $D0 // P
    elsif key == $DD
        key = $CD // M
    elsif key == $DE
        key = $CE // N
    fin
else
    key = key | $E0
fin
```

The `when`/`is`/`otherwise`/`wend` statement is similar to the `if`/`elsif`/`else`/`fin` construct except that it is more efficient.  It selects one path based on the evaluated expressions, then merges the code path back together at the end.  Only the `when` value is compared against a list of constant.  Just as in C programs, a `break` statement is required to keep one clause from falling through to the next. Falling through from one clause to the next can have its uses, so this behavior has been added to PLASMA.

```
when keypressed
    is keyarrowup
        cursup
        break
    is keyarrowdown
        cursdown
        break
    is keyarrowleft
        cursleft
        break
    is keyarrowright
        cursright
        break
    is keyctrlx
        cutline
        break
    is keyctrlv
        pasteline
        break
    is keyescape
        cursoff
        cmdmode
        redraw
        break
    otherwise
        bell
wend
```

The most commonly used looping statement is the `for`/`next` construct.

```
for xscan = 0 to 19
    (scanptr):[xscan] = val
next
```

The `for`/`next` statement will efficiently increment or decrement a variable form the starting value to the ending value.  The increment/decrement amount can be set with the step option after the ending value; the default is one.  If the ending value is less than the starting value, use downto instead of `to` to progress in the negative direction.  Only use positive step values.  The `to` or `downto` will add or subtract the step value appropriately.

```
for i = heapmapsz - 1 downto 0
    if sheapmap.[i] <> $FF
        mapmask = szmask
    fin
next
```

`while`/`loop` statements will continue looping as long as the `while` expression is non-zero.

```
while !(mask & 1)
    addr = addr + 16
    mask = mask >> 1
loop
```

Lastly, the `repeat`/`until` statement will continue looping as long as the `until` expression is zero.

```
repeat
    txtbuf   = read(refnum, @txtbuf + 1, maxlnlen)
    numlines = numlines + 1
until txtbuf == 0 or numlines == maxlines
```
Function definitions are completed with the `end` statement. All definitions return a value, even if not specified in the source. A return value of zero will be inserted by the compiler at the `end` of a definition (or a `return` statement without a value). Defined functions without parameters can be called simply, without any parenthesis.

```
def drawscrn(topline, leftpos)#0
    byte i
    for i = 0 to 23
        drawline(textbuff[i + topline], leftpos)
    next
end
def redraw#0
    cursoff
    drawscrn(scrntop, scrnleft)
    curson
end

redraw
```

### Exported Declarations

Data and function labels can be exported so other modules may access this modules' data and code. By prepending `export` to the data or functions declaration, the label will become available to the loader for inter-module resolution. Exported labels are converted to uppercase with 16 significant characters.  Although the label will have to match the local version, external modules will match the case-insignificant, short version. Thus, "ThisIsAVeryLongLabelName" would be exported as: "THISISAVERYLONGL".

Here is an example using the `import`s from the previous examples to export an initialized array of 10 elements (2 defined + null delimiter):

```
predef mydef(var)

export var[10] myfuncs = @putc, @mydef, $0000
```

Exporting functions is simple:

```
export def plot(x, y)
    call($F800, y, 0, x, 0)
end
```

### Module Main Initialization Function

After all the function definitions are complete, an optional module initialization routine follows. This is an un-named definition and is written without a definition declaration. As such, it doesn't have parameters or local variables. Function definitions can be called from within the initialization code.

For libraries or class modules, the initialization routine can perform any up-front work needed before the module is called. For program modules, the initialization routine is the "main" routine, called after all the other module dependencies are loaded and initialized.

A return value is system specific. The default of zero should mean "no error". Negative values should mean "error", and positive values can instruct the system to do extra work, perhaps leaving the module in memory (terminate and stay resident).

### Module Done

The final declaration of a module source file is the `done` statement. This declares the end of the source file. Anything following this statement is ignored.  It might be useful to add documentation or other incidental information after this statement.

## Runtime

PLASMA includes a very minimal runtime that nevertheless provides a great deal of functionality to the system.  Two system calls are provided to access native 6502 routines (usually in ROM) and ProDOS/SOS.

call(addr, aReg, xReg, yReg, statusReg) returns a pointer to a four-byte structure containing the A,X,Y and STATUS register results.

```
const xreg = 1
const getlin = $FD6A

numchars = call(getlin, 0, 0, 0, 0)->xreg // return char count in X reg
```

syscall(cmd, params) calls ProDOS, returning the status value.

```
def read(refnum, buff, len)
    byte params[8]

    params.0 = 4
    params.1 = refnum
    params:2 = buff
    params:4 = len
    perr     = syscall($CA, @params)
    return params:6
end
```

putc(char), puts(string), home, gotoxy(x,y), getc() and gets() are other handy utility routines for interacting with the console.

```
putc('.')
char okstr[] = "OK"
puts(@okstr)
```

memset(addr, val, len) will fill memory with a 16-bit value.  memcpy(dstaddr, srcaddr, len) will copy memory from one address to another, taking care to copy in the proper direction.

```
char nullstr[] = ""
memset(strlinbuf, @nullstr, maxfill * 2) // fill line buff with pointer to null string
memcpy(scrnptr, strptr + ofst + 1, numchars)
```
# Reference

## Decimal and Hexadecimal Numbers

Numbers can be represented in either decimal (base 10), or hexadecimal (base 16). Values beginning with a `$` will be parsed as hexadecimal, in keeping with 6502 assembler syntax.

## Character and String Literals

A character literal, represented by a single character or an escaped character enclosed in single quotes `'`, can be used wherever a number is used. A length byte will be calculated and prepended to the character data. This is the Pascal style of string definition used throughout PLASMA and ProDOS. When referencing the string, its address is used:

```
char mystring[] = "This is my string; I am very proud of it."

putc('[') // enclose string in square brackets
puts(@mystring)
putc(']')
putc('\n')
```

Escaped characters, like the `\n` above are replaced with the Carriage Return character.  The list of escaped characters is:

| Escaped Char | ASCII Value
|:------------:|------------
|   \n         |    LF
|   \t         |    TAB
|   \r         |    CR
|   \\\\       |    \
|   \\0        |    NUL

### In-line String Literals

Strings can be used as literals inside expression or as parameters. The above puts() call can be written as:

```
puts("This is my string; I am very proud of it.")
```

just like any proper language. This makes coding a much simpler task when it comes to spitting out strings to the screen.

## Words

Words, 16-bit signed values, are the native sized quanta of PLASMA. All calculations, parameters, and return values are words.

## Bytes

Bytes are unsigned, 8-bit values, stored at an address.  Bytes cannot be manipulated as bytes, but are promoted to words as soon as they are read onto the evaluation stack. When written to a byte address, the low order byte of a word is used.

## Addresses

Words can represent many things in PLASMA, including addresses. PLASMA uses a 16-bit address space for data and function entry points. There are many operators in PLASMA to help with address calculation and access. Due to the signed implementation of word in PLASMA, the Standard Library has some unsigned comparison functions to help with address comparisons.

### Arrays

Arrays are the most useful data structure in PLASMA. Using an index into a list of values is indispensible. PLASMA has a flexible array operator. Arrays can be defined in many ways, usually as:

[`export`] <`byte`, `word`> [label] [= < number, character, string, address, ... >]

For example:

```
predef myfunc

byte smallarray[4]
byte initbarray[] = 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
char string[64] = "Initialized string"
var wlabel[]
word = 1000, 2000, 3000, 4000 // Anonymous array
word funclist = @myfunc, $0000
```

Equivalently written as:

```
predef myfunc(var)#0

byte[4] smallarray
byte[] initbarray = 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
char[64] string = "Initialized string"
var[] wlabel
word = 1000, 2000, 3000, 4000 // Anonymous array
word funclist = @myfunc, $0000
```

Arrays can be uninitialized and reserve a size, as in `smallarray` above.  Initialized arrays without a size specified in the definition will take up as much data as is present, as in `initbarray` above. Strings are special arrays that include a hidden length byte in the beginning (Pascal strings). When specified with a size, a minimum size is reserved for the string value. Labels can be defined as arrays without size or initializers; this can be useful when overlapping labels with other arrays or defining the actual array data as anonymous arrays in following lines as in `wlabel` and following lines. Addresses of other data (must be defined previously) or function definitions (pre-defined with predef), including imported references, can be initializers.

The base array size can be used to initialize multiple variable of arbitrary size. Three, four byte values can be defined as such:

```
byte[4] a, b, c
```

All three variables will have 4 bytes reserved for them. If you combine a base size with an array size, you can define multiple large values. For instance,

```
byte[4] a[5]
```

will assign an array of five, four byte elements, for a total of 20 bytes. This may make more sense when we combine the alias for `byte`, `res` with structure definitions. An array of five structures would look like:

```
res[t_record] patient[20]
```
The result would be to reserve 20 patient records.

#### Type Overrides

Arrays are usually identified by the data type specifier, `byte` or `word` when the array is defined. However, this can be overridden with the type override specifiers: `:` and `.`. `:` overrides the type to be `word`, `.` overrides the type to be `byte`. An example of accessing a `word` array as `bytes`:

```
word myarray = $AABB, $CCDD, $EEFF

def prarray
    byte i
    for i = 0 to 5
        puti(myarray.[i])
    next
end
```

The override operator becomes more useful when multi-dimensional arrays are used.

#### Multi-Dimensional Arrays

Multi-dimensional arrays are implemented as arrays of arrays, not as a single block of memory. This allows constructs such as:

```
//
// Hi-Res scanline addresses
//
word hgrscan = $2000,$2400,$2800,$2C00,$3000,$3400,$3800,$3C00
word         = $2080,$2480,$2880,$2C80,$3080,$3480,$3880,$3C80
```

...

```
def hgrfill(val)
    byte yscan, xscan

    for yscan = 0 to 191
        for xscan = 0 to 19
            hgrscan:[yscan, xscan] = val
        next
    next
end
```

Every array dimension except the last is a pointer to another array of pointers, thus the type is word. The last dimension is either `word` or `byte`, but cannot be specified with an array declaration, so the type override is used to identify the type of the final element. In the above example, the memory would be accessed as bytes with the following:

```
def hgrfill(val)
    byte yscan, xscan

    for yscan = 0 to 191
        for xscan = 0 to 39
            hgrscan.[yscan, xscan] = val
        next
    next
end
```

Notice how xscan goes to 39 instead of 19 in the byte accessed version.

### Offsets (Structure Elements)

Structures are another fundamental construct when accessing in-common data. Using fixed element offsets from a given address means you only have to pass one address around to access the entire record. Offsets are specified with a constant expression following the type override specifier.

```
predef puti // print an integer
byte myrec[]
word = 2
byte = "PLASMA"

puti(myrec:0) // ID = 2
putc($8D) // Carriage return
puti(myrec.2) // Name length = 6 (Pascal string puts length byte first)
```

This contrived example shows how one can access offsets from a variable as either `byte`s or `word`s regardless of how they were defined. This operator becomes more powerful when combined with pointers, defined below.

### Defining Structures

Structures can be defined so that the offsets are calculated for you. The previous example can be written as:

```
predef puti // print an integer
struc mystruc // mystruc will be defined as the size of the structure itself
  word id
  byte name // one byte for length, the number of characters are variable
end

byte myrec[]
word = 2
byte = "PLASMA"

puti(mystruc) // This will print '3', the size of the structure as defined
putc($8D) // Carriage return
puti(myrec:id) // ID = 2
putc($8D) // Carriage return
puti(myrec.name) // Name length = 6 (Pascal string puts length byte first)
```

### Pointers

Pointers are values that represent addresses. In order to get the value pointed to by the address, one must 'dereference' the pointer. All data and code memory has a unique address, all 65536 of them (16 bits). In the Apple architecture, many addresses are actually connected to hardware instead of memory. Accessing these addresses can make thing happen in Apples, or read external inputs like the keyboard and joystick.

### Pointer Dereferencing

Just as there are type override for arrays and offsets, there is a `byte` and `word` type override for pointers. Prepending a value with `^` dereferences a `byte`. Prepending a value with `*` dereferences a `word`. These are unary operators, so they won't be confused with the binary operators using the same symbol. An example getting the length of a Pascal string (length byte at the beginning of character array):

```
byte mystring = "This is my string"

def strlen(strptr)
    return ^strptr
end

puti(strlen(@mystring)) // print 17 in this case
```

Pointers to structures or arrays can be referenced with the `->` and `=>` operators, pointing to `byte` or `word` sized elements.

```
struc record
  byte id
  word addr
end

def addentry(entry, new_id, new_addr)
    entry->id   = new_id   // set ID      (byte)
    entry=>addr = new_addr // set address (word)
    return entry + record  // return next entry address
end
```

The above is equivalent to:

```
const elem_id = 0
const elem_addr  = 1
const record_size = 3

def addentry(entry, new_id, new_addr)
    (entry).elem_id   = new_id   // set ID byte
    (entry):elem_addr = new_addr // set address
    return entry + record_size   // return next entry address
end
```

### Addresses of Data/Code

Along with dereferencing a pointer, there is the question of getting the address of a variable. The `@` operator prepended to a variable name or a function definition name, will return the address of the variable/definition. From the previous example, the call to `strlen` would look like:

```
puti(strlen(@mystring)) // would print 17 in this example
```

#### Function Pointers

One very powerful combination of operations is the function pointer. This involves getting the address of a function and saving it in a `word` variable. Then, the function can be called be dereferencing the variable as a function call invocation. PLASMA is smart enough to know what you mean when your code looks like this:

```
word funcptr

def addvals(a, b)
    return a + b
end
def subvals(a, b)
    return a - b
end

funcptr = @addvals
puti(funcptr(5, 2)) // Outputs 7
funcptr = @subvals
puti(funcptr(5, 2)) // Outputs 3
```

These concepts can be combined with the structure offsets to create a function table that can be easily changed on the fly. Virtual functions in object-oriented languages are implemented this way.

```
predef myinit(), mynew(size), mydelete(obj)

export word myobject_class = @myinit, @mynew, @mydelete
// Rest of class data/code follows...
```

And an external module can call into this library (class) like:

```
import myclass
    const init   = 0
    const new    = 2
    const delete = 4
    word myobject_class
end

word an_obj // an object pointer

myobject_class:init()
an_obj = myobject_class:new()
myobject_class:delete(an_obj)
```

## Function Definitions

Function definitions in PLASMA are what really separate PLASMA from a low level language like assembly, or even a language like FORTH. The ability to pass in arguments and declare local variables provides PLASMA with a higher language feel and the ability to easily implement recursive functions.

### Expressions and Statements

PLASMA programs are a list of statements the carry out the algorithm. Statements are generally assignment or control flow in nature. Generally there is one statement per line. The ';' symbol separates multiple statements on a single line. It is considered bad form to have multiple statements per line unless they are very short. Expressions are comprised of operators and operations. Operator precedence follows address, arithmetic, binary, and logical from highest to lowest. Parentheses can be used to force operations to happen in a specific order.

#### Address Operators

Address operators can work on any value, i.e. anything can be an address. Parentheses can be used to get the value from a variable, then use that as an address to dereference for any of the post-operations.

| OP   |  Pre-Operation      |
|:----:|---------------------|
| ^    | byte pointer
| *    | word pointer
| @    | address of

| OP   |  Post-Operation     |
|:----:|---------------------|
| .    | byte type override
| :    | word type override
| ->   | pointer to byte type
| =>   | pointer to word type
| []   | array index
| ()   | functional call

#### Arithmetic, Bitwise, and Logical Operators

|  OP   |     Unary Operation |
|:-----:|---------------------|
|  -    | negate
|  ~    | bitwise compliment
|  NOT  | logical NOT
|  !    | logical NOT (alternate)

|  OP   |     Binary Operation |
|:-----:|----------------------|
|  *    | multiply
|  /    | divide
|  %    | modulo
|  +    | add
|  -    | subtract
|  <<   | shift left
|  >>   | shift right
|  &    | bitwise AND
|  ^    | bitwise XOR
|  &#124; | bitwise OR
|  ==   | equals
|  <>   | not equal
|  !=   | not equal (alt)
|  >=   | greater than or equal
|  >    | greater than
|  <=   | less than or equal
|  <    | less than
|  OR   |  logical OR
|  AND  |  logical AND
|  &#124;&#124;    |  logical OR (alt)
|  &&   |  logical AND (alt)

|  OP   |    Ternary Operation |
|:-----:|----------------------|
| ?? :: | if/then/else

### Assignment

Assignments evaluate an expression and save the result into memory. They can be very simple or quite complex. A simple example:

```
byte a
a = 0
```
Multiple assignement are supported. They can be very useful for returning more than one value from a function or efficiently swapping values around.
```
predef bivalfunc#2

a, b = bivalfunc() // Two values returned from function
stack[0], stack[1], stack[3] = 0, stack[0], stack[1] // Push 0 to bottom of three element stack
```
Should multiple values be returned, but only a subset is interesting, the special value `drop` can be used to ignore values.
```
predef trivalfunc#3

drop, drop, c = trivalfunc() // Three values returned from function, but we're only interested in the last one
```
#### Empty Assignments

An assignment doesn't even need to save the expression into memory, although the expression will be evaluated. This can be useful when referencing hardware that responds just to being accessed. On the Apple II, the keyboard is read from location $C000, then the strobe, telling the hardware to prepare for another key press is cleared by just reading the address $C010. In PLASMA, this looks like:

```
byte keypress

keypress = ^$C000 // read keyboard
^$C010 // read keyboard strobe, throw away value
```

### Increment and Decrement

PLASMA has an increment and decrement statement. This is different than the increment and decrement operations in languages like C and Java. They cannot be part of an expression, and only exist as a statement in postfix:

```
byte i

i = 4
i++ // increment i by 1
puti(i) // print 5
i-- // decrement i by 1
puti(i) // print 4
```

### Lambda Functions

A Lambda function is a simple, anonymous function that can be passed to a function or assigned to a variable. It is called as a function pointer. The function can take a number of parameters and return a value based on the parameters and/or global values. By enclosing the expression of the lambda function in parenthesis, multiple values can be returned.

```

def what_op(a, b, op)
    puti(a)
    puts(" ? ")
    puti(b)
    puts(" = ")
    puti(op(a, b))
    putln
end

def lambdas
    word lambda1
    word lambda2
    word x, y

    lambda1 = &(a, b) a + b
    lambda2 = &(a, b) (a + b, a - b)
    x    = lambda1(1, 2)           // This will return 3
    x, y = lambda2(3, 4)#2         // This will return 7, -1 (the #2 denotes two returned values instead of the default of one)
    what_op(10, 20, &(a, b) a * b) // This will print 10 ? 20 = 200
    return &(x, y, z) x * z / y    // You can even return lambdas from definitions (these x, y are different from the locally defined x, y)
end
````
There are some limitations to lambda functions. They don't have any local variables except for the passed parameters. They can only return an expression; there are no control flow statements allowed, but the ternary operator can be used for simple `if-then-else` constructs. Lastly, they can only be defined inside another definition. They cannot be defined in global data space or in the module main function (it gets deallocated after initialization).

### Control Flow

PLASMA implements most of the control flow that most high-level languages provide. It may do it in a slightly different way, though. One thing you won't find in PLASMA is GOTO - there are always other ways to achieve the same end.

#### CALL

Function calls are the easiest ways to pass control to another function. Function calls can be part of an expression, or can be all by themselves - the same as an empty assignment statement.

#### RETURN

`return` will exit the current definition. An optional value can be returned. However, if a value isn't specified, a default of zero will be returned. All definitions return a value, regardless of whether it used or not.

#### IF/[ELSIF]/[ELSE]/FIN

The common `if` test can have optional `elsif` and/or `else` clauses. Any expression that is evaluated to non-zero is treated as TRUE, zero is treated as FALSE.

#### WHEN/IS/[OTHERWISE]/WEND

The complex test case is handled with `when`. Basically an `if`, `elsif`, `else` list of comparisons, it is generally more efficient. The `is` value must be a constant.

```
when key
    is 'A'
        // handle A character
        break
    is 'B'
        // handle B character
        break
```

...

```
    is 'Z'
        // handle Z character
        break
    otherwise
        // Not a known key
wend
```

A `when` clause can fall-through to the following clause, just like C `switch` statements by leaving out the `break`.

#### FOR \<TO,DOWNTO\> [STEP]/NEXT

Iteration over a range is handled with the `for`/`next` loop. When iterating from a smaller to larger value, the `to` construct is used; when iterating from larger to smaller, the `downto` construct is used.

```
for a = 1 to 10
    // do something with a
next

for a = 10 downto 1
    // do something else with a
next
```

An optional stepping value can be used to change the default iteration step from 1 to something else. Always use a positive integral value; when iterating using `downto`, the step value will be subtracted from the current value.

#### WHILE/LOOP

For loops that test at the top of the loop, use `while`. The loop will run zero or more times.

```
a = c // who knows what c could be
while a < 10
    // do something
    a = b * 2 // b is something special, I'm sure
loop
```

#### REPEAT/UNTIL

For loops that always run at least once, use the `repeat` loop.

```
repeat
    update_cursor
until keypressed
```

#### CONTINUE

To continue to the next iteration of a looping structure, the `continue` statement will immediately skip to the next iteration of the innermost looping construct.

#### BREAK

To exit early from one of the looping constructs or `when`, the `break` statement will break out of it immediately and resume control immediately following the bottom of the loop/`when`.

# Advanced Topics

There are some things about PLASMA that aren't necessary to know, but can add to its effectiveness in a tight situation. Usually you can just code along, and the system will do a pretty reasonable job of carrying out your task. However, a little knowledge in the way to implement small assembly language routines or some coding practices just might be the ticket.

## Code Optimizations

### Functions Without Parameters Or Local Variables

Certain simple functions that don't take parameters or use local variables will skip the Frame Stack Entry/Leave setup. That can speed up the function significantly. The following could be a very useful function:

```
def keypress
    while ^$C000 < 128
    loop
    ^$C010
    return ^$C000
end
```

### Return Values

PLASMA always returns values from a function as it is defined, even if you don't explicitly supply one. Unless you define no return values with ```#0``` in the function definition.  For example:
```
def mydef()#2
    return 100
end
```
would silently return 100,0 because of the return value count in the definition.

## Native Assembly Functions

Native assembly functions are only available on the cross-compiler. Assembly code in PLASMA is implemented strictly as a pass-through to the assembler. No syntax checking, or checking at all, is made. All assembly routines *must* come after all data has been declared, and before any PLASMA function definitions. Native assembly functions can't see PLASMA labels and definitions, so they are pretty much relegated to leaf functions. Lastly, PLASMA modules are re-locatable, but labels inside assembly functions don't get flagged for fix-ups. The assembly code must use all relative branches and only access data/code at a fixed address. Data passed in on the PLASMA evaluation stack is readily accessed with the X register and the zero page address of the ESTK. The X register must be properly saved, incremented, and/or decremented to remain consistent with the rest of PLASMA. Parameters are **popped** off the evaluation stack with `INX`, and the return value is **pushed** with `DEX`. It is possible to relocate absolute addresses with a little trickery. Look to some of the library modules where native code is fixed up in the initialization block.

# Implementation

Both the Pascal and Java VMs used a bytecode to hide the underlying CPU architecture and offer platform agnostic application execution. The application and tool chains were easily moved from platform to platform by simply writing a bytecode interpreter and small runtime to translate the higher level constructs to the underlying hardware. The performance of the system was dependent on the actual hardware and efficiency of the interpreter. Just-in-time compilation wasn't really an option on small, 8-bit systems. FORTH, on the other hand, was usually implemented as a threaded interpreter. A threaded interpreter will use the address of functions to call as the code stream instead of a bytecode, eliminating one level of indirection with a slight increase in code size.  The threaded approach can be made faster at the expense of another slight increase in size by inserting an actual Jump SubRoutine opcode before each address, thus removing the interpreter's inner loop altogether.

All three of these systems were implemented using a stack architecture.  Pascal and Java were meant to be compiled high-level languages, using a stack machine as a simple compilation target.  FORTH was meant to be written directly as a stack-oriented language, similar to RPN on HP calculators. The 6502 is a challenging target due to its unusual architecture so writing a bytecode interpreter for Pascal and Java results in some inefficiencies and limitations.  FORTH's inner interpreter loop on the 6502 tends to be less efficient than most other CPUs.  Another difference is how each system creates and manipulates its stack. Pascal and Java use the 6502 hardware stack for all stack operations. Unfortunately the 6502 stack is hard-limited to 256 bytes. However, in normal usage this isn't too much of a problem as the compilers don't put undue pressure on the stack size by keeping most values in global or local variables.  FORTH creates a small stack using a portion of the 6502's zero page, a 256 byte area of low memory that can be accessed with only a byte address and indexed using either of the X or Y registers. With zero page, the X register can be used as an indexed, indirect address and the Y register can be used as an indirect, indexed address.

## A New Approach

PLASMA takes an approach that uses the best of all the above implementations to create a unique, powerful and efficient platform for developing new applications on the Apple 1, II, and ///. One goal was to create a very small VM runtime, bytecode interpreter, and module loader. The decision was made early on to implement a stack-based architecture duplicating the approach taken by FORTH. Space in the zero page would be assigned to a 16-bit, 16-element evaluation stack, indexed by the X register.

A simple compiler was written so that higher-level constructs could be used and global/local variables would hold values instead of using clever stack manipulation. Function/procedure frames would allow for local variables, but with a limitation - the frame could be no larger than 256 bytes. By enforcing this limitation, the function frame could easily be accessed through a frame pointer value in zero page, indexed by the Y register. The call stack uses the 6502's hardware stack resulting in the same 256-byte limitation imposed by the hardware. However, extending the call sequence to save and restore the return address in the function frame could lift this limitation. This was not done initially for performance reasons and simplicity of implementation. Even with these limitations, recursive functions can be effectively implemented.

One of the goals of PLASMA was to allow for intermixing of functions implemented as either bytecode or native code. Taking a page from the FORTH playbook, a function call is implemented as a native subroutine call to an address. If the function is in bytecode, the first thing it does is call back into the interpreter to execute the following bytecode (or a pointer to the bytecode). Function call parameters are pushed onto the evaluation stack in order they are written. The first operation inside of the function call is to pull the parameters off the evaluation stack and put them in local frame storage. Function callers and callees must agree on the number of parameters to avoid stack underflow/overflow. All functions return a value on the evaluation stack regardless of it being used or not.

The bytecode interpreter is capable of executing code in main memory or banked/extended memory, increasing the available code space and relieving pressure on the limited 64K of addressable data memory in the 6502 architecture. In the Apple IIe with 64K expansion card, the IIc, and the IIgs, there is auxiliary memory that swaps in and out for the main memory in banks.  The interpreter resides in the Language Card memory area that can easily swap in and out the $0200 to $BFFF memory bank.  The module loader will move the bytecode into this auxiliary memory and fix up the entry points to reflect the bytecode location. The Apple /// has a sophisticated extended addressing architecture where bytecode is located and interpreted.

Lastly, PLASMA is not a typed language. Just like assembly, any value can represent a character, integer, or address. It's the programmer's job to know the type. Only bytes and words are known to PLASMA. Bytes are unsigned 8-bit quantities, words are signed 16-bit quantities. All stack operations involve 16 bits of precision.

## The Virtual Machine

The 6502 processor is a challenging target for a compiler. Most high-level languages do have a compiler available targeting the 6502, but none are particularly efficient at code generation. Usually a series of calls into routines that do much of the work, not too dissimilar to a threaded interpreter. Generating inline 6502 leads quickly to code bloat and unwieldy binaries. The trick is to find a happy medium between efficient code execution and small code size. To this end, the PLASMA VM enforces some restrictions that are a result of the 6502 architecture, yet don't hamper the expressiveness of the PLASMA language.

### The Stacks

The basic architecture of PLASMA relies on different stack based FIFO data structures. The stacks aren't directly manipulated from PLASMA, but almost every PLASMA operation involves one or more of the stacks. A stack architecture is a very flexible and convenient way to manage an interpreted language, even if it isn't the highest performance.

The PLASMA VM is architected around three stacks: the evaluation stack, the call stack, and the local frame stack. These stacks provide the PLASMA VM with foundation for efficient operation and compact bytecode. The stack architecture also creates a simple target for the PLASMA compiler.

#### Evaluation Stack

All temporary values are loaded and manipulated on the PLASMA evaluation stack. This is a small (16 element) stack implemented in high performance memory/registers of the host CPU. Parameters to functions are passed on the evaluation stack, then moved to local variables for named reference inside the function.

All calculations, data moves, and parameter passing are done on the evaluation stack. This stack is located on the zero page of the 6502; an efficient section of memory that can be addressed with only an eight bit address. As a structure that is accessed more than any other on PLASMA, it makes sense to put it in fastest memory. The evaluation stack is a 16-entry stack that is split into low bytes and high bytes. The 6502's X register is used to index into the evaluation stack. It *always* points to the top of the evaluation stack, so care must be taken to save and restore its value when calling native 6502 code. Parameters and results are also passed on the evaluation stack. Caller and callee must agree on the number of parameters: PLASMA does no error checking. Native functions can pull values from the evaluation stack by using the zero page indexed addressing using the X register.

#### Call Stack

The call stack, where function return addresses are saved, is implemented using the hardware call stack of the CPU. This makes for a fast and efficient implementation of function call/return.

Function calls use the call stack to save the return address of the calling code. PLASMA uses the 6502 hardware stack for this purpose, as it is the 6502's JSR (Jump SubRoutine) instruction that PLASMA's call opcodes are implemented.

#### Local Frame Stack

Any function definition that involves parameters or local variables builds a local frame to contain the variables. Often called automatic variables, they only persist during the lifetime of the function. They are a very powerful tool when implementing recursive algorithms. PLASMA puts a limitation of 256 bytes for the size of the frame, due to the nature of the 6502 CPU (8-bit index register). With careful planning, this shouldn't be too constraining.

One of the biggest problems to overcome with the 6502 is its very small hardware stack. Algorithms that incorporate recursive procedure calls are very difficult or slow on the 6502. PLASMA takes the middle ground when implementing local frames; a frame pointer on the zero page is indirectly indexed by the Y register. Because the Y register is only eight bits, the local frame size is limited to 256 bytes. 256 bytes really are sufficient for all but the most complex of functions. With a little creative use of dynamic memory allocation, almost anything can be implemented without undue hassle. When a function with parameters is called, the first order of business is to allocate the frame, copy the parameters off the evaluation stack into local variables, and save a link to the previous frame. This is all done automatically with the `ENTER` opcode. The reverse takes place with the `LEAVE` opcode when the function exits. Functions that have neither parameters nor local variables forgo the frame build/destroy process.

#### Local String Pool

Any function that uses in-line strings may have those strings copied to the local string pool for usage. This allows string literals to exist in the same memory as the bytecode and only copied to main memory when used. The string pool is deallocated along with the local frame stack when the function exits.

## Apple 1 PLASMA

Obviously the Apple 1 is a little more constrained than most machines PLASMA is targeting. But, with the required addition of the CFFA1 (http://dreher.net/?s=projects/CFforApple1&c=projects/CFforApple1/main.php), the Apple 1 gets 32K of RAM and a mass storage device. Enough to run PLASMA and load/execute modules.

## Apple II PLASMA

The Apple ][ support covers the full range of the Apple II family. From the Rev 0 Apple II to the ROM3 Apple IIgs. The only requirement is 64K of RAM. If 128K is present, it will be automatically used to load and interpret bytecode, freeing up the main 40K for data and native 6502 code. The IIgs is currently operated in the compatibility 8-bit mode.

## Apple /// PLASMA

Probably the most exciting development is the support for the Apple ///. PLASMA on the Apple /// provides 32K for global data and 6502 code, and the rest of the memory for bytecode and extended data.


# Links

[PLASMA on Acorn Beeb](http://stardot.org.uk/forums/viewtopic.php?f=55&t=12306&p=163288#p163288)

[ACME 6502 assembler](https://sourceforge.net/projects/acme-crossass/)

[HiDef PLASMA KFEST 2015 video](https://archive.org/details/Kansasfest2015ThePlasmaLanguage)

[Mike Finger's PLASMA blog](http://retro2neo.org/2016/04/02/working-with-plasma)

[BCPL Programming Language](http://en.wikipedia.org/wiki/BCPL)

[B Programming Language User Manual](https://www.bell-labs.com/usr/dmr/www/bintro.html)

[FORTH Programming Language](http://en.wikipedia.org/wiki/Forth_(programming_language))

[UCSD Pascal](http://wiki.freepascal.org/UCSD_Pascal)

[Pascal Pseudo Machine (p-code)](https://www.princeton.edu/~achaney/tmve/wiki100k/docs/P-code_machine.html)

[VM02: Apple II Java VM](http://sourceforge.net/projects/vm02/)

[Threaded code](http://en.wikipedia.org/wiki/Threaded_code)

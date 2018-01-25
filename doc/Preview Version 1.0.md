# Developer Preview #3 Version 1.0

PLASMA is approaching a 1.0 release after _only_ 12 years. Hopefully it was worth the wait. To work out the remaining kinks, this Developer Preview will allow programmers to kick the tires, so to speak, to provide feedback on the system.

Download the three disk images:

[PLASMA Preview #3 1.0 System](https://github.com/dschmenk/PLASMA/blob/master/PLASMA-PRE3.PO?raw=true)

[PLASMA 1.0 Build System](https://github.com/dschmenk/PLASMA/blob/master/PLASMA-BLD1.PO?raw=true)

[PLASMA 1.0 Demos](https://github.com/dschmenk/PLASMA/blob/master/PLASMA-DEM1.PO?raw=true)

PLASMA can be run from floppies, System in drive 1, and Build in drive 2. All Apple II computers are supported, from the earliest Rev 0 to the last Apple IIGS. However, an accelerator and hard disk/CFFA are highly recommended. The recommended mass storage installation looks like:

System Files => /HARDISK/PLASMA.PRE3/

Build Files => /HARDISK/BLD/

Demo Files => /HARDISK/DEMOS/

Keeping the system files seperate from the build directory will make upgrading to the final 1.0 Release later a little easier. To boot directly into PLASMA, you will need to put the system files in the root prefix of the boot device and make sure PLASMA.SYSTEM is the first SYSTEM file in the directory. Otherwise, start PLASMA.SYSTEM from your program launcher of choice.

## 65802/65816 Support

PLASMA can utilize the 16 bit features of the 65802 and 65816 processors to improve performance of the PLASMA VM operation. This is transparent to the programmer/user and doesn't make any additional memory or capabilities available to PLASMA. Launch `PLASMA16.SYSTEM` to use the 16 bit PLASMA VM. If you don't have the right CPU, it will print a message and restart.

## PLASMA Command Line Shell

PLASMA incorporates a very basic command line shell to facilitate navigating the filesystem and executing both SYSTEM programs and PLASMA modules. It has a few built-in commands:

|       Command       |      Operation        |
|:----------------------------:|-------------------------|
| C [PREFIX]                   | Catalog prefix
| P \<PREFIX\>                 | change to Prefix
| /                            | change to parent prefix
| V                            | show online Volumes
| -\<SYSTEM PROGRAM\> [PARAMS] | launch SYSTEM program
| +\<PLASMA MODULE\> [PARAMS]  | exec PLASMA module
```
[Optional parameters]
<Required parameters>
```

The shell is very brief with error messages. It is meant solely as a way to run programs that accept command line parameters and take up as little memory as possible. It does, however, provide a rich runtime for PLASMA modules.

## Included Modules

The Developers Preview comes with a basic set of system modules. When PLASMA is launched, the SYS/ directory immediately below where PLASMA.SYSTEM was launched contains the modules. The system volume (where PLASMA was started) must remain in place for the duration of PLASMAs run. Otherwise it won't be able to find CMD or the system libraries. Probably the most useful included module is the editor. It is used for editing PLASMA source file, assembly source files, or any ProDOS text file. Execute it with:
```
+ED [TEXT FILE]
```

### Compiler Modules

The build disk includes sample source, include files for the system modules, and the PLASMA compiler+optimizer modules. The compiler is invoked with:
```
+PLASM [-[W][O[2]] <SOURCE FILE> [OUTPUT FILE]
```
Compiler warnings are enabled with `-W`. The optional optimizer is enabled with `-O` and extra optimizations are enabled with `-O2`. The source code for a few sample programs are included. The big one, `RPNCALC.PLA`, is the sample RPN calculator that uses many of PLASMA's advanced features. The self-hosted compiler is the same compiler as the cross-compiler, just transcribed from C to PLASMA (yes, the self-hosted PLASMA compiler is written in PLASMA). It requires patience when compiling: it is a fairly large and extensive program.

### Demos

There are some demo programs included for your perusal. Check out `ROGUE` for some diversion. You can find the documentation here: https://github.com/dschmenk/PLASMA/blob/master/doc/Rogue%20Instructions.md. A music sequencer to play through a MockingBoard if it is detected, or the built-in speaker if not. There may be problems if there is a CP/M card present when detecting the MockingBoard. A minimal Web server if you have an Uthernet2 card (required). Bug reports appreciated.

## Source Code

This is a Developers Preview, all sample source code is included from the project. It builds without alteration and should be a good starting point for further explorations. The header files for the included library modules are in the INC directory. Previously, examples from the sandbox were included but they have been removed to make room for all the project samples and include files.

## Issues

- All the modules and runtime are written mostly in PLASMA; the compiler and editor as well. This means that there may be some startup delay as the PLASMA module loader reads in the module dependencies and performs dynamic linking. But a 1 MHz, 8 bit CPU interpreting bytecodes is never going to match a modern computer. As noted earlier, an accelerator and mass storage are your (and PLASMA's) friend.

- All the project modules are included. They have been tested, with the exception of the Uthernet2 driver. I seem to have misplaced mine. If someone can try the Web Server demo in /PLASMA.DEMOS/NET and leave feedback would be very appreciated.

- The documentation is sparse and incomplete. Yep, could use your help...

# Changes in PLASMA for 1.0

If you have been programming in PLASMA before, the 1.0 version has some major and minor changes that you should be aware of:

1. Case is no longer significant. Imported symbols were always upper case. Now, all symbols are treated as if they were upper case. You may find that some symbols clash with previously defined symbols of different case. Hey, we didn't need lower case in 1977 and we don't need it now. You kids, get off my lawn!

2. Modules are now first class citizens. Translation: importing a module adds a symbol with the module name. You can simply refer to a module's address with it's name. This is how a module's API table is accessed (instead of adding a variable of the same name in the IMPORT section).

3. Bytecode changes means previously compiled modules will crash. Rebuild.

4. `BYTE` and `WORD` have aliases that may improve readability of the code. `CHAR` (character) and `RES` (reserve) are synonyms for `BYTE`. `VAR` (variable) is a synonym for `WORD`. These aliases add no functionality. They are simply syntactic sugar to add context to the source code, but may cause problems if you've previously used the same names for identifiers.

5. When declaring variables, a base size can come after the type, and an array size can folllow the identifier. For instance:
```
res[10] a, b, c
```
will reserve three variables of 10 bytes each. Additionally:
```
res[10] v[5], w[3]
```
will reserve a total of 80 bytes (10 * 5 + 10 * 3). This would be useful when combined with a structure definition. One could:
```
res[t_record] patients[20]
```
to reserve an array of 20 patient records.

6. Ternary operator. Just like C and descendants, `??` and `::` allow for an if-then-else inside an expression:
```
puts(truth == TRUE ?? "TRUE" :: "FALSE")
```

7. Multiple value assignements. Multiple values can be returned from functions and listed on variable assignments:
```
def func#3 // Return 3 values
  return 10, 20, 30
end

a, b, c = 1, 2, 3
c, d, f = func()
x, y = y, x // Swap x and y
```

8. `DROP` allows for explicit dropping of values. In the above `func()` example, if the middle value was the only one desired, the others can be ignored with:
```
drop, h, drop = func()
```

9. The compiler tracks parameter and return counts for functions. If the above `func()` were used without assigning all the return values, they would be dropped:
```
a = func() // Two values silently dropped
```
To generate compiler warning for this issue, and a few others, use the `-W` option when compiling.

10. Lambda (Anonymous) Functions. The ability to code a quick function in-line can be very powerful when used properly. Look here, https://en.wikipedia.org/wiki/Anonymous_function, for more information.

11. SANE (Standard Apple Numerics Environment) Floating Point Library. An extensive library (two, actually) of extended floating point (80 bit IEEE precision) functionality is suported. A wrapper library has been written to greatly simplify the interface to SANE. Look at the `RPNCALC.PLA` source code as an example.

12. Library Documentation. Preliminary documentation is available on the Wiki: https://github.com/dschmenk/PLASMA/wiki

13. Significant effort has gone into VM tuning and speeding up module loading/dynamic linking.

14. The VM zero page usage has changed. If you write assembly language routines, you will need to rebuild.

# Thanks

I wish to thank the people who have contributed the the PLASMA project. They have greatly improved the development of the language and documentation:

- Martin Haye: PLASMA programmer extraordinaire. Mr. Lawless Legends has requested many of the crucial features that set PLASMA apart.
- Steve F (ZornsLemma): Has taken the optimizer to new levels and his work on porting PLASMA to the Beeb are amazing: http://stardot.org.uk/forums/viewtopic.php?f=55&t=12306&sid=5a503c593f0698ebc31e590ac61b09fc
- Peter Ferrie: Assembly optimizer extraordinaire. He has made significant improvements into the code footprint in PLASMA so all the functionality can exist in just a few bytes.
- David Schmidt (DaveX): His help in documentation have made it much more accessible and professional. Of course any errors are all his. Just kidding, they're mine ;-)
- Andy Werner (6502.org): Catching the grammatical errors that I ain't no good at.
- John Brooks: Apple II Guru par excellence. His insights got 10% performance increase out of the VM.

Dave Schmenk
http://schmenk.is-a-geek.com

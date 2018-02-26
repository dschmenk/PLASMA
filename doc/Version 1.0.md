# PLASMA Version 1.0

Welcome to PLASMA: the Grand Unifying Platform for the Apple 1, ][, and ///.

Download the four disk images (three if you don't plan to boot an Apple ///):

[PLASMA 1.0 System and ProDOS Boot](https://github.com/dschmenk/PLASMA/blob/master/PLASMA-SYS1.PO?raw=true)

[PLASMA 1.0 Build Tools](https://github.com/dschmenk/PLASMA/blob/master/PLASMA-BLD1.PO?raw=true)

[PLASMA 1.0 Demos](https://github.com/dschmenk/PLASMA/blob/master/PLASMA-DEM1.PO?raw=true)

[PLASMA 1.0 Apple /// SOS Boot ](https://github.com/dschmenk/PLASMA/blob/master/PLASMA-SOS1.PO?raw=true)

PLASMA can be run from floppies, System in Drive 1, and Build or Demos in Drive 2. Mass storage is the recommended installation that looks like (replacing HARDISK with your volume name of choice):

System Files => /HARDISK/PLASMA/

Build Files => /HARDISK/PLASMA/BLD/

Demo Files => /HARDISK/PLASMA/DEMOS/

Use the System Utilities to copy the floppy images into the above mentioned directories.

## Apple 1

The Apple 1 is a very constrained system compared to the ][ and ///. It is required to have the CFFA1 disk adapter installed to provide file storage and a full 32K od RAM. To get the files onto the CF card required the use of [CiderPress](http://a2ciderpress.com) and they must be placed in one directory. Most PLASMA programs won't work on the Apple 1 due to limited filesystem support, video/graphics capabilities, and lack of audio output. It does, however, make a good place to start when porting PLASMA to a new platform.

## Apple ][

To boot directly into PLASMA, you will need to put the system files in the root prefix of the boot device and make sure PLASMA.SYSTEM is the first SYSTEM file in the directory. Otherwise, start PLASMA.SYSTEM from your program launcher of choice. All Apple ][ models with 64K and two floppy drives are supported up to a maxed out IIGS with accelerator and hard drive.

#### 65802/65816 Support

PLASMA can utilize the 16 bit features of the 65802 and 65816 processors to improve performance of the PLASMA VM operation. This is transparent to the programmer/user and doesn't make any additional memory or capabilities available to PLASMA. Launch `PLASMA16.SYSTEM` to use the 16 bit PLASMA VM. If you don't have the right CPU, it will print a message and restart.

## Apple ///

The Apple /// gets the environment it always wanted: The ability to navigate the filesystem with a command line interface. The Apple /// always boots from the floppy drive, even if a hard disk is installed. The PLASMA.SOS floppy should be updated with the SOS.DRIVER configured for your machine. Once booted, type `S /HARDISK/PLASMA` (or your install directory of choice) to change to, and set, the system directory. This can be automated by creating an `AUTORUN` file on the boot floppy with the above command in it.

## PLASMA Command Line Shell

PLASMA incorporates a very basic command line shell to facilitate navigating the filesystem and executing both SYSTEM/SOS programs and PLASMA modules. It has a few built-in commands:

|       Command       |      Operation        |
|:----------------------------:|-------------------------|
| C [PREFIX]                   | Catalog prefix
| P \<PREFIX\>                 | change to Prefix
| /                            | change to parent prefix
| V                            | show online Volumes
| S \<PREFIX\>                 | set System prefix*
| +SOS \<SOS.INTERP\> [PREFIX] | launch SOS interpreter*
| -\<SYSTEM PROGRAM\> [PARAMS] | launch SYSTEM program**
| +\<PLASMA MODULE\> [PARAMS]  | exec PLASMA module
```
[Optional parameters]
<Required parameters>
* Apple /// only
** Apple ][ only
```

The shell is very brief with error messages. It is meant solely as a way to run programs that accept command line parameters and take up as little memory as possible. It does, however, provide a rich runtime for PLASMA modules.

## Included Modules

PLASMA comes with many library modules used by the tools, demos and sample code. The PLASMA system volume must remain in place for the duration of PLASMAs run otherwise it won't be able to find CMD or the system libraries. Probably the most useful included module is the editor. It is used for editing PLASMA source file, assembly source files, or any text file. Execute it with:
```
+ED [TEXT FILE]
```

## Compiler Modules

The build disk includes sample source, include files for the system modules, and the PLASMA compiler+optimizer modules. The compiler is invoked with:
```
+PLASM [-[W][O[2]] <SOURCE FILE> [OUTPUT FILE]
```
Compiler warnings are enabled with `-W`. The optional optimizer is enabled with `-O` and extra optimizations are enabled with `-O2`. The source code for a few sample programs are included. The big one, `RPNCALC.PLA`, is the sample RPN calculator that uses many of PLASMA's advanced features. The self-hosted compiler is the same compiler as the cross-compiler, just transcribed from C to PLASMA (yes, the self-hosted PLASMA compiler is written in PLASMA). It requires patience when compiling: it is a fairly large and extensive program.

## Demos

There are some demo programs included for your perusal. Check out `ROGUE` for some diversion. You can find the documentation here: https://github.com/dschmenk/PLASMA/blob/master/doc/Rogue%20Instructions.md. A music sequencer to play through a MockingBoard if it is detected, or the built-in speaker if not. A minimal Web server if you have an Uthernet2 card (required). Bug reports appreciated.

## Source Code

Most sample source code is included from the project. They build without alteration and should be a good starting point for further explorations. The header files for the included library modules are in the INC directory.

## Video Playlist

There is a [YouTube playlist](https://www.youtube.com/playlist?list=PLlPKgUMQbJ79VJvZRfv1CJQf4SP2Gw3yU) created for learning PLASMA. It is a WIP, with updates every week or so

## Issues

- All the modules and runtime are written mostly in PLASMA; the compiler and editor as well. This means that there may be some startup delay as the PLASMA module loader reads in the module dependencies and performs dynamic linking. But a 1 MHz, 8 bit CPU interpreting bytecodes is never going to match a modern computer. As noted earlier, an accelerator and mass storage are your (and PLASMA's) friend.

- All the project modules are included. They have been tested, with the exception of the Uthernet2 driver. I seem to have misplaced mine. If someone can try the Web Server demo in /PLASMA.DEMOS/NET and leave feedback would be very appreciated.

- The Apple /// may not always report errors properly or at all.

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

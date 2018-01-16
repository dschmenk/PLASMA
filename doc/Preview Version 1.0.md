# Developer Preview Version 1.0

PLASMA is approaching a 1.0 release after _only_ 12 years. Hopefully iy was worth the wait. To work out the remaining kinks, this Developer Preview will allow programmers to kcick the tires, so to speak, to provide feedback on the system.

Download the two disk images:

(PLASMA Preview 1.0 System)[https://github.com/dschmenk/PLASMA/blob/master/PLASMA-PRE1.PO?raw=true]

(PLASMA 1.0 Build System)[https://github.com/dschmenk/PLASMA/blob/master/PLASMA-BLD1.PO?raw=true]

PLASMA can be run from floppies, System in drive 1, and Build in drive 2. All Apple II computers are supported, from the earliest Rev 0 to the last Apple IIGS. However, an accelerator and hard disk/CFFA are highly recommended. The recommended mass storage installation looks like:

System Files => /HARDISK/PLASMA.PRE1/

Build Files => /HARDISK/BLD/

Keeping the system files seperate from the build directory will make upgrading to the final 1.0 Release later a little easier. To boot directly into PLASMA, you will need to put the system files in the root prefix of the boot device and make sure PLASMA.SYSTEM is the first SYSTEM file in the directory.

## 65802/65816 Support

PLASMA can utilize the 16 bit features of the 65802 and 65816 processors to improve performance of the PLASMA VM operation. This is transparent to the programmer/user and doesn't make any additional memory or capabilities available to PLASMA. Launch `PLASMA16.SYSTEM` to use the 16 bit PLASMA VM. If you don't have the right CPU, it will print a message and restart.

# PLASMA Command Line Shell

PLASMA incorporates a very basic command line shell to facilitate navigating the filesystem and executing both SYSTEM programs and PLASMA modules. It has a few built-in commands:

|       Command       |      Operation        |
|:-------------------:|-----------------------|
| C [PREFIX]          | Catalog prefix
| P \<PREFIX\>        | change to Prefix
| /                   | change to parent prefix
| V                   | show online Volumes
| -\<SYSTEM PROGRAM\> | launch SYSTEM program
| +\<PLASMA MODULE\>  | exec PLASMA module


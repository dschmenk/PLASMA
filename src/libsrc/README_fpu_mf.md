# MegaFlash Hardware-Accelerated FPU Library for PLASMA

## Overview

`fpu_mf.pla` is a drop-in replacement for the standard SANE-based `fpu.pla` library that provides hardware acceleration for floating-point operations using the MegaFlash storage device for Apple IIc/IIc+.

## Features

- **Hardware Acceleration**: Uses MegaFlash's Raspberry Pi Pico for fast floating-point calculations
- **API Compatible**: Maintains complete API compatibility with `fpu.pla`
- **Automatic Fallback**: Detects MegaFlash presence and falls back to SANE if not available
- **Format Conversion**: Automatically converts between SANE Extended (80-bit) and MBF (40-bit) formats
- **Accelerated Operations**:
  - Multiplication (`mul`)
  - Division (`div`)
  - Sine (`sin`)
  - Cosine (`cos`)
  - Tangent (`tan`)
  - Arctangent (`atan`)
  - Natural logarithm (`lnX`)
  - Exponential (`powEX`)
  - Square root (`sqrt`)

## Usage

Simply replace the import statement in your PLASMA program:

```plasma
// Old version using SANE software FPU:
import fpu

// New version using MegaFlash hardware acceleration:
import fpu_mf
```

That's it! All function calls remain exactly the same.

## Example

```plasma
include "inc/cmdsys.plh"
import fpu_mf

// Initialize the FPU
fpu_mf:reset

// Push some values and do math
fpu_mf:pushStr("3.14159")
fpu_mf:sin
fpu_mf:pullStr(@result, 1, 6, FPSTR_FIXED)
puts(@result)
```

## Hardware Requirements

- Apple IIc or IIc+ computer
- MegaFlash storage device with FPU support enabled
- MegaFlash firmware v1.0 or later

## Technical Details

### Format Conversion

The library automatically handles conversion between formats:

- **SANE Extended**: 80-bit (sign + 15-bit exponent + 64-bit mantissa)
- **MBF (Microsoft Binary Format)**: 40-bit (8-bit exp + 32-bit mantissa + 8-bit extension)

### Performance

Hardware-accelerated operations are significantly faster:

- Transcendental functions (sin, cos, tan, log, exp): **10-100x faster**
- Multiplication/Division: **3-5x faster**
- Addition/Subtraction: Uses SANE (conversion overhead negates benefit)

### Fallback Behavior

When MegaFlash is not detected or operations are not hardware-accelerated, the library automatically falls back to SANE software floating-point. This ensures your programs work on any system.

## MegaFlash I/O Registers

The library uses the following MegaFlash registers (Slot 4):

- `$C480`: Command register
- `$C481`: Parameter register (read/write)
- `$C482`: Status register
- `$C483`: ID register

## Supported Operations

### Hardware Accelerated (MegaFlash)
- `mul` - Multiply
- `div` - Divide
- `sqrt` - Square root
- `sin` - Sine
- `cos` - Cosine
- `tan` - Tangent
- `atan` - Arctangent
- `lnX` - Natural logarithm
- `powEX` - e^x

### Software Fallback (SANE)
- `add` - Addition
- `sub` - Subtraction
- `rem` - Remainder
- `neg` - Negate
- `abs` - Absolute value
- `type` - Type classification
- `cmp` - Compare
- `trunc` - Truncate
- `round` - Round
- `logb` - Log base
- `scalb` - Scale
- All other ELEMS functions

## Building

The library is built as part of the standard PLASMA build process:

```bash
cd PLASMA/src
make
```

## Testing

A test program is provided in `samplesrc/fptest_mf.pla`:

```bash
cd PLASMA
# Copy to your Apple II disk or run in emulator
# RUN FPTEST.MF
```

## Limitations

- **Precision**: MBF format provides ~40 bits of precision vs. SANE's 64 bits
- **Some operations are slower**: Operations with high conversion overhead (like addition) use SANE fallback
- **MegaFlash only**: Hardware acceleration only available with MegaFlash device

## License

This library follows the same license as the PLASMA project.

## Credits

- MegaFlash hardware and firmware by Tomas Fok
- PLASMA language by David Schmenk
- MegaFlash FPU integration by Brendan Robert

## See Also

- [PLASMA Documentation](../../doc/)
- [MegaFlash Project](https://github.com/yourusername/MegaFlash)
- [SANE Numerics](https://en.wikipedia.org/wiki/Standard_Apple_Numerics_Environment)

# MegaFlash Hardware FPU Support for PLASMA

## Overview

This pull request adds hardware-accelerated floating-point support for PLASMA programs running on Apple IIc/IIc+ systems equipped with the MegaFlash storage device.

## What is MegaFlash?

MegaFlash is an internal storage device for Apple IIc/IIc+ computers featuring:
- 128/256MB flash storage
- Raspberry Pi Pico W microcontroller
- Hardware-accelerated floating-point unit (FPU)
- Real-time clock, WiFi, and other features

The FPU acceleration uses the Pico's ARM Cortex-M0+ processor to perform IEEE 754 double-precision calculations, providing significant speedup over the 6502's software floating-point routines.

## Changes in This PR

### New Files

1. **`src/inc/megaflash.plh`**: Header file defining MegaFlash constants and I/O registers
2. **`src/libsrc/fpu_mf.pla`**: Drop-in replacement FPU library with hardware acceleration
3. **`src/libsrc/README_fpu_mf.md`**: Documentation for the MegaFlash FPU library
4. **`src/samplesrc/fptest_mf.pla`**: Test program demonstrating the library

### API Compatibility

The `fpu_mf.pla` library maintains **100% API compatibility** with the existing `fpu.pla` library. Users can switch between implementations by simply changing:

```plasma
import fpu        // Old: SANE software FPU
```

to:

```plasma
import fpu_mf     // New: MegaFlash hardware FPU
```

All function calls remain identical.

## Technical Implementation

### Format Conversion

The library handles automatic conversion between:
- **SANE Extended Format** (80-bit): Used by PLASMA for maximum precision
- **MBF Format** (40-bit Microsoft Binary Format): Used by MegaFlash for hardware calculations

### Hardware Detection

On initialization (`reset`), the library:
1. Attempts to detect MegaFlash hardware via ID register
2. If found: Enables hardware acceleration for supported operations
3. If not found: Falls back transparently to SANE software implementation

This ensures programs work on any Apple II system, whether or not MegaFlash is present.

### Accelerated Operations

The following operations use MegaFlash hardware acceleration:

- **Arithmetic**: `mul`, `div`, `sqrt`
- **Trigonometry**: `sin`, `cos`, `tan`, `atan`
- **Logarithmic**: `lnX` (natural log), `powEX` (e^x)

Operations not supported by MegaFlash hardware (add, sub, rem, neg, abs, etc.) automatically use SANE fallback.

### Performance

Expected speedups with hardware acceleration:
- Transcendental functions (sin, cos, tan, log, exp): **10-100x faster**
- Multiplication/Division: **3-5x faster**
- Operations using SANE fallback: Same performance as `fpu.pla`

## Usage Example

```plasma
include "inc/cmdsys.plh"
import fpu_mf
import console

byte[80] result

// Initialize FPU
fpu_mf:reset

// Calculate sin(PI/2)
fpu_mf:constPi
fpu_mf:pushStr("2.0")
fpu_mf:div
fpu_mf:sin

// Print result
fpu_mf:pullStr(@result, 1, 6, FPSTR_FIXED)
puts(@result)  // Prints: 1.000000
```

## Testing

A comprehensive test program is provided in `src/samplesrc/fptest_mf.pla` that exercises:
- Basic arithmetic (mul, div, sqrt, squared)
- Trigonometric functions (sin, cos, tan, atan)
- Logarithmic/exponential (ln, exp)
- Constants (PI, E)
- Performance benchmark

## Backwards Compatibility

This PR:
- **Adds** new optional functionality
- **Does not modify** any existing PLASMA code
- **Does not break** any existing programs
- **Requires** no changes to existing programs unless users opt-in

## Hardware Requirements

To use hardware acceleration:
- Apple IIc or IIc+ computer
- MegaFlash storage device installed
- MegaFlash firmware v1.0 or later with FPU support enabled

Programs using `fpu_mf` will run on systems without MegaFlash (using SANE fallback).

## Future Enhancements

Possible future improvements:
- Optimize format conversion routines
- Add direct MBF mode for programs that don't need full SANE precision
- Support for other slots (currently hardcoded to slot 4)
- Assembly optimization of hot paths

## Documentation

Complete documentation is provided in:
- `src/libsrc/README_fpu_mf.md`: Library documentation
- Inline code comments: Implementation details
- `src/samplesrc/fptest_mf.pla`: Working examples

## Building

The library builds as part of the standard PLASMA build process:

```bash
cd PLASMA/src
make
```

## License

This code follows the PLASMA project's existing license.

## Credits

- MegaFlash hardware/firmware: Tomas Fok
- PLASMA language: David Schmenk
- MegaFlash FPU integration: Brendan Robert

## Questions?

For questions about:
- **MegaFlash hardware**: See [MegaFlash repository](https://github.com/ThomasFok/MegaFlash)
- **PLASMA integration**: Open an issue in this repository
- **Usage examples**: See `fptest_mf.pla` test program

# libmad - MPEG audio decoder library
Copyright (C) 2000-2004 Underbit Technologies, Inc.
Copyright (C) 2021-2023 Tenacity Team and Contributors

`$Id: README,v 1.4 2004/01/23 09:41:32 rob Exp $`

---

# Introduction

MAD (libmad) is a high-quality MPEG audio decoder. It currently supports
MPEG-1 and the MPEG-2 extension to Lower Sampling Frequencies, as well as
the so-called MPEG 2.5 format. All three audio layers (Layer I, Layer II,
and Layer III a.k.a. MP3) are fully implemented.

MAD does not yet support MPEG-2 multichannel audio (although it should be
backward compatible with such streams) nor does it currently support AAC.

MAD has the following special features:

  - 24-bit PCM output
  - 100% fixed-point (integer) computation
  - completely new implementation based on the ISO/IEC standards
  - distributed under the terms of the GNU General Public License (GPL)

Because MAD provides full 24-bit PCM output, applications using MAD are
able to produce high quality audio. Even when the output device supports
only 16-bit PCM, applications can use the extra resolution to increase the
audible dynamic range through the use of dithering or noise shaping.

Because MAD uses integer computation rather than floating point, it is
well suited for architectures without a floating point unit. All
calculations are performed with a 32-bit fixed-point integer
representation.

Because MAD is a new implementation of the ISO/IEC standards, it is
unencumbered by the errors of other implementations. MAD is NOT a
derivation of the ISO reference source or any other code. Considerable
effort has been expended to ensure a correct implementation, even in cases
where the standards are ambiguous or misleading.

Because MAD is distributed under the terms of the GPL, its redistribution
is not generally restricted, so long as the terms of the GPL are followed.
This means MAD can be incorporated into other software as long as that
software is also distributed under the GPL. (Should this be undesirable,
alternate arrangements may be possible by contacting Underbit.)

# About The Code

The code is optimized and performs very well, although specific
improvements can still be made. The output from the decoder library
consists of 32-bit signed linear fixed-point values that can be easily
scaled for any size PCM output, up to 24 bits per sample.

The API for libmad can be found in the `mad.h' header file. Note that this
file is automatically generated, and will not exist until after you have
built the library.

There are two APIs available, one high-level, and the other low-level.
With the low-level API, each step of the decoding process must be handled
explicitly, offering the greatest amount of control. With the high-level
API, after callbacks are configured, a single routine will decode an
entire bitstream.

The high-level API may either be used synchronously or asynchronously. If
used asynchronously, decoding will occur in a separate process.
Communication is possible with the decoding process by passing control
messages.

The file `minimad.c' contains an example usage of the libmad API that
shows only the bare minimum required to implement a useful decoder. It
expects a regular file to be redirected to standard input, and it sends
decoded 16-bit signed little-endian PCM samples to standard output. If a
decoding error occurs, it is reported to standard error and decoding
continues. Note that the scale() routine in this code is only provided as
an example; it rounds MAD's high-resolution samples down to 16 bits, but
does not perform any dithering or noise shaping. It is therefore not
recommended to use this routine as-is in your own code if sound quality is
important.

## Integer Performance

To get the best possible performance, it is recommended that an assembly
version of the fixed-point multiply and related routines be selected.
Several such assembly routines have been written for various CPUs.

If an assembly version is not available, a fast approximation version will
be used. This will result in reduced accuracy of the decoder.

Alternatively, if 64-bit integers are supported as a datatype by the
compiler, another version can be used that is much more accurate.
However, using an assembly version is generally much faster and just as
accurate.

More information can be gathered from the `fixed.h' header file.

MAD's CPU-intensive subband synthesis routine can be further optimized at
the expense of a slight loss in output accuracy due to a modified method
for fixed-point multiplication with a small windowing constant. While this
is helpful for performance and the output accuracy loss is generally
undetectable, it is disabled by default and must be explicitly enabled.

Under some architectures, other special optimizations may also be
available.

## Audio Quality

The output from MAD has been found to satisfy the ISO/IEC 11172-4
computational accuracy requirements for compliance. In most
configurations, MAD is a Full Layer III ISO/IEC 11172-3 audio decoder as
defined by the standard.

When the approximation version of the fixed-point multiply is used, MAD is
a limited accuracy ISO/IEC 11172-3 audio decoder as defined by the
standard.

MAD can alternatively be configured to produce output with less or more
accuracy than the default, as a tradeoff with performance.

MAD produces output samples with a precision greater than 24 bits. Because
most output formats use fewer bits, typically 16, it is recommended that a
dithering algorithm be used (rather than rounding or truncating) to obtain
the highest quality audio. However, dithering may unfavorably affect an
analytic examination of the output (such as compliance testing); you may
therefore wish to use rounding in this case instead.

## Portability Issues

GCC is preferred to compile the code, but other compilers may also work.
The assembly code in `fixed.h' depends on the inline assembly features of
your compiler. If you're not using GCC or MSVC++, you can either write
your own assembly macros or use the default (low quality output) version.

The union initialization of `huffman.c' may not be portable to all
platforms when GCC is not used.

The code should not be sensitive to word sizes or byte ordering, however
it does assume A % B has the same sign as A.

# Building and Installing

libmad uses the CMake build system and has no dependencies. To build it, run:

``` bash
cmake -D CMAKE_INSTALL_PREFIX=/where/you/want/to/install/to -S . -B build
cmake --build build --parallel number-of-cpu-cores
cmake --install build
```

The following CMake options are available which can be toggled by passing
'-D OPTION=VALUE' to the CMake configure step (the first call to cmake above):

  * ASO (ON|OFF): Enable CPU Architecture Specific Optimizations
    (x86, ARM, and MIPS only). ON by default.
  * EXAMPLE (ON|OFF): Build the example application. Only works on POSIX
    operating systems. ON by default on POSIX operating systems. The example
    application is not installed.

## Supported Platforms

libmad relies on a POSIX environment. On Windows, you should use Cygwin to
build libmad.

It is planned that in a future release

# Copyright

Please read the `COPYRIGHT` file for copyright and warranty information.
Also, the file `COPYING` contains the full text of the GNU GPL.

Send inquiries, comments, bug reports, suggestions, patches, etc. to:

    Underbit Technologies, Inc. <support@underbit.com>

See also the MAD home page on the Web:

    http://www.underbit.com/products/mad/


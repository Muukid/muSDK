

# muSDK v1.0.0

muSDK is a public domain* single-file C library for general high-level software development. Its header is automatically defined upon inclusion if not already included (`MUSDK_H`), and the source code is defined if `MUSDK_IMPLEMENTATION` is defined, following the internal structure of:

```c
#ifndef MUSDK_H
#define MUSDK_H
// (Header code)
#endif

#ifdef MUSDK_IMPLEMENTATION
// (Source code)
#endif
```

Therefore, a standard inclusion of the file to get all automatic functionality looks like:

```c
#define MUSDK_IMPLEMENTATION
#include "muSDK.h"
```

More information about the general structure of a mu library is provided at [the mu library information GitHub repository](https://github.com/Muukid/mu-library-information).

Clarification on the asterik next to "public domain" can be found in the [licensing section](#licensing).

# Licensing

muSDK is licensed under public domain or MIT, whichever you prefer, as well as [Apache 2.0 due to OpenGL's licensing](https://github.com/KhronosGroup/OpenGL-Registry/issues/376#issuecomment-596187053).

# Libraries

Since muSDK is a general software development kit, it contains various libraries whose code needs to be toggled manually with macros, as otherwise, all libraries would be defined automatically, which can be inefficient in many circumstances. This section covers every library included in muSDK and how to define it, as well as macros that apply to each library.

Note that the macros that activate these libraries need to be defined BEFORE `muSDK.h` is included for the first time in order to function properly.

## Turn on all name macros

All mu libraries included in muSDK have name functions to convert some of their enumerators to `const char*` representations. To define all of the name functions for all of the libraries, define `MUSDK_ALL_NAMES` before `muSDK.h` is included for the first time.


## muGraphics

muSDK has support for muGraphics ([commit a0b9746](https://github.com/Muukid/muGraphics/tree/a0b9746f02c5c615777f7514ec2f7fd71ca62068)). The macro to define it is `MUSDK_MUG`.

## muTrueType

muSDK has support for muTrueType ([commit f530d14](https://github.com/Muukid/muTrueType/tree/f530d14be2199adbef1324b08a3b830c7ee67275)). The macro to define it is `MUSDK_MUTT`.

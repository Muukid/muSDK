

# muSDK v1.0.0

muSDK is a public domain single-file C library for general high-level software development. Its header is automatically defined upon inclusion if not already included (`MUSDK_H`), and the source code is defined if `MUSDK_IMPLEMENTATION` is defined, following the internal structure of:

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

# Demos

Demos are designed for muSDK to both test its functionality and to allow users to get the basic idea of the structure of the library quickly without having to read the documentation in full. These demos are available in the `demos` folder.

## Demo resources

The demos use other files to operate correctly when running as a compiled executable. These other files can be found in the `resources` folder within `demos`, and this folder is expected to be in the same location that the program is executing from. For example, if a user compiles a demo into `main.exe`, and decides to run it, the `resources` folder from `demos` should be in the same directory as `main.exe`.

The resources' licenses may differ from the licensing of muSDK itself; see the [license section that covers demo resources](#licensing-of-demo-resources).

# Licensing

muSDK is licensed under public domain or MIT, whichever you prefer, as well as (technically) [Apache 2.0 due to OpenGL's licensing](https://github.com/KhronosGroup/OpenGL-Registry/issues/376#issuecomment-596187053).

## Licensing of demo resources

The resources used by the demos may differ from licensing of the demos themselves; in that context, their licenses apply, with licensing of each file available as a separate file with the same name, but with no filename extension.

# Libraries

Since muSDK is a general software development kit, it contains various libraries whose code needs to be toggled manually with macros, as otherwise, all libraries would be defined automatically, which can be inefficient in many circumstances. This section covers every library included in muSDK and how to define it, as well as macros that apply to each library.

Note that the macros that activate these libraries need to be defined BEFORE `muSDK.h` is included for the first time in order to function properly.

## Turn on all name macros

All mu libraries included in muSDK have name functions to convert some of their enumerators to `const char*` representations. To define all of the name functions for all of the libraries, define `MUSDK_ALL_NAMES` before `muSDK.h` is included for the first time.


## muGraphics

muSDK has support for [muGraphics v1.0.0](https://github.com/Muukid/muGraphics/releases/tag/v1.0.0). The macro to define it is `MUSDK_MUG`.

## muTrueType

muSDK has support for muTrueType ([commit f75d153](https://github.com/Muukid/muTrueType/tree/f75d153134eea147105b9de27dba475ddeb0866d)). The macro to define it is `MUSDK_MUTT`.



# muSDK v1.0.0

muSDK (SDK standing for software development kit) is a public domain* single-file C library for high-level software development, and exists as a collection of the mu libraries and their capabilities.

***WARNING!*** This library is still under heavy development, has no official releases, and won't be stable until its first public release v1.0.0.

To use it, download the latest stable release (or download the repository itself for the latest but unstable commit), extract `muSDK.h`, add it to your include path, and include it like so:

```c
#define MUSDK_IMPLEMENTATION
#include "muSDK.h"
```

More information about the general structure of a mu library is provided at [the mu library information GitHub repository](https://github.com/Muukid/mu-library-information).

Clarification on the asterik next to public domain can be found in the licensing section.

# Demos

Demos that quickly show the gist of the library and how it works whilst also testing the functionality of the library are available in the `demos` folder.

# Licensing

muSDK is licensed under public domain or MIT, whichever you prefer, as well as [Apache 2.0 due to OpenGL's licensing](https://github.com/KhronosGroup/OpenGL-Registry/issues/376#issuecomment-596187053).


# Libraries

Since muSDK is a collection of libraries, the respective code for the libraries needs to be toggled manually with macros, as otherwise, all libraries would be defined automatically, which can be inefficient in most circumstances. This section covers every single library included in muSDK and how to define it, as well as macros that apply to each library.

Note that the macros that turn these libraries on need to be defined BEFORE `muSDK.h` is included for the first time.

## muGraphics

muSDK has support for muGraphics ([commit feebeaf](https://github.com/Muukid/muGraphics/tree/feebeaf029ac970113b725bf7df696b0980e94f4)). The macro to define it is `MUSDK_MUG`.

## muTrueType

muSDK has support for muTrueType ([commit 5cbc300](https://github.com/Muukid/muTrueType/tree/5cbc300ea3b1187bf1953b101e8f95e49ba500ec)). The macro to define it is `MUSDK_MUTT`.

## Turn on all name macros

All mu libraries included in muSDK have name functions to convert some of their enumerators to `const char*` representations. To define all of the name functions for all of the libraries, define `MUSDK_ALL_NAMES` before `muSDK.h` is included for the first time.

# About

AnyBox is a compatibility layer for running Xbox Series X|S executables natively on Windows. Part of [AnyPS5](https://github.com/boykopovar/AnyPS5) ecosystem. The [shader recompiler from AnyPS5 core](https://github.com/boykopovar/AnyPS5/tree/main/core/shader/recompiler) is planned to be used.

## Status

Initial development. No playable titles confirmed.

## Building

AnyBox uses standard C++20 and can be built with any conforming compiler targeting 64-bit Windows. MSVC or `clang-cl` with the MSVC runtime is recommended for ABI compatibility and correct stack unwinding of diagnostic C++ exceptions through title code.

## Disclaimer

The project is intended for interoperability, research and preservation. Game files, firmware, cryptographic keys and proprietary SDK libraries are not distributed with the project.

AnyBox is an independent project and is not affiliated with, authorized or endorsed by Microsoft. Xbox is a trademark of Microsoft Corporation.

## License

This project is licensed under the [GNU General Public License version 2 only](LICENSE).

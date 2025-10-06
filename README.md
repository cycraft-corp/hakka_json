# Hakka JSON 🚀

**Blazingly small, memory-efficient JSON library for C++** ⚡

Modern C++ JSON library designed for **extreme memory efficiency**. Built with C++23, provides both C++ and C APIs.

[![License](https://img.shields.io/badge/license-BSL--1.0%20OR%20BSD--3--Clause-blue.svg)](LICENSE)
[![C++](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.20%2B-blue.svg)](https://cmake.org/)

## ✨ Features

- **💾 Memory Champion**: Minimal runtime footprint with string deduplication
- **📚 Read-Heavy Optimized**: Load once, read many times (R ≫ W)
- **🌍 Unicode Ready**: Full ICU integration
- **🔧 Dual APIs**: Modern C++ API + clean C API with C99 compatibility
- **🧪 Battle Tested**: Comprehensive test suite

## 🎯 When to Use

**Perfect when memory is your bottleneck!** 🌟

- Read-heavy workloads with repeated string values
- Memory-critical environments (embedded, mobile, IoT, large json data)
- Frequent small appends to JSON structures

[📖 **Full Documentation**](https://cycraft-corp.github.io/hakka_json/) | [🚀 **Getting Started**](https://cycraft-corp.github.io/hakka_json/)

## ⚡ Benchmark

TBD

## 🔨 Compiler Support

| Platform | Compiler | Versions | Status |
|----------|----------|----------|--------|
| **Linux** | GCC | 11, 12, 13, 14, 15, 16 | ![GCC](https://img.shields.io/github/actions/workflow/status/cycraft-corp/hakka_json/sanitize-build.yml?branch=main&label=GCC) |
| **Linux** | Clang/LLVM | 17, 18, 19, 20, 21, 22 | ![Clang](https://img.shields.io/github/actions/workflow/status/cycraft-corp/hakka_json/sanitize-build.yml?branch=main&label=Clang) |
| **macOS** | Apple Clang | Xcode 15.2, 15.4 | ![macOS](https://img.shields.io/github/actions/workflow/status/cycraft-corp/hakka_json/sanitize-build.yml?branch=main&label=macOS) |
| **Windows** | MSVC | VS 2022 | ![Windows](https://img.shields.io/github/actions/workflow/status/cycraft-corp/hakka_json/sanitize-build.yml?branch=main&label=Windows) |

**Sanitizers Tested**: Address, Undefined Behavior, Thread, Memory (Clang) | **Build Types**: Debug, Release

## 🤝 Contributing

Contributions welcome! See [CONTRIBUTING.md](CONTRIBUTING.md).

## 📄 License

Dual-licensed: [Boost Software License 1.0](LICENSE-BOOST) OR [BSD 3-Clause](LICENSE-BSD). See [LICENSE](LICENSE).

---
*Made with ❤️ by scc@cycraft*

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

### Memory Usage Comparison

**Test**: 481 MB [ClickHouse dataset from jsonbench.com](https://jsonbench.com/)

```mermaid
%%{init: {'theme':'base'}}%%
graph LR
    subgraph "Memory Usage (MB)"
        A["hakka_json<br/>1,490 MB<br/><b>⭐ 1.00x</b>"]
        B["golang<br/>2,258 MB<br/>1.52x"]
        C["python3.13<br/>2,321 MB<br/>1.56x"]
        D["cppstd<br/>2,770 MB<br/>1.86x"]
        E["python3.10<br/>2,808 MB<br/>1.88x"]
        F["jansson<br/>3,009 MB<br/>2.02x"]
        G["rust/serde_json<br/>3,326 MB<br/>2.23x"]
    end

    style A fill:#2ecc71,stroke:#27ae60,stroke-width:4px,color:#fff
    style B fill:#3498db,stroke:#2980b9,stroke-width:2px,color:#fff
    style C fill:#3498db,stroke:#2980b9,stroke-width:2px,color:#fff
    style D fill:#e74c3c,stroke:#c0392b,stroke-width:2px,color:#fff
    style E fill:#e74c3c,stroke:#c0392b,stroke-width:2px,color:#fff
    style F fill:#e74c3c,stroke:#c0392b,stroke-width:2px,color:#fff
    style G fill:#e74c3c,stroke:#c0392b,stroke-width:2px,color:#fff
```

| Library | Memory (MB) | Relative | File Size Multiplier |
|---------|-------------|----------|----------------------|
| **hakka_json** 🏆 | 1,490 | **1.00x** | **3.1x** |
| golang | 2,258 | 1.52x | 4.7x |
| python3.13 | 2,321 | 1.56x | 4.8x |
| cppstd | 2,770 | 1.86x | 5.8x |
| python3.10 | 2,808 | 1.88x | 5.8x |
| jansson | 3,009 | 2.02x | 6.3x |
| rust/serde_json | 3,326 | 2.23x | 6.9x |

**🎯 Key Results:**
- **33% less memory** than golang (2nd place)
- **55% less memory** than rust/serde_json (last place)
- **Only 3.1x file size** - competitors need 4.7x to 6.9x

**📊 Full benchmark details**: [tests/benchmark/README.md](tests/benchmark/README.md)

## 🔨 Compiler Support

| Platform | Compiler | Versions | Status |
|----------|----------|----------|--------|
| **Linux** | GCC | 11, 12, 13, 14, 15, 16 | ![GCC](https://img.shields.io/github/actions/workflow/status/cycraft-corp/hakka_json/linux-gcc.yml?branch=main&label=GCC) |
| **Linux** | Clang/LLVM | 17, 18, 19, 20, 21, 22 | ![Clang](https://img.shields.io/github/actions/workflow/status/cycraft-corp/hakka_json/linux-clang.yml?branch=main&label=Clang) |
| **macOS** | Apple Clang | Xcode 15.2, 15.4 | ![macOS](https://img.shields.io/github/actions/workflow/status/cycraft-corp/hakka_json/macos.yml?branch=main&label=macOS) |
| **Windows** | MSVC | VS 2022 | ![Windows](https://img.shields.io/github/actions/workflow/status/cycraft-corp/hakka_json/windows.yml?branch=main&label=Windows) |

**Sanitizers Tested**: Address, Undefined Behavior, Thread, Memory (Clang) | **Build Types**: Debug, Release

## 🤝 Contributing

Contributions welcome! See [CONTRIBUTING.md](CONTRIBUTING.md).

## 📄 License

Dual-licensed: [Boost Software License 1.0](LICENSE-BOOST) OR [BSD 3-Clause](LICENSE-BSD). See [LICENSE](LICENSE).

---
*Made with ❤️ by scc@cycraft*

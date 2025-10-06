# Contributing to Hakka JSON

First off, thanks for taking the time to contribute! 🎉
Contributions are what make open source great, and we welcome all forms of help — whether it's reporting bugs, improving documentation, or submitting code.

---

## 🐛 Reporting Issues

- Use the [GitHub Issues](../../issues) tracker.
- Clearly describe the problem:
  - What you expected to happen.
  - What actually happened.
  - Steps to reproduce (if applicable).
- Include environment details (OS, compiler, version).

---

## 💡 Suggesting Features

- Before opening a new feature request, check the [existing issues](../../issues) to avoid duplicates.
- Clearly explain your use case and why the feature is valuable.
- If possible, suggest how it could be implemented.

---

## 🔧 Development Setup

1. Fork the repository and clone your fork:
   ```bash
   git clone https://github.com/cycraft-corp/hakka_json.git
   cd hakka_json
   ```

2. Create a branch for your change:
   ```bash
   git checkout -b feature/your-feature
   ```

3. Build and test with CMake:
   ```bash
   cmake -S . -B build
   cmake --build build
   ctest --test-dir build
   ```

---

## ✅ Coding Guidelines

* Follow modern C++23 best practices.
* Keep the code clean, minimal, and memory-efficient.
* Write unit tests for new features or bug fixes.
* Ensure all tests pass before submitting a PR.

---

## 🔍 Pull Request Process

1. Update documentation if your change affects usage.
2. Make sure all tests pass (`ctest`).
3. Submit a Pull Request against the `main` branch.
4. A maintainer will review and may suggest changes.
5. Once approved, your PR will be merged 🎉.

---

## 🤝 Community Guidelines

* Be respectful and constructive in discussions.
* Assume good intent from others.
* Help foster an inclusive, collaborative environment.

---

*Thank you for contributing to Hakka JSON!* ❤️
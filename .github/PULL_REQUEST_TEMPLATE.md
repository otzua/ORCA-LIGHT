## Description

Briefly describe the change and motivation behind this Pull Request.

Fixes #(issue number)

---

## Type of Change

- [ ] Bug fix (non-breaking change which fixes an issue)
- [ ] New feature (non-breaking change which adds functionality)
- [ ] Performance optimization (improves latency, reduces memory/CPU usage)
- [ ] Documentation update
- [ ] Refactoring (no functional changes)

---

## Technical Verification Checklist

- [ ] My code adheres to the project code style in `.clang-format`.
- [ ] I have compiled and tested the code locally using MSVC (`build.bat`) or MinGW-w64.
- [ ] The build produces zero compilation warnings (`/W4` / `-Wall -Wextra`).
- [ ] No emojis have been added to code comments, commit messages, or UI copy.
- [ ] No external DLLs or third-party heavy frameworks have been introduced.
- [ ] Hot code paths (keystroke input, Direct2D rendering) do not perform dynamic memory allocations.
- [ ] Background worker threads respect `THREAD_PRIORITY_LOWEST` and yield cooperatively.
- [ ] Documentation (`README.md`, `ARCHITECTURE.md`, or `docs/`) has been updated where relevant.

---

## Memory & Performance Validation

- **Private Bytes (before / after)**:
- **Keystroke-to-paint latency impact**:

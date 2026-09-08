# Orca Light Documentation Index

Welcome to the comprehensive technical documentation for **Orca Light**. This directory contains detailed architecture notes, internal design specifications, algorithmic breakdowns, and subsystem references.

---

## Document Navigation Map

| Document | Primary Focus | Target Audience |
| :--- | :--- | :--- |
| [ARCHITECTURE_INTERNALS.md](ARCHITECTURE_INTERNALS.md) | Win32 message loop, threading model, and IPC synchronization | Systems Engineers / Contributors |
| [FUZZY_SEARCH_ALGORITHM.md](FUZZY_SEARCH_ALGORITHM.md) | Bitap / dynamic programming scoring rules and prefix boosts | Algorithm Engineers / Search Devs |
| [DIRECT2D_RENDER_PIPELINE.md](DIRECT2D_RENDER_PIPELINE.md) | Direct2D 1.1 double buffering, DWM blur, and glyph rasterization | Graphics Engineers |
| [INDEXER_SUBSYSTEM.md](INDEXER_SUBSYSTEM.md) | Asynchronous DFS crawling, disk throttling, and cache layout | Core Developers |
| [KEYBOARD_SHORTCUTS.md](KEYBOARD_SHORTCUTS.md) | Exhaustive keyboard mapping and modifier combinations | End Users / Power Users |
| [SECURITY_MODEL.md](SECURITY_MODEL.md) | Privilege separation, Zone.Identifier, and network isolation | Security Researchers / IT Admins |

---

## Root Documentation Reference

In addition to the documents in this folder, key root-level files provide high-level orientation:

- **[README.md](../README.md)**: Main project landing page, quickstart, and feature overview.
- **[ARCHITECTURE.md](../ARCHITECTURE.md)**: Executive architectural specification.
- **[CONFIGURATION.md](../CONFIGURATION.md)**: Complete parameter guide for `config.ini`.
- **[BENCHMARKS.md](../BENCHMARKS.md)**: Performance profiling, comparative metrics, and memory stability data.
- **[CONTRIBUTING.md](../CONTRIBUTING.md)**: Coding standards and pull request workflows.
- **[SECURITY.md](../SECURITY.md)**: Threat model and vulnerability disclosure policy.
- **[CHANGELOG.md](../CHANGELOG.md)**: Semantic versioning history.
- **[LICENSE](../LICENSE)**: MIT License terms.

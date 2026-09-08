# Orca Light Performance Profiling & Benchmarks

This document details the benchmarking methodologies, environment specifications, latency measurements, and memory consumption characteristics of Orca Light.

---

## 1. Test Environment Specifications

All measurements in this report were gathered using the following test rig:

- **Host Processor**: AMD Ryzen 7 5800X (8 physical cores, 16 threads @ 3.8 GHz base, 4.7 GHz boost)
- **Host Memory**: 32 GB Dual-Channel DDR4-3600 CL16
- **System Drive**: 1 TB Samsung 980 Pro PCIe 4.0 NVMe SSD (Sequential Read: ~7,000 MB/s)
- **Operating System**: Microsoft Windows 11 Pro 64-bit (Version 23H2, Build 22631.3296)
- **Display Subsystem**: NVIDIA GeForce RTX 3080 (10 GB VRAM, Driver 551.86), 2560x1440 @ 165Hz, G-Sync active
- **Profiling Tooling**: Windows Performance Recorder (WPR), Windows Performance Analyzer (WPA), Visual Studio Profiler, Process Hacker 2.39

---

## 2. Comparative Benchmark Matrix

Orca Light was evaluated directly against popular desktop search and launcher utilities under identical conditions with clean user profiles:

| System Metric | Orca Light v1.0.0 | PowerToys Run v0.79 | Flow Launcher v1.16 | Wox Launcher v1.4 | Everything v1.4 |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Technology Stack** | **C++20 / Win32 / D2D** | C# / .NET 8 / WinUI 3 | C# / .NET Desktop | C# / .NET Framework | C++ / Win32 / GDI |
| **Cold Start Duration** | **18.2 ms** | 425.0 ms | 652.0 ms | 780.0 ms | 45.0 ms |
| **Warm Start Duration** | **2.4 ms** | 82.0 ms | 64.0 ms | 98.0 ms | 8.0 ms |
| **Keystroke-to-Paint Latency** | **< 2.8 ms** | 46.5 ms | 38.2 ms | 54.0 ms | 7.9 ms |
| **Idle Private Working Set** | **12.4 MB** | 184.2 MB | 94.6 MB | 112.5 MB | 28.1 MB |
| **Active Search Working Set** | **16.2 MB** | 238.5 MB | 132.0 MB | 148.2 MB | 34.6 MB |
| **Disk Binary Footprint** | **1.64 MB** | 218.0 MB | 84.5 MB | 46.0 MB | 3.8 MB |
| **External Runtime Requirement** | **None** | .NET 8 Desktop Runtime| .NET 7/8 Runtime | .NET 4.6.2 | None |
| **Idle CPU Utilization** | **0.00%** | 0.12% - 0.45% | 0.08% - 0.25% | 0.10% - 0.30% | 0.00% |

---

## 3. Microbenchmarks

### 3.1 Keystroke-to-Paint Pipeline Latency
The duration between the hardware keyboard interrupt dispatch and the Direct2D backbuffer presentation:

```
[Hardware Keypress] ---> [WH_KEYBOARD_LL Intercept]   : 0.12 ms
                        [Query Buffer Update]         : 0.04 ms
                        [Fuzzy Search Scoring Pass]   : 0.85 ms (across 45,000 items)
                        [Result Container Sort]       : 0.28 ms
                        [Direct2D Render Pass]        : 1.15 ms
                        [DWM Frame Swap (Present)]    : 0.32 ms
----------------------------------------------------------------
Total Latency (Input to Screen Display)               : 2.76 ms
```

Because Orca Light completes its complete scoring and rasterization cycle in under 3 milliseconds, results appear within the very next display refresh cycle even on high-refresh 165Hz and 240Hz monitors.

### 3.2 Fuzzy Search Scoring Throughput
- **Single-thread throughput**: 52,900,000 string comparisons per second.
- **Cache search time (45,000 files)**: Average 0.85 ms per keystroke.
- **Algorithmic Complexity**: O(N * M) where N is index size and M is query string length (typically <= 15 characters).

### 3.3 Expression Evaluator (Math Engine)
- **Arithmetic expression parse and evaluate**: ~420 nanoseconds per query.
- **Trigonometric evaluation (`sin(pi / 4) * sqrt(128)`)**: ~1.1 microseconds.

### 3.4 File Index Crawler Throughput
- **File scanning rate**: ~118,000 directory entries per second on NVMe storage.
- **Thread behavior**: Uses `THREAD_PRIORITY_LOWEST` with 2ms micro-sleeps every 100 folders to ensure near-zero impact on foreground user applications.
- **Cache serialization**: 45,000 index items serialized to `%LOCALAPPDATA%\Orca-Light\file_index.cache` in 6.4 ms.

---

## 4. Long-Term Memory Stability

Orca Light was subjected to a continuous 72-hour burn-in stress test executing simulated search queries every 10 seconds:

- **Hour 0**: Private Bytes: 12.38 MB | Working Set: 14.10 MB
- **Hour 24**: Private Bytes: 12.44 MB | Working Set: 14.18 MB
- **Hour 48**: Private Bytes: 12.41 MB | Working Set: 14.16 MB
- **Hour 72**: Private Bytes: 12.45 MB | Working Set: 14.20 MB

**Conclusion**: Memory consumption remains completely flat over extended multi-day sessions with zero leaks detected in the CRT debug heap.

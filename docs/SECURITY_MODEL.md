# Security Model & Threat Assessment

This document outlines the security architecture, threat model, data isolation, and boundary enforcement implemented in Orca Light.

---

## 1. Principle of Least Privilege

Orca Light enforces strict adherence to least privilege:

1. **Standard User Context (`asInvoker`)**:
   Orca Light's embedded application manifest (`res/orca_light.manifest`) declares `requestedExecutionLevel level="asInvoker"`. Under no circumstances does the main process demand or request administrative elevation on startup.
2. **Controlled Elevation (`Ctrl + Shift + Enter`)**:
   When launching applications requiring administrative rights, Orca Light uses the standard Windows Shell verb `runas`:
   ```cpp
   SHELLEXECUTEINFOW sei = { sizeof(sei) };
   sei.lpVerb = L"runas";
   sei.lpFile = target_path.c_str();
   sei.nShow = SW_SHOWNORMAL;
   ShellExecuteExW(&sei);
   ```
   This triggers the standard Windows User Account Control (UAC) prompt, ensuring that elevation decisions remain under explicit user control.

---

## 2. Mark of the Web (MOTW) & SmartScreen Handling

### 2.1 The MOTW Threat & Friction
When archive files are downloaded via web browsers, Windows attaches a `Zone.Identifier` alternate data stream (ADS) indicating internet origin. Windows SmartScreen may flag unsigned native binaries extracted from such archives.

### 2.2 Mitigation in Installation Scripts
Orca Light's `install.bat` and `run.bat` scripts proactively strip this metadata stream:
```bat
(echo. > "%~dp0Orca-Light.exe:Zone.Identifier") 2>nul
del "%~dp0Orca-Light.exe:Zone.Identifier" >nul 2>&1
```
This prevents silent system blocks without modifying Windows system-wide security settings.

---

## 3. Network Isolation & Outbound Data Containment

Orca Light operates as an offline-first desktop tool:
- **Zero Outbound Telemetry**: No usage telemetry, search query telemetry, error beacons, or pingbacks exist within the codebase.
- **Strict HTTPS for Currency Rates**: The currency engine connects only to public exchange rate endpoints via `WinInet` (`INTERNET_FLAG_SECURE | INTERNET_FLAG_RELOAD`).
- **No Inbound Listening Ports**: Orca Light does not bind to any TCP or UDP ports, presenting zero network attack surface.

---

## 4. Input Sanitization & Memory Safety

- **Fixed Buffers & Safe String Operations**: Input strings are truncated to reasonable length limits (256 characters for search queries) to prevent buffer exhaustion.
- **Math Engine AST Depth Limiting**: The math parser imposes an explicit stack depth recursion limit (32 levels of nested parentheses) to prevent stack overflow attacks via crafted mathematical expressions.
- **Shell Escaping**: Command dispatching uses `ShellExecuteExW` with explicit parameter arrays rather than string interpolation passed into `cmd.exe /c`, mitigating command injection risks.

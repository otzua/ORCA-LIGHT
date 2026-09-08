# Security Policy

Orca Light takes security, privacy, and host system integrity seriously. Because this application operates with low-level keyboard hooks, interacts with the Windows Shell, and executes user-specified commands, strict boundaries are enforced.

---

## Supported Versions

Only the latest release of Orca Light receives official security updates and patches.

| Version | Supported          |
| ------- | ------------------ |
| 1.0.x   | Yes                |
| < 1.0   | No                 |

---

## Threat Model & Security Boundaries

### 1. Privilege Boundaries
- Orca Light runs strictly with standard user privileges (`asInvoker`).
- It does not require or request UAC administrative elevation on startup.
- Child processes are launched via `ShellExecuteExW` with the exact privilege token of the caller unless the user explicitly invokes `Ctrl + Shift + Enter`, which passes `runas` to prompt the Windows UAC consent dialog.

### 2. Network Isolation
- Orca Light makes zero outgoing connections for telemetry, analytics, updates, or diagnostics.
- Outbound network activity is strictly limited to currency rate synchronization over HTTPS using standard Windows `WinInet`.
- No user queries, clipboard data, keystrokes, or local directory indices are ever serialized or sent across the network.

### 3. Binary Integrity & SmartScreen MOTW
- Windows SmartScreen attaches an NTFS Alternate Data Stream (`Zone.Identifier`) to files downloaded from web browsers, marking them with Mark of the Web (MOTW).
- The `install.bat` and `run.bat` scripts proactively strip this stream to prevent silent blocks or security warnings on unsigned developer builds.
- Checksums (SHA-256) are generated for all official release archives.

---

## Reporting a Vulnerability

If you discover a security vulnerability or privilege escalation bug in Orca Light, please report it privately:

1. **Do not create a public GitHub issue.**
2. Send an email to **sglaxy936@gmail.com** with the subject `[SECURITY] Orca Light Vulnerability Report`.
3. Provide a detailed summary, including:
   - Specific version of Orca Light.
   - Operating system version and architecture.
   - Step-by-step reproduction steps or Proof of Concept (PoC).
   - Potential impact of the vulnerability.

We will acknowledge receipt within 48 hours and provide a remediation timeline. Once resolved, security advisories will be published alongside patched releases.

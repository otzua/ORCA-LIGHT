# Keyboard Shortcuts & Navigation Matrix

This reference documents the complete keyboard matrix, activation keys, modifier chords, and navigation controls available in Orca Light.

---

## 1. Global Activation Shortcuts

| Keystroke | Function | Notes |
| :--- | :--- | :--- |
| `Win + Space` | Toggle Orca Light (Show/Hide) | Primary global hotkey registered on startup |
| `Ctrl + Space` | Toggle Orca Light (Show/Hide) | Automatic fallback hotkey if Win+Space is claimed |
| `Ctrl + Shift + V` | Open Clipboard History | Opens launcher directly in clipboard history mode |

---

## 2. Search & Navigation Controls

| Keystroke | Context | Functional Behavior |
| :--- | :--- | :--- |
| `Up Arrow` | Results List | Move active highlight up one result |
| `Down Arrow` | Results List | Move active highlight down one result |
| `Page Up` | Results List | Jump selection up by 5 results |
| `Page Down` | Results List | Jump selection down by 5 results |
| `Home` | Results List | Jump directly to the top result (index 0) |
| `End` | Results List | Jump directly to the bottom visible result |
| `Enter` | Any Result | Execute highlighted result / copy math answer |
| `Ctrl + Enter` | File / Folder | Open containing folder in Windows File Explorer |
| `Ctrl + Shift + Enter` | Application | Execute target application as Administrator (UAC runas) |
| `Ctrl + Shift + C` | Any Result | Copy absolute file path or evaluated value to clipboard |
| `Escape` | Query Active | Clear current query string |
| `Escape` | Query Empty | Dismiss launcher window immediately |

---

## 3. Query Trigger Prefixes

Special character prefixes allow direct targeting of specialized subsystems:

| Trigger Prefix | Target Subsystem | Example Query | Expected Action |
| :--- | :--- | :--- | :--- |
| `>` | System Commands | `> shutdown` | Initiates safe operating system shutdown |
| `>` | System Commands | `> restart` | Reboots the operating system |
| `>` | System Commands | `> lock` | Locks user session immediately |
| `>` | System Commands | `> sleep` | Suspends system to low-power sleep state |
| `>` | System Commands | `> empty recycle bin` | Clears all recycle bins on all drives |
| `>` | System Commands | `> task manager` | Spawns Windows Task Manager |
| `>` | System Commands | `> settings` | Opens Windows Settings app |
| `>` | System Commands | `> exit` | Quits Orca Light daemon process |
| `cb` or `@clip` | Clipboard History | `cb meeting notes` | Searches clipboard history entries |
| Math Syntax | Calculator | `(45 * 12) / sqrt(16)` | Instantly evaluates expression |
| Currency Syntax | Currency Engine | `$50 in EUR` | Converts foreign exchange rate |

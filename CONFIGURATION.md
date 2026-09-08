# Orca Light Configuration Guide & Reference

This document provides a comprehensive guide to configuring Orca Light via the `config.ini` file.

---

## 1. File Location & Precedence

Orca Light searches for `config.ini` in the following locations in order of priority:

1. **Current Working Directory**: Alongside `Orca-Light.exe` (Portable mode).
2. **Local Application Data**: `%LOCALAPPDATA%\Orca-Light\config.ini` (Installed mode).

If no configuration file exists at either path, Orca Light automatically creates a clean default `config.ini` in `%LOCALAPPDATA%\Orca-Light\`.

---

## 2. Complete INI Schema

Below is an annotated template illustrating all available sections and configuration parameters.

```ini
; ==============================================================================
; Orca Light Configuration File
; ==============================================================================

[General]
; Primary hotkey combination for summoning Orca Light.
; Supported values: "Win + Space", "Ctrl + Space", "Alt + Space", "Ctrl + Shift + Space"
; Default: Win + Space
hotkey=Win + Space

; Enable or disable automatic Windows startup on user login.
; Supported values: true, false
; Default: true
autostart=true

; Maximum number of search results displayed in the dropdown list.
; Supported range: 3 to 15
; Default: 9
max_results=9

; Maximum number of text history records stored in the clipboard ring buffer.
; Supported range: 10 to 200
; Default: 50
max_clipboard_items=50

; UI entrance animation duration in milliseconds.
; Set to 0 to disable entrance animation completely for instantaneous display.
; Supported range: 0 to 300
; Default: 150
animation_duration_ms=150

; Global dark mode accent color in hex notation (RRGGBB).
; Default: 0A84FF (Electric Blue)
accent_color=0A84FF

; Window width in logical pixels (automatically scaled by Per-Monitor DPI).
; Default: 680
window_width=680

; Window height per search result item in logical pixels.
; Default: 56
item_height=56


[Directories]
; Explicit custom directories to include in the asynchronous file indexer.
; Environment variables such as %USERPROFILE% and %APPDATA% are automatically expanded.
; You may specify numbered keys (dir1, dir2, ...) or raw paths.
dir1=%USERPROFILE%\Desktop
dir2=%USERPROFILE%\Documents
dir3=%USERPROFILE%\Downloads
; dir4=D:\Code
; dir5=E:\Assets


[Excludes]
; Directory names and path fragments that the asynchronous indexer must skip.
; Matching is performed against relative directory segments during traversal.
exclude1=.git
exclude2=.svn
exclude3=node_modules
exclude4=AppData\Local\Temp
exclude5=AppData\Local\Microsoft
exclude6=$Recycle.Bin
exclude7=Windows\WinSxS
exclude8=System Volume Information
exclude9=__pycache__
exclude10=.vscode
exclude11=.idea
exclude12=dist
exclude13=build
exclude14=target


[Search]
; Minimum fuzzy match confidence score required for an item to appear in results.
; Lower values return broader matches; higher values enforce strict relevance.
; Supported range: 10 to 100
; Default: 25
min_score_threshold=25

; Weight multiplier applied to installed applications versus generic files.
; A multiplier of 2.0 ensures applications always sort above files with matching names.
; Default: 2.0
app_priority_multiplier=2.0

; Maximum file size in megabytes indexed for direct search.
; Files larger than this value are still indexed by name, but metadata inspection is skipped.
; Default: 500
max_indexed_file_size_mb=500


[Calculator]
; Precision for floating point mathematical evaluations.
; Supported range: 2 to 12 decimal places
; Default: 6
decimal_precision=6

; Default angle mode for trigonometric functions (sin, cos, tan).
; Supported values: "radians", "degrees"
; Default: radians
angle_mode=radians


[Currency]
; Base currency code for relative exchange conversions.
; Default: USD
base_currency=USD

; Interval in hours between background currency exchange rate synchronizations.
; Set to 0 to disable automatic online sync and use offline tables.
; Default: 24
sync_interval_hours=24


[Logging]
; Diagnostic logging verbosity.
; Supported values: "error", "warn", "info", "debug"
; Default: info
log_level=info

; Path to diagnostic log file.
; Default: %LOCALAPPDATA%\Orca-Light\orca-light.log
log_path=%LOCALAPPDATA%\Orca-Light\orca-light.log
```

---

## 3. Section Explanations

### 3.1 `[General]`
Controls runtime lifecycle properties, keyboard registration, and UI dimensions:
- **`hotkey`**: Specifies the primary key combination. If the requested hotkey cannot be registered because another program has claimed it, Orca Light automatically registers `Ctrl + Space` as a fallback.
- **`autostart`**: When set to `true`, the installer registers an entry in `HKCU\Software\Microsoft\Windows\CurrentVersion\Run`.
- **`max_results`**: Controls visual density. Setting this to 5 creates an ultra-compact HUD; setting to 9 or 12 gives a comprehensive list.

### 3.2 `[Directories]`
Custom roots for the background file crawler:
- Common directories like Desktop and Documents are indexed by default.
- Any additional local drive letters (e.g. `D:\`, `E:\Projects`) can be added here.
- Network drives (mapped UNC paths) are supported, though local drives are recommended for optimal traversal speed.

### 3.3 `[Excludes]`
Specifies directories that must be omitted during file index crawling. Skipping massive dependency trees (such as `node_modules` or `.git`) prevents unnecessary disk thrashing and keeps search results relevant.

### 3.4 `[Search]`
Fine-tunes the fuzzy matching algorithms:
- `app_priority_multiplier` gives applications a numerical advantage over plain files.
- `min_score_threshold` filters out low-probability false matches for short queries.

---

## 4. Applying Changes

After editing `config.ini`:
1. Open Orca Light.
2. Type `> exit` and press `Enter` to quit the application.
3. Re-launch Orca Light via `run.bat` or your Start Menu shortcut.
4. The updated configuration will be parsed and applied immediately.

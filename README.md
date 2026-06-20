# MyUtils
An easy utility manager for you lazy efficient folk.

Your easy utiltiy manager for, .cmd, .bat, .ps1, and .exe files with more automation lists to come. We all hate that feeling of scrolling through folders looking for a script or running tree 3000 times on linux. Here, there's no worry, just plop your supported filetypes in the same folder directory and bam you're done.

## Features
 
- **Zero-config auto-discovery** — recursively scans a folder tree for executables and scripts. No JSON/YAML config file to write or maintain.
- **Type-aware menus** — `Executables (.exe)`, `Batch Scripts (.bat)`, `Command Scripts (.cmd)`, and `PowerShell Scripts (.ps1)` each get their own menu, but only appear if a file of that type actually exists somewhere in the tree.
- **Keyboard-only navigation** — arrow keys to move, Enter or Space to select, Esc or a pinned QUIT/BACK bar to exit. No mouse required.
- **Single static binary** — no installer, no .NET/VC++ Redistributable dependency, no DLLs to ship alongside it.
- **Persistent default folder** — set a default scan directory once, then just run `myutils` from anywhere.

### Manual download
 
Grab the latest `myutils.exe` from the [Releases](../../releases) page. It's a single portable executable — no installation required, just run it.

**Note:** This binary is not code-signed. Windows SmartScreen may show an "Unknown Publisher" warning the first time you run it — this is expected for an unsigned indie tool. Click **More info → Run anyway** to proceed. You can verify the download against the SHA-256 checksum published with each release.

## Usage
 
```
myutils                       Scan the saved folder, or the current folder if none is saved
myutils -dir "C:\path"        Scan a specific folder for this run only
myutils "C:\path"             Same as above (bare path also accepted)
myutils -dir set "C:\path"    Save a default folder, then scan it
myutils -dir show             Print the currently saved folder
myutils -dir clear            Forget the saved folder
myutils -h, --help            Show usage
```

### Typical setup
 
Set a default folder once:
 
```
myutils -dir set "C:\Scripts"
```
 
From then on, just run `myutils` from anywhere and it scans `C:\Scripts` automatically.
 
### Folder resolution order
 
When run with no arguments, MyUtils picks a folder to scan in this order:
 
1. A folder passed on the command line (`-dir` or a bare path) — this run only, nothing is saved.
2. A previously saved default folder (set via `-dir set`).
3. The current working directory.
4. The folder the `MyUtils.exe` file itself lives in (useful when double-clicking a portable copy dropped next to your scripts).
### Controls
 
| Key | Action |
|---|---|
| `↑` / `↓` | Move selection |
| `Home` / `End` | Jump to first / last item |
| `PgUp` / `PgDn` | Jump a page at a time |
| `Enter` / `Space` | Select / launch |
| `→` | Same as Enter/Space |
| `←` | Back (same as Esc) |
| `Esc` | Back one level, or quit from the main menu |
 
## How it works
 
On startup, MyUtils resolves the folder to scan (see resolution order above), then walks it and every subfolder looking for files ending in `.exe`, `.bat`, `.cmd`, or `.ps1`. It builds one menu per file type found — empty categories are skipped entirely, and if nothing turns up at all, the main menu just shows **"No utilities loaded"**.
 
Selecting a file launches it in the same console window; once it exits, MyUtils redraws and you're back at the menu.
 
A small per-user config file at `%APPDATA%\MyUtils\config.json` stores the saved default folder (if you set one with `-dir set`). This location is used specifically because it survives `winget upgrade` — the folder winget actually installs the executable into is replaced on every upgrade, so nothing should be saved there.
 
## Building from source
 
Requirements: Visual Studio 2022 (or later) with the Desktop development with C++ workload, C++17 or later.
 
1. Clone the repo and open `myutils.slnx` (or the `.sln`) in Visual Studio.
2. Set the configuration to **Release** / **x64**.
3. Build → Rebuild Solution.
4. The compiled binary will be at `x64\Release\myutils.exe`.
The project links the C++ runtime statically (`/MT` in Release), so the resulting `.exe` has no external runtime dependency and runs on a clean Windows machine without needing the Visual C++ Redistributable installed.
 
## License
 
MIT — see [LICENSE](LICENSE).
 
## Author
 
Ray Bunton, 2026

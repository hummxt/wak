# Wakbg

Windows app for live desktop backgrounds. Procedural gradients and motion run on the GPU (DirectX 11 / HLSL) — not video files.

Built by **Hummet Azim** (software engineer) for personal use.

- Website: [hummet.dev](https://www.hummet.dev)
- LinkedIn: [linkedin.com/in/hummet](https://www.linkedin.com/in/hummet/)
- GitHub: [github.com/hummxt](https://github.com/hummxt)

---

## Features

- Animated mesh, fluid, aurora, wave, liquid, noise, abstract, and still gradient presets
- Live preview before apply
- Color / speed / intensity / brightness controls
- Runs behind desktop icons on Windows 10 / 11
- Native C++ — no Electron / browser runtime
- Tray mode: close the UI and the wallpaper keeps running
- Optional start-with-Windows restore after reboot (`--background`)

Settings live in `%AppData%\Wakbg\config.json`.

---

## Stack

| Layer | Tech |
|---|---|
| App / engine | C++17 |
| UI | Dear ImGui |
| Rendering | DirectX 11 + HLSL |
| Desktop host | Win32 (WorkerW / Progman + DirectComposition on Win11) |
| Build | CMake + MSVC |
| Config / presets | JSON |

---

## Build

Requirements: Windows 10/11, Visual Studio Build Tools (or VS) with C++ workload, CMake.

```powershell
.\scripts\build.ps1
```

Or:

```powershell
cmake -S . -B build -G "Visual Studio 18 2026" -A x64
cmake --build build --config Release
```

Output: `build\Release\Wakbg.exe`

---

## Run

```powershell
.\build\Release\Wakbg.exe
```

1. Pick a preset in Library  
2. Adjust motion and colors  
3. Click **Apply to Desktop**  
4. Close the window — wallpaper stays via the tray (and after reboot if start-with-Windows is on)

Tray → **Stop Wallpaper and Exit** restores the normal Windows background.

---

## Layout

```text
src/           app, renderer, desktop host, UI
shaders/       HLSL
library/       preset JSON
config/        defaults
assets/        logo / palettes / thumbnails
third_party/   Dear ImGui
```

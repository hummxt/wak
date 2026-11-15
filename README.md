# Wakbg

**Wakbg** is a lightweight Windows app for live desktop backgrounds — procedural gradients that run on the GPU, not video files.

Quiet, fast, and meant to stay out of the way. Pick a look, tweak colors, apply it behind your icons, close the window. The wallpaper keeps running from the tray until you stop it.

---

## What it does

- Animated mesh, fluid, aurora, wave, liquid, noise, and abstract backgrounds (i will add more animations and make the resolution better)
- Live preview before you apply
- Per-preset color controls (four palette colors, speed, intensity, brightness)
- Runs behind desktop icons on Windows 10 / 11
- Low footprint: native C++, DirectX 11, no Electron / browser runtime

Close the UI anytime — right-click the **Wakbg** tray icon to open it again or **Stop Wallpaper and Exit**.

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

Animations are shader-driven. Only the active preset is loaded.

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

1. Browse the library and select a preset  
2. Adjust motion and colors in the preview panel  
3. Click **Apply to Desktop**  
4. Close the window — wallpaper stays active via the tray  

---

## Project layout

```text
src/           app, renderer, desktop host, UI
shaders/       HLSL presets (mesh, fluid, aurora, …)
library/       preset JSON metadata
config/        default settings
assets/        palettes / thumbnails
third_party/   Dear ImGui
```

---

## Notes

- Win11 “raised desktop” uses DirectComposition so the wallpaper composites correctly behind icons.
- Prefer measuring RAM/CPU on your machine; targets are roughly tens of MB and capped FPS (default 30).
- This is not a video wallpaper player — everything is generated in real time.

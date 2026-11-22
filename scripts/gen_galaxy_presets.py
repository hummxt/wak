#!/usr/bin/env python3
"""Galaxy presets matched to reference wallpaper styles."""

from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1] / "library" / "presets"


def write(preset: dict) -> None:
    path = ROOT / f"{preset['id']}.json"
    path.write_text(json.dumps(preset, indent=2) + "\n", encoding="utf-8")
    print("write", path.name)


def p(id_, name, description, colors, **kw):
    out = {
        "id": id_,
        "name": name,
        "category": "galaxy",
        "shader": "galaxy/Galaxy",
        "description": description,
        "thumbnail": f"thumbnails/{id_}.png",
        "colorA": colors[0],
        "colorB": colors[1],
        "colorC": colors[2],
        "colorD": colors[3],
    }
    out.update(kw)
    return out


PRESETS = [
    p(
        "galaxy-violet-nebula",
        "Violet Nebula",
        "Dense purple nebula with bright star bloom — soft parallax drift.",
        [[0.01, 0.0, 0.03, 1], [0.18, 0.05, 0.35, 1], [0.55, 0.2, 0.85, 1], [0.95, 0.85, 1.0, 1]],
        speed=0.22, intensity=0.95, brightness=1.0, zoom=1.05, freq=5.0, sharpness=2.2,
        amplitude=1.15, waveWidthMod=0.15, offsetX=0.0, offsetY=0.05,
        localWarpIntensity=0.85, localWarpFreqY=1.6,
    ),
    p(
        "galaxy-andromeda",
        "Andromeda Drift",
        "Tilted spiral disk with warm core and blue arms.",
        [[0.01, 0.02, 0.06, 1], [0.12, 0.2, 0.45, 1], [0.45, 0.65, 0.95, 1], [1.0, 0.9, 0.7, 1]],
        speed=0.18, intensity=0.9, zoom=1.2, freq=4.5, sharpness=2.6,
        amplitude=1.25, waveWidthMod=0.85, offsetX=0.08, offsetY=-0.05,
        localWarpIntensity=0.55, localWarpFreqY=1.4,
    ),
    p(
        "galaxy-milky-band",
        "Milky Band",
        "Diagonal milky-way style star river with warm dust.",
        [[0.01, 0.02, 0.05, 1], [0.15, 0.18, 0.35, 1], [0.75, 0.45, 0.35, 1], [1.0, 0.85, 0.7, 1]],
        speed=0.16, intensity=0.85, zoom=0.95, freq=6.0, sharpness=2.0,
        amplitude=1.05, waveWidthMod=0.25, offsetX=-0.05, offsetY=0.1,
        localWarpIntensity=0.7, localWarpFreqY=1.8,
    ),
    p(
        "galaxy-deep-field",
        "Deep Field",
        "Vast dark starfield with faint brown/blue dust.",
        [[0.005, 0.008, 0.02, 1], [0.06, 0.05, 0.08, 1], [0.12, 0.15, 0.25, 1], [0.85, 0.88, 0.95, 1]],
        speed=0.2, intensity=1.05, zoom=1.0, freq=3.5, sharpness=1.6,
        amplitude=0.75, waveWidthMod=0.05, offsetX=0.0, offsetY=0.0,
        localWarpIntensity=0.4, localWarpFreqY=1.2,
    ),
    p(
        "galaxy-magenta-haze",
        "Magenta Haze",
        "Indigo-to-magenta nebula clouds packed with stars.",
        [[0.01, 0.02, 0.08, 1], [0.15, 0.1, 0.45, 1], [0.7, 0.25, 0.85, 1], [0.9, 0.85, 1.0, 1]],
        speed=0.24, intensity=0.95, zoom=1.1, freq=5.2, sharpness=2.1,
        amplitude=1.2, waveWidthMod=0.2, offsetX=-0.1, offsetY=0.12,
        localWarpIntensity=0.95, localWarpFreqY=1.7,
    ),
    p(
        "galaxy-teal-cloud",
        "Teal Cloud",
        "Teal/emerald nebula with purple highlights.",
        [[0.01, 0.03, 0.05, 1], [0.05, 0.25, 0.28, 1], [0.25, 0.75, 0.55, 1], [0.75, 0.35, 0.85, 1]],
        speed=0.21, intensity=0.9, zoom=1.05, freq=4.8, sharpness=2.0,
        amplitude=1.1, waveWidthMod=0.18, offsetX=0.05, offsetY=-0.08,
        localWarpIntensity=0.9, localWarpFreqY=1.5,
    ),
    p(
        "galaxy-runaway",
        "Runaway Cluster",
        "Crisp star cluster with a faint distant galaxy glow.",
        [[0.0, 0.01, 0.03, 1], [0.08, 0.12, 0.25, 1], [0.35, 0.45, 0.75, 1], [1.0, 0.95, 0.9, 1]],
        speed=0.26, intensity=1.1, zoom=1.15, freq=4.0, sharpness=2.8,
        amplitude=1.0, waveWidthMod=0.55, offsetX=0.18, offsetY=0.0,
        localWarpIntensity=0.5, localWarpFreqY=1.3,
    ),
]


def main() -> None:
    # Replace previous galaxy / constellation-backed entries.
    for path in ROOT.glob("galaxy-*.json"):
        path.unlink()
        print("remove", path.name)
    for preset in PRESETS:
        write(preset)
    print(f"wrote {len(PRESETS)}")


if __name__ == "__main__":
    main()

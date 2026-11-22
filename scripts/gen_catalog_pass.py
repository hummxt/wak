#!/usr/bin/env python3
"""Catalog cleanup pass: more noise/abstract/shader, galaxy starfields, no aurora/foil/stars."""

from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1] / "library" / "presets"


def write(preset: dict) -> None:
    path = ROOT / f"{preset['id']}.json"
    path.write_text(json.dumps(preset, indent=2) + "\n", encoding="utf-8")
    print("write", path.name)


def p(id_, name, category, shader, description, colors, **kw):
    out = {
        "id": id_,
        "name": name,
        "category": category,
        "shader": shader,
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
    # noise — keep grainy look, more colorways
    p("noise-beam", "Grain Beam", "noise", "noise/NoiseField",
      "Soft diagonal light beam with fine film grain.",
      [[0.05, 0.05, 0.06, 1], [0.22, 0.22, 0.24, 1], [0.72, 0.72, 0.74, 1], [0.95, 0.95, 0.96, 1]],
      speed=0.12, intensity=0.7, angle=135, zoom=1.1, freq=5, amplitude=0.9, grainIntensity=0.1),
    p("noise-ember-grey", "Ash Gradient", "noise", "noise/NoiseField",
      "Matte charcoal-to-silver grain field.",
      [[0.02, 0.02, 0.025, 1], [0.14, 0.14, 0.15, 1], [0.45, 0.45, 0.47, 1], [0.82, 0.82, 0.84, 1]],
      speed=0.08, intensity=0.55, angle=90, zoom=0.9, freq=4, amplitude=0.7, grainIntensity=0.12),
    p("noise-halo", "Soft Halo Grain", "noise", "noise/NoiseField",
      "Central glow through dense monochrome grain.",
      [[0.04, 0.04, 0.05, 1], [0.18, 0.18, 0.2, 1], [0.6, 0.6, 0.62, 1], [0.92, 0.92, 0.93, 1]],
      speed=0.1, intensity=0.8, angle=45, zoom=1.3, amplitude=1.1, grainIntensity=0.09),
    p("noise-wave-grey", "Grain Wave", "noise", "noise/NoiseField",
      "Wavy soft grey shapes under fine static.",
      [[0.06, 0.06, 0.07, 1], [0.25, 0.25, 0.27, 1], [0.55, 0.55, 0.58, 1], [0.88, 0.88, 0.9, 1]],
      speed=0.18, intensity=0.9, angle=20, zoom=1.4, freq=6, amplitude=1.2, grainIntensity=0.11),
    p("noise-blue", "Blue Grain", "noise", "noise/NoiseField",
      "Cool blue grain wash.",
      [[0.02, 0.05, 0.1, 1], [0.08, 0.18, 0.32, 1], [0.25, 0.45, 0.7, 1], [0.7, 0.85, 0.95, 1]],
      speed=0.12, intensity=0.7, angle=120, zoom=1.1, amplitude=0.95, grainIntensity=0.1),
    p("noise-violet", "Violet Grain", "noise", "noise/NoiseField",
      "Soft violet film-grain gradient.",
      [[0.06, 0.03, 0.1, 1], [0.2, 0.1, 0.3, 1], [0.45, 0.3, 0.65, 1], [0.85, 0.75, 0.95, 1]],
      speed=0.14, intensity=0.75, angle=50, zoom=1.2, amplitude=1.0, grainIntensity=0.1),
    p("noise-olive", "Olive Grain", "noise", "noise/NoiseField",
      "Muted olive grain field.",
      [[0.06, 0.07, 0.04, 1], [0.18, 0.22, 0.12, 1], [0.4, 0.48, 0.3, 1], [0.82, 0.85, 0.7, 1]],
      speed=0.1, intensity=0.65, angle=100, zoom=1.0, amplitude=0.85, grainIntensity=0.11),
    p("noise-rose", "Rose Grain", "noise", "noise/NoiseField",
      "Warm rose grain gradient.",
      [[0.1, 0.04, 0.06, 1], [0.3, 0.12, 0.16, 1], [0.65, 0.35, 0.4, 1], [0.95, 0.8, 0.8, 1]],
      speed=0.12, intensity=0.7, angle=160, zoom=1.15, amplitude=0.9, grainIntensity=0.1),
    p("noise-teal", "Teal Grain", "noise", "noise/NoiseField",
      "Teal-to-cream grain wash.",
      [[0.03, 0.08, 0.09, 1], [0.1, 0.28, 0.3, 1], [0.35, 0.65, 0.65, 1], [0.85, 0.92, 0.9, 1]],
      speed=0.11, intensity=0.68, angle=80, zoom=1.05, amplitude=0.9, grainIntensity=0.1),
    p("noise-amber", "Amber Grain", "noise", "noise/NoiseField",
      "Amber light beam through grain.",
      [[0.08, 0.04, 0.02, 1], [0.28, 0.14, 0.05, 1], [0.7, 0.45, 0.2, 1], [0.95, 0.85, 0.65, 1]],
      speed=0.13, intensity=0.8, angle=140, zoom=1.25, amplitude=1.05, grainIntensity=0.09),

    # abstract
    p("abstract-orbit", "Abstract Orbit", "abstract", "abstract/Abstract",
      "Abstract orbiting colour forms.",
      [[1.0, 0.37, 0.23, 1], [1.0, 0.18, 0.39, 1], [0.23, 0.11, 0.44, 1], [0.84, 0.43, 0.47, 1]],
      speed=0.45, intensity=0.8, angle=60),
    p("abstract-cyan", "Abstract Cyan", "abstract", "abstract/Abstract",
      "Cool cyan abstract rings.",
      [[0.02, 0.08, 0.16, 1], [0.05, 0.45, 0.65, 1], [0.35, 0.85, 0.95, 1], [0.85, 0.95, 1.0, 1]],
      speed=0.4, intensity=0.75, angle=30),
    p("abstract-lime", "Abstract Lime", "abstract", "abstract/Abstract",
      "Electric lime abstract spokes.",
      [[0.05, 0.08, 0.02, 1], [0.35, 0.75, 0.15, 1], [0.85, 0.95, 0.35, 1], [0.2, 0.45, 0.85, 1]],
      speed=0.5, intensity=0.9, angle=90),
    p("abstract-magenta", "Abstract Magenta", "abstract", "abstract/Abstract",
      "Hot magenta abstract orbit.",
      [[0.12, 0.02, 0.1, 1], [0.85, 0.15, 0.55, 1], [0.95, 0.45, 0.85, 1], [0.35, 0.65, 0.95, 1]],
      speed=0.48, intensity=0.85, angle=120),
    p("abstract-gold", "Abstract Gold", "abstract", "abstract/Abstract",
      "Warm gold abstract field.",
      [[0.12, 0.06, 0.02, 1], [0.75, 0.45, 0.1, 1], [0.95, 0.8, 0.35, 1], [0.45, 0.2, 0.35, 1]],
      speed=0.35, intensity=0.7, angle=45),
    p("abstract-ice", "Abstract Ice", "abstract", "abstract/Abstract",
      "Icy pale abstract rings.",
      [[0.08, 0.12, 0.18, 1], [0.45, 0.6, 0.75, 1], [0.8, 0.9, 0.95, 1], [0.95, 0.98, 1.0, 1]],
      speed=0.3, intensity=0.6, angle=15),

    # galaxy + former stars folded in
    p("galaxy-nebula", "Soft Nebula", "galaxy", "galaxy/Galaxy",
      "Soft animated nebula clouds with a quiet star field.",
      [[0.02, 0.02, 0.06, 1], [0.2, 0.12, 0.45, 1], [0.45, 0.35, 0.75, 1], [0.9, 0.85, 0.95, 1]],
      speed=0.22, intensity=0.85, zoom=1.05, freq=4.0, sharpness=2.0, amplitude=1.0, waveWidthMod=0.45),
    p("galaxy-starfield", "Deep Starfield", "galaxy", "stars/Constellation",
      "Sparse twinkling stars over deep space (ex-stars category).",
      [[0.02, 0.03, 0.08, 1], [0.08, 0.1, 0.22, 1], [0.25, 0.2, 0.45, 1], [0.95, 0.95, 1.0, 1]],
      speed=0.35, intensity=0.8, brightness=0.95, freq=5.0, amplitude=1.1),
    p("galaxy-ember-sky", "Ember Starfield", "galaxy", "stars/Constellation",
      "Warm ember stars on a dusk nebula.",
      [[0.08, 0.03, 0.06, 1], [0.22, 0.08, 0.12, 1], [0.45, 0.18, 0.28, 1], [1.0, 0.82, 0.55, 1]],
      speed=0.3, intensity=0.7, freq=4.2, amplitude=1.0),
    p("galaxy-cyan-dust", "Cyan Dust", "galaxy", "galaxy/Galaxy",
      "Cool cyan dust clouds and stars.",
      [[0.01, 0.03, 0.08, 1], [0.05, 0.3, 0.5, 1], [0.25, 0.7, 0.85, 1], [0.85, 0.95, 1.0, 1]],
      speed=0.25, intensity=0.9, zoom=1.15, freq=4.5, sharpness=2.2, amplitude=1.1, waveWidthMod=0.4),
    p("galaxy-rose-cloud", "Rose Cloud", "galaxy", "galaxy/Galaxy",
      "Rose nebula haze with soft stars.",
      [[0.04, 0.02, 0.05, 1], [0.45, 0.12, 0.28, 1], [0.8, 0.35, 0.5, 1], [0.95, 0.8, 0.75, 1]],
      speed=0.2, intensity=0.8, zoom=1.0, freq=3.8, sharpness=1.8, amplitude=0.95, waveWidthMod=0.5),

    # more shader colourways (same engine people liked)
    p("sg-coral-drift", "Coral Drift", "shader", "shadergrad/ShaderGradient",
      "Warm coral shader gradient.",
      [[0.15, 0.04, 0.05, 1], [0.75, 0.25, 0.25, 1], [0.95, 0.55, 0.35, 1], [0.95, 0.85, 0.65, 1]],
      speed=0.25, intensity=1.4, brightness=1.0, angle=40, zoom=0.95, freq=5.5, sharpness=1.2,
      amplitude=1.4, waveWidthMod=0.4, localWarpIntensity=0.9, grainIntensity=0.05, contrast=1.0),
    p("sg-forest-haze", "Forest Haze", "shader", "shadergrad/ShaderGradient",
      "Deep forest shader wash.",
      [[0.03, 0.08, 0.05, 1], [0.1, 0.28, 0.18, 1], [0.35, 0.65, 0.4, 1], [0.75, 0.9, 0.65, 1]],
      speed=0.2, intensity=1.2, angle=95, zoom=1.0, freq=4.8, sharpness=1.1,
      amplitude=1.2, waveWidthMod=0.35, localWarpIntensity=0.8, grainIntensity=0.04),
    p("sg-skyline", "Skyline", "shader", "shadergrad/ShaderGradient",
      "Bright skyline cyan-blue shader.",
      [[0.05, 0.15, 0.35, 1], [0.15, 0.45, 0.85, 1], [0.45, 0.8, 0.95, 1], [0.95, 0.95, 0.85, 1]],
      speed=0.3, intensity=1.5, angle=70, zoom=0.9, freq=6.0, sharpness=1.0,
      amplitude=1.5, waveWidthMod=0.42, localWarpIntensity=1.0, grainIntensity=0.05),
    p("sg-plum-night", "Plum Night", "shader", "shadergrad/ShaderGradient",
      "Plum night shader gradient.",
      [[0.06, 0.02, 0.08, 1], [0.25, 0.08, 0.28, 1], [0.55, 0.25, 0.55, 1], [0.9, 0.65, 0.85, 1]],
      speed=0.22, intensity=1.6, angle=130, zoom=1.05, freq=5.2, sharpness=1.15,
      amplitude=1.55, waveWidthMod=0.38, localWarpIntensity=1.1, grainIntensity=0.06),
    p("sg-honey", "Honey Glow", "shader", "shadergrad/ShaderGradient",
      "Honey amber shader glow.",
      [[0.12, 0.05, 0.02, 1], [0.55, 0.28, 0.08, 1], [0.9, 0.65, 0.25, 1], [0.98, 0.9, 0.65, 1]],
      speed=0.18, intensity=1.3, angle=25, zoom=0.92, freq=4.6, sharpness=1.05,
      amplitude=1.25, waveWidthMod=0.4, localWarpIntensity=0.75, grainIntensity=0.04),
    p("sg-mint-glass", "Mint Glass", "shader", "shadergrad/ShaderGradient",
      "Mint glass shader gradient.",
      [[0.04, 0.1, 0.1, 1], [0.15, 0.4, 0.38, 1], [0.45, 0.85, 0.75, 1], [0.9, 0.98, 0.9, 1]],
      speed=0.28, intensity=1.35, angle=55, zoom=0.98, freq=5.0, sharpness=1.1,
      amplitude=1.3, waveWidthMod=0.36, localWarpIntensity=0.85, grainIntensity=0.045),
]


def main() -> None:
    # remove galaxy old procedural set we are replacing names for
    for old in [
        "galaxy-spiral.json",
        "galaxy-rose.json",
        "galaxy-cyan.json",
        "galaxy-ember.json",
        "liquid-foil.json",
    ]:
        path = ROOT / old
        if path.exists():
            path.unlink()
            print("remove", old)

    for preset in PRESETS:
        write(preset)
    print(f"wrote {len(PRESETS)}")


if __name__ == "__main__":
    main()

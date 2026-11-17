#!/usr/bin/env python3
"""Generate Wakbg sg-* presets from extracted MagicPattern states."""

from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "insiraptions" / "mp_presets.json"
OUT = ROOT / "library" / "presets"

DEFAULTS = {
    "speed": 1.0,
    "zoom": 1.0,
    "freq": 4.0,
    "sharpness": 2.0,
    "amplitude": 1.0,
    "waveWidthMod": 0.5,
    "offsetX": 0.0,
    "offsetY": 0.0,
    "rotation": 0.0,
    "localWarpIntensity": 0.4,
    "localWarpFreqY": 1.9,
    "ditherEnabled": False,
    "ditherLevels": 16.0,
    "ditherScale": 4.0,
    "grainEnabled": False,
    "grainIntensity": 0.02,
    "grainSpeed": 50.0,
    "vignetteEnabled": False,
    "vignetteIntensity": 1.0,
    "vignetteRadius": 0.4,
    "brightness": 1.0,
    "contrast": 1.0,
    "animate": False,
}


def hex_to_rgba(h: str) -> list[float]:
    h = h.lstrip("#")
    return [
        round(int(h[0:2], 16) / 255.0, 4),
        round(int(h[2:4], 16) / 255.0, 4),
        round(int(h[4:6], 16) / 255.0, 4),
        1.0,
    ]


def main() -> None:
    presets = json.loads(SRC.read_text(encoding="utf-8"))
    # remove old approximate sg-* files
    for old in OUT.glob("sg-*.json"):
        old.unlink()

    for p in presets:
        st = {**DEFAULTS, **{k: v for k, v in p["state"].items() if v is not None}}
        colors = st.get("colors") or ["#000000", "#333333", "#666666", "#999999"]
        while len(colors) < 4:
            colors.append(colors[-1])
        colors = colors[:4]

        # MagicPattern animate:false => still wallpaper; keep tiny drift if they used speed
        anim_speed = 0.0 if not st.get("animate") else float(st.get("speed", 1.0)) * 0.35

        dither_levels = float(st["ditherLevels"]) if st.get("ditherEnabled") else 0.0
        grain = float(st["grainIntensity"]) if st.get("grainEnabled") else 0.0
        vig = float(st["vignetteIntensity"]) if st.get("vignetteEnabled") else 0.0

        preset_id = f"sg-{p['id']}"
        preset = {
            "id": preset_id,
            "name": p["name"],
            "category": "shader",
            "shader": "shadergrad/ShaderGradient",
            "description": f"Shader gradient port of MagicPattern preset: {p['name']}.",
            "thumbnail": f"thumbnails/{preset_id}.png",
            "defaultSettings": {
                "speed": anim_speed,
                "intensity": float(st["amplitude"]),
                "brightness": float(st["brightness"]),
                "angle": float(st["rotation"]),
                "zoom": float(st["zoom"]),
                "freq": float(st["freq"]),
                "sharpness": float(st["sharpness"]),
                "amplitude": float(st["amplitude"]),
                "waveWidthMod": float(st["waveWidthMod"]),
                "offsetX": float(st["offsetX"]),
                "offsetY": float(st["offsetY"]),
                "localWarpIntensity": float(st["localWarpIntensity"]),
                "localWarpFreqY": float(st["localWarpFreqY"]),
                "ditherLevels": dither_levels,
                "ditherScale": float(st["ditherScale"]),
                "grainIntensity": grain,
                "grainSpeed": float(st["grainSpeed"]),
                "vignetteIntensity": vig,
                "vignetteRadius": float(st.get("vignetteRadius", 0.4)),
                "contrast": float(st["contrast"]),
            },
            "colorA": hex_to_rgba(colors[0]),
            "colorB": hex_to_rgba(colors[1]),
            "colorC": hex_to_rgba(colors[2]),
            "colorD": hex_to_rgba(colors[3]),
        }
        (OUT / f"{preset_id}.json").write_text(json.dumps(preset, indent=2) + "\n", encoding="utf-8")
        print("wrote", preset_id)

    print("done", len(presets))


if __name__ == "__main__":
    main()

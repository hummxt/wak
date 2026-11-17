#!/usr/bin/env python3
"""Import Coolors CSS gradients from insiraptions into library/presets."""

from __future__ import annotations

import json
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "insiraptions" / "coolors-gradients-css.txt"
OUT = ROOT / "library" / "presets"


def hex_to_rgba(hex6: str) -> list[float]:
    h = hex6.strip().lstrip("#")
    r = int(h[0:2], 16) / 255.0
    g = int(h[2:4], 16) / 255.0
    b = int(h[4:6], 16) / 255.0
    return [round(r, 4), round(g, 4), round(b, 4), 1.0]


def lerp(a: list[float], b: list[float], t: float) -> list[float]:
    return [round(a[i] + (b[i] - a[i]) * t, 4) for i in range(4)]


def to_four_stops(colors: list[list[float]]) -> tuple[list[float], list[float], list[float], list[float]]:
    if len(colors) == 1:
        c = colors[0]
        return c, c, c, c
    if len(colors) == 2:
        a, d = colors[0], colors[1]
        return a, lerp(a, d, 0.33), lerp(a, d, 0.66), d
    if len(colors) == 3:
        a, mid, d = colors[0], colors[1], colors[2]
        return a, lerp(a, mid, 0.5), mid, d
    # 4+ : evenly sample
    last = len(colors) - 1
    idxs = [0, last // 3, (2 * last) // 3, last]
    return colors[idxs[0]], colors[idxs[1]], colors[idxs[2]], colors[idxs[3]]


def slugify(name: str) -> str:
    s = name.lower().strip()
    s = re.sub(r"[^a-z0-9]+", "-", s)
    return s.strip("-")


def parse_entries(text: str) -> list[tuple[str, list[str], float]]:
    parts = re.split(r"\n(?=\d+\.\s+)", text)
    entries: list[tuple[str, list[str], float]] = []
    for part in parts:
        m = re.match(r"(\d+)\.\s+(.+)", part.strip(), re.DOTALL)
        if not m:
            continue
        body = m.group(2)
        lines = body.splitlines()
        name = lines[0].strip()
        hexes: list[str] = []
        for line in lines[1:8]:
            if "#" in line and ("\u2192" in line or "->" in line or "→" in line):
                hexes = re.findall(r"#([0-9A-Fa-f]{6})", line)
                break
        if len(hexes) < 2:
            filt = re.search(
                r'startColorstr="#([0-9A-Fa-f]{6})".*endColorstr="#([0-9A-Fa-f]{6})"',
                body,
                re.IGNORECASE,
            )
            if filt:
                hexes = [filt.group(1), filt.group(2)]
        if len(hexes) < 2:
            continue
        ang = 90.0
        am = re.search(r"linear-gradient\((\d+)deg", body)
        if am:
            ang = float(am.group(1))
        entries.append((name, hexes, ang))
    return entries


def main() -> None:
    text = SRC.read_text(encoding="utf-8")
    entries = parse_entries(text)
    OUT.mkdir(parents=True, exist_ok=True)

    written = 0
    skipped = 0
    used_ids: set[str] = set()

    for name, hexes, angle in entries:
        base = slugify(name)
        preset_id = f"cl-{base}"
        # avoid collisions
        if preset_id in used_ids:
            n = 2
            while f"{preset_id}-{n}" in used_ids:
                n += 1
            preset_id = f"{preset_id}-{n}"
        used_ids.add(preset_id)

        rgba = [hex_to_rgba(h) for h in hexes]
        a, b, c, d = to_four_stops(rgba)

        preset = {
            "id": preset_id,
            "name": name,
            "category": "gradient",
            "shader": "still/SoftLinear",
            "description": f"CSS linear gradient from Coolors: {name}.",
            "thumbnail": f"thumbnails/{preset_id}.png",
            "defaultSettings": {
                "speed": 0.0,
                "intensity": 0.85,
                "angle": angle,
            },
            "colorA": a,
            "colorB": b,
            "colorC": c,
            "colorD": d,
        }

        path = OUT / f"{preset_id}.json"
        path.write_text(json.dumps(preset, indent=2) + "\n", encoding="utf-8")
        written += 1

    print(f"Wrote {written} Coolors presets to {OUT}")
    print(f"Skipped {skipped}")


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""Extract MagicPattern shader-gradient preset states for Wakbg porting."""

from __future__ import annotations

import json
import re
import ssl
import urllib.request
from pathlib import Path

CTX = ssl.create_default_context()
UA = {"User-Agent": "Mozilla/5.0"}
URL = "https://www.magicpattern.design/_next/static/chunks/8586-7601dec2b040f1a6.js"


def fetch(url: str) -> str:
    req = urllib.request.Request(url, headers=UA)
    with urllib.request.urlopen(req, context=CTX, timeout=60) as resp:
        return resp.read().decode("utf-8", "replace")


def extract_object_after(js: str, start: int) -> tuple[str, int]:
    """Given index at '{', return object text and end index."""
    assert js[start] == "{"
    depth = 0
    i = start
    in_str = False
    esc = False
    while i < len(js):
        ch = js[i]
        if in_str:
            if esc:
                esc = False
            elif ch == "\\":
                esc = True
            elif ch == '"':
                in_str = False
        else:
            if ch == '"':
                in_str = True
            elif ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    return js[start : i + 1], i + 1
        i += 1
    raise RuntimeError("unbalanced")


def js_object_to_dict(obj: str) -> dict:
    """Best-effort parse of a JS object literal with our known fields."""
    out: dict = {}

    def num(key: str, default=None):
        m = re.search(rf"{key}:(-?(?:\d+\.\d+|\.\d+|\d+)(?:e[+-]?\d+)?)", obj)
        if m:
            out[key] = float(m.group(1))
        elif default is not None:
            out[key] = default

    def flag(key: str, default=False):
        # !0 => true, !1 => false, true/false
        m = re.search(rf"{key}:(!0|!1|true|false)", obj)
        if not m:
            out[key] = default
            return
        v = m.group(1)
        out[key] = v in ("!0", "true")

    for key in [
        "speed", "zoom", "freq", "sharpness", "amplitude", "waveWidthMod",
        "offsetX", "offsetY", "rotation", "seed",
        "localWarpIntensity", "localWarpFreqX", "localWarpFreqY",
        "ditherLevels", "ditherScale", "grainIntensity", "grainSpeed",
        "vignetteIntensity", "vignetteRadius", "brightness", "contrast", "hue",
        "fractalGlassSteps", "fractalGlassDistortion", "fractalGlassBlur",
    ]:
        num(key)

    for key in [
        "ditherEnabled", "grainEnabled", "vignetteEnabled",
        "fractalGlassEnabled", "animate",
    ]:
        flag(key)

    cols = re.findall(r'"(#[0-9A-Fa-f]{6})"', obj)
    if cols:
        out["colors"] = cols

    wd = re.search(r"warpDirection:\[([^\]]+)\]", obj)
    if wd:
        out["warpDirection"] = [float(x.strip()) for x in wd.group(1).split(",")]
    else:
        out["warpDirection"] = [1.0, -1.0]

    return out


def main() -> None:
    js = fetch(URL)
    presets = []
    for m in re.finditer(r'\{id:"([^"]+)",name:"([^"]+)",dimensions:', js):
        pid, name = m.group(1), m.group(2)
        # find state:{ after dimensions
        state_key = js.find("state:", m.end())
        if state_key < 0 or state_key > m.end() + 200:
            continue
        brace = js.find("{", state_key)
        try:
            obj, _ = extract_object_after(js, brace)
        except Exception:
            continue
        state = js_object_to_dict(obj)
        if len(state.get("colors", [])) < 2:
            continue
        presets.append({"id": pid, "name": name, "state": state})

    # de-dupe by id keeping first
    seen = set()
    unique = []
    for p in presets:
        if p["id"] in seen:
            continue
        seen.add(p["id"])
        unique.append(p)

    out = Path(__file__).resolve().parents[1] / "insiraptions" / "mp_presets.json"
    out.write_text(json.dumps(unique, indent=2), encoding="utf-8")
    print(f"presets={len(unique)} -> {out}")
    for p in unique:
        st = p["state"]
        print(
            f"{p['id']:24s} colors={len(st.get('colors',[]))} "
            f"freq={st.get('freq')} amp={st.get('amplitude')} "
            f"dither={st.get('ditherEnabled')} grain={st.get('grainEnabled')}"
        )


if __name__ == "__main__":
    main()

#include "../common/Common.hlsli"
#include "../common/Noise.hlsli"

// Reference-driven space look:
// - domain-warped nebula gas
// - ridged dust lanes
// - dense hashed starfield with core + bloom (3x3 neighbourhood)
// - optional soft spiral disk (WaveWidthMod)
// Slow parallax drift keeps it alive without looking like a screensaver.

PSInput VSMain(VSInput input)
{
    PSInput output;
    output.Position = float4(input.Position, 1.0);
    output.UV = input.UV;
    return output;
}

float2 Hash22(float2 p)
{
    float n = Hash21(p);
    float n2 = Hash21(p + float2(19.1, 7.3));
    return float2(n, n2);
}

float2 Rotate2(float2 p, float a)
{
    float s = sin(a);
    float c = cos(a);
    return float2(c * p.x - s * p.y, s * p.x + c * p.y);
}

float Fbm4(float2 p)
{
    float value = 0.0;
    float amplitude = 0.5;
    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        value += amplitude * Noise2D(p);
        p *= 2.02;
        amplitude *= 0.5;
    }
    return value;
}

// Soft star with core + halo. Samples 3x3 cells so glow isn't clipped.
float StarLayer(float2 uv, float cellSize, float t, float seed, float presence)
{
    float2 g = uv * float2(ResolutionX, ResolutionY) / max(cellSize, 1.0);
    float2 id = floor(g);
    float2 f = frac(g);
    float acc = 0.0;

    [unroll]
    for (int j = -1; j <= 1; ++j)
    {
        [unroll]
        for (int i = -1; i <= 1; ++i)
        {
            float2 off = float2(i, j);
            float2 cid = id + off;
            float2 rnd = Hash22(cid + seed);
            float gate = Hash21(cid * 2.13 + seed);
            if (gate > presence)
            {
                continue;
            }

            float2 pos = off + 0.2 + rnd * 0.6;
            float2 d = (f - pos);
            // Keep aspect roughly circular in pixel space.
            d.x *= ResolutionX / max(ResolutionY, 1.0);
            float dist = length(d);

            float size = lerp(0.012, 0.055, Hash21(cid + 19.0)) * lerp(0.7, 1.35, Amplitude);
            float tw = 0.55 + 0.45 * sin(t * (0.7 + rnd.y * 2.2) + rnd.x * 6.2831);
            float bright = 0.35 + 0.65 * rnd.x;

            float core = smoothstep(size, 0.0, dist);
            float halo = smoothstep(size * 4.5, 0.0, dist) * 0.22;
            // Occasional brighter “lens” sparkles.
            float spike = 0.0;
            if (rnd.y > 0.92)
            {
                float ax = abs(d.x);
                float ay = abs(d.y);
                spike = exp(-ax * 55.0) * exp(-ay * 8.0) * 0.35;
                spike += exp(-ay * 55.0) * exp(-ax * 8.0) * 0.35;
            }

            acc += (core + halo + spike) * bright * tw;
        }
    }
    return acc;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float t = Time * Speed;
    float2 uv = AspectCorrect(input.UV);
    float2 centered = uv - 0.5 - float2(OffsetX, OffsetY) * 0.25;
    centered *= Zoom;

    // Parallax: gas drifts slower than fine dust / stars feel.
    float bill = t * 0.035;
    float2 p = centered + float2(bill * 0.15, -bill * 0.05);

    // ---- Nebula gas (double domain warp) ----
    float gscale = lerp(1.1, 2.6, saturate(Freq * 0.12));
    float2 wp = p * gscale;
    float chaos = 0.7 + LocalWarpIntensity * 0.9;
    float2 q = float2(Fbm4(wp + float2(0.0, bill)), Fbm4(wp + float2(5.2, -bill)));
    float2 r = float2(Fbm4(wp + q * chaos + float2(1.7, 9.2)), Fbm4(wp + q * chaos + float2(8.3, 2.8) - bill));
    float gas = Fbm4(wp + r * (1.1 + chaos * 0.4));
    gas = pow(saturate(gas), lerp(1.55, 0.9, saturate(Sharpness * 0.18)));
    float clump = Fbm4(p * 0.75 + 9.0 + bill * 0.2);

    // Ridged dust lanes (dark channels through the gas)
    float nRidge = Fbm4(p * 2.1 + q * 0.5);
    float ridge = 1.0 - abs(2.0 * nRidge - 1.0);
    float dust = smoothstep(0.72, 0.97, ridge) * saturate(0.35 + LocalWarpIntensity * 0.4);

    // Soft spiral disk option (Andromeda-like), blended by WaveWidthMod.
    float spiralAmt = saturate(WaveWidthMod);
    float2 sp = Rotate2(centered, length(centered) * 2.8 - t * 0.04);
    float arms = 0.5 + 0.5 * sin(atan2(sp.y, sp.x) * 2.0 + length(sp) * 7.0);
    float disk = exp(-length(centered) * 2.4);
    float spiral = smoothstep(0.25, 0.9, arms) * disk;
    spiral *= 0.55 + 0.45 * Fbm4(sp * 3.0 + bill);

    float gasBody = gas * (0.35 + 0.65 * clump);
    gasBody = lerp(gasBody, saturate(gasBody * 0.55 + spiral * 1.1), spiralAmt);
    gasBody *= (1.0 - dust * 0.75);

    // Milky-Way style diagonal band when OffsetY pushes composition.
    float band = saturate(1.0 - abs(dot(centered, normalize(float2(0.85, 0.45))) * 2.2));
    band = pow(band, 1.6) * Fbm4(centered * 2.5 + bill);
    gasBody = saturate(gasBody + band * 0.35 * saturate(1.0 - spiralAmt) * saturate(Freq * 0.08));

    // Colour: deep space -> cool dust -> warm emission core
    float3 color = ColorA.rgb;
    color = lerp(color, ColorB.rgb, saturate(gasBody * 1.1));
    color = lerp(color, ColorC.rgb, saturate(gas * gasBody));
    color += ColorD.rgb * pow(saturate(gasBody), 2.2) * 0.55 * Amplitude;

    // Bright galactic core
    float core = exp(-length(centered) * lerp(5.0, 10.0, saturate(Sharpness * 0.15))) * Amplitude;
    color += lerp(ColorC.rgb, ColorD.rgb, 0.55) * core * (0.45 + spiralAmt * 0.55);

    // Soft tone map so blooms don't clip harshly
    color = color / (color + 0.75) * 1.15;

    // ---- Starfield layers (parallax) ----
    float presence = lerp(0.78, 0.93, saturate(1.0 - Intensity * 0.35));
    float stars = 0.0;
    stars += StarLayer(uv + float2(t * 0.002, 0.0), 28.0, t, 1.1, presence) * 0.55;
    stars += StarLayer(uv - float2(0.0, t * 0.0015), 18.0, t, 4.7, presence + 0.04) * 0.85;
    stars += StarLayer(uv + float2(-t * 0.0008, t * 0.0006), 11.0, t, 9.3, presence + 0.08) * 1.1;

    // Dim stars inside thick gas a bit; keep sparkle in open space.
    float starMix = lerp(1.0, 0.35, saturate(gasBody * 1.2));
    color += stars * starMix * lerp(float3(0.85, 0.9, 1.0), ColorD.rgb, 0.25);

    return float4(ApplyBrightness(saturate(color)), 1.0);
}

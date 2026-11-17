#include "../common/Common.hlsli"

// Faithful port of MagicPattern's shader-gradient fragment pipeline.

PSInput VSMain(VSInput input)
{
    PSInput output;
    output.Position = float4(input.Position, 1.0);
    output.UV = input.UV;
    return output;
}

float FastTanh(float x)
{
    return clamp(x * (27.0 + x * x) / (27.0 + 9.0 * x * x), -1.0, 1.0);
}

float Hash12(float2 p)
{
    float3 p3 = frac(float3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return frac((p3.x + p3.y) * p3.z);
}

float ModulatedSquareWave(float x, float freq, float sharpness, float widthMod)
{
    float m = 1.0 + sin(x * freq * 0.5) * widthMod;
    m = clamp(m, 0.3, 2.0);
    return FastTanh(sin(x * freq * m) * sharpness);
}

float2 LocalWarp(float2 uv, float intensity, float freqX, float freqY)
{
    float warpX = (freqX > 0.0) ? sin(uv.y * freqX) * intensity * 0.5 : 0.0;
    float warpY = (freqY > 0.0) ? sin(uv.x * freqY) * intensity * 0.4 : 0.0;
    uv.x += warpX;
    uv.y += warpY;
    return uv;
}

float3 SamplePalette(float t)
{
    t = saturate(t);
    if (t < 0.333333)
    {
        return lerp(ColorA.rgb, ColorB.rgb, saturate(t / 0.333333));
    }
    if (t < 0.666667)
    {
        return lerp(ColorB.rgb, ColorC.rgb, saturate((t - 0.333333) / 0.333333));
    }
    return lerp(ColorC.rgb, ColorD.rgb, saturate((t - 0.666667) / 0.333333));
}

float3 DitherColor(float3 color, float2 coord, float levels, float scale)
{
    float2 dc = floor(coord / max(scale, 1.0));
    float t = Bayer4(dc);
    return floor(color * levels + t) / max(levels, 1.0);
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float2 fragCoord = input.UV * float2(ResolutionX, ResolutionY);

    float freq = max(Freq, 0.1);
    float sharpness = max(Sharpness, 0.1);
    float amplitude = max(Amplitude, 0.01);
    float widthMod = WaveWidthMod;
    float zoom = max(Zoom, 0.001);
    float warpIntensity = LocalWarpIntensity;
    float warpFreqY = max(LocalWarpFreqY, 0.01);

    bool ditherOn = DitherLevels > 0.5;
    float ditherScale = max(DitherScale, 1.0);
    float scale = ditherOn ? ditherScale : 1.0;

    float2 dbc = floor(fragCoord / scale) * scale + scale * 0.5;
    float2 blockUV = dbc / float2(ResolutionX, ResolutionY);
    blockUV = blockUV * 2.0 - 1.0;
    blockUV.x *= ResolutionX / max(ResolutionY, 1.0);
    blockUV /= zoom;

    float rotRad = radians(Pad0);
    float2x2 rotM = float2x2(cos(rotRad), -sin(rotRad), sin(rotRad), cos(rotRad));
    blockUV = mul(rotM, blockUV);

    float t = Time * Speed;
    blockUV.x += OffsetX + sin(t * 0.15) * 0.5 + cos(t * 0.08) * 0.3;
    blockUV.y += OffsetY;

    float animWI = warpIntensity + 0.12 * sin(t * 0.9);
    float animWW = widthMod + 0.25 * sin(t * 0.3 + 3.14159265);

    float angle = atan2(-1.0, 1.0) + sin(t * 0.1) * 1.57;
    float2 dir = float2(cos(angle), sin(angle));

    float diag = dot(blockUV, dir);
    float wave = ModulatedSquareWave(diag, freq, sharpness, animWW);
    float animAmp = amplitude * (1.5 + 0.4 * sin(t * 0.13));
    float2 warpedUV = blockUV + dir * wave * animAmp;
    warpedUV = LocalWarp(warpedUV, animWI, 1.0, warpFreqY);

    float gradientT = clamp(warpedUV.y * -0.33 + 0.5, 0.0, 1.0);
    float3 color = SamplePalette(gradientT);

    float contrast = max(Contrast, 0.05);
    color = (color - 0.5) * contrast + 0.5;
    color *= Brightness;
    color = saturate(color);

    if (GrainIntensity > 0.0001)
    {
        float grainSeed = frac(Time * max(GrainSpeed, 0.001) * 0.02);
        float grain = Hash12(dbc + grainSeed) * 2.0 - 1.0;
        color += grain * GrainIntensity;
    }

    if (VignetteIntensity > 0.0001)
    {
        float radius = max(VignetteRadius, 0.01);
        float2 normUV = dbc / float2(ResolutionX, ResolutionY);
        float2 feather = smoothstep(0.0, radius, normUV) * smoothstep(0.0, radius, 1.0 - normUV);
        float edgeMask = feather.x * feather.y;
        color = lerp(color, color * edgeMask, saturate(VignetteIntensity));
    }

    if (ditherOn)
    {
        color = DitherColor(color, fragCoord, DitherLevels, ditherScale);
    }

    return float4(saturate(color), 1.0);
}

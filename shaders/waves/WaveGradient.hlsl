#include "../common/Common.hlsli"

// Lightweight wave field — no FBM, keeps 60+ FPS at desktop resolution.
PSInput VSMain(VSInput input)
{
    PSInput output;
    output.Position = float4(input.Position, 1.0);
    output.UV = input.UV;
    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float t = Time * Speed;
    float2 uv = AspectCorrect(input.UV);
    float2 p = uv + float2(OffsetX, OffsetY) * 0.2;

    float freq = lerp(2.5, 9.0, saturate(Freq * 0.12));
    float amp = Intensity * Amplitude;

    float w1 = sin((p.x * freq + t) + sin(p.y * (freq * 0.5) + t * 0.7) * amp);
    float w2 = sin((p.y * (freq * 0.85) - t * 0.8) + cos(p.x * (freq * 0.7) + t * 0.5) * amp);
    float w3 = sin((p.x + p.y) * (freq * 0.65) + t * 1.2 + WaveWidthMod);

    float m1 = saturate(w1 * 0.5 + 0.5);
    float m2 = saturate(w2 * 0.5 + 0.5);
    float m3 = saturate(w3 * 0.5 + 0.5);
    m1 = pow(m1, lerp(1.4, 0.7, saturate(Sharpness * 0.2)));

    float3 color = lerp(ColorA.rgb, ColorB.rgb, m1);
    color = lerp(color, ColorC.rgb, m2 * 0.65);
    color = lerp(color, ColorD.rgb, m3 * 0.35);
    return float4(ApplyBrightness(saturate(color)), 1.0);
}

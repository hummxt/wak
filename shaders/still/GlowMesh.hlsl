#include "../common/Common.hlsli"

PSInput VSMain(VSInput input)
{
    PSInput output;
    output.Position = float4(input.Position, 1.0);
    output.UV = input.UV;
    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float t = Time * Speed * 0.18;
    float2 uv = AspectCorrect(input.UV);
    float drift = Speed > 0.001 ? 1.0 : 0.0;

    float2 c0 = float2(0.42 + 0.10 * sin(t * 0.55) * drift, 0.48 + 0.08 * cos(t * 0.40) * drift);
    float2 c1 = float2(0.68 + 0.08 * cos(t * 0.48) * drift, 0.36 + 0.10 * sin(t * 0.62) * drift);
    float2 c2 = float2(0.30 + 0.09 * sin(t * 0.35) * drift, 0.70 + 0.07 * cos(t * 0.70) * drift);

    float r0 = distance(uv, c0);
    float r1 = distance(uv, c1);
    float r2 = distance(uv, c2);

    float glow = exp(-r0 * lerp(3.6, 2.0, saturate(Intensity)));
    float side = exp(-r1 * 2.8);
    float rim = exp(-r2 * 3.1);

    float3 base = ColorD.rgb;
    float3 color = base;
    color = lerp(color, ColorA.rgb, saturate(glow));
    color = lerp(color, ColorB.rgb, saturate(side * 0.9));
    color = lerp(color, ColorC.rgb, saturate(rim * 0.75));

    float edge = smoothstep(0.2, 1.05, length(uv - 0.5));
    color = lerp(color, base * 0.4, edge * 0.7);
    color = ApplyBrightness(color);
    color = Dither8Bit(input.UV, color);
    return float4(color, 1.0);
}

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
    float t = Time * Speed;
    float2 uv = AspectCorrect(input.UV);

    float w1 = sin((uv.x * 6.0 + t) + sin(uv.y * 3.0 + t * 0.7) * Intensity);
    float w2 = sin((uv.y * 5.0 - t * 0.8) + cos(uv.x * 4.0 + t * 0.5) * Intensity);
    float w3 = sin((uv.x + uv.y) * 4.0 + t * 1.2);

    float m1 = w1 * 0.5 + 0.5;
    float m2 = w2 * 0.5 + 0.5;
    float m3 = w3 * 0.5 + 0.5;

    float3 color = lerp(ColorA.rgb, ColorB.rgb, m1);
    color = lerp(color, ColorC.rgb, m2 * 0.65);
    color = lerp(color, ColorD.rgb, m3 * 0.35);
    return float4(ApplyBrightness(color), 1.0);
}

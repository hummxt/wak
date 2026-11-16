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
    float t = Time * Speed * 1.35;
    float2 uv = AspectCorrect(input.UV);

    float2 p0 = float2(0.22 + 0.28 * sin(t * 0.85), 0.28 + 0.24 * cos(t * 0.65));
    float2 p1 = float2(0.78 + 0.24 * cos(t * 0.75), 0.24 + 0.28 * sin(t * 0.95));
    float2 p2 = float2(0.28 + 0.30 * sin(t * 0.55), 0.78 + 0.22 * cos(t * 1.05));
    float2 p3 = float2(0.72 + 0.26 * cos(t * 0.70), 0.74 + 0.24 * sin(t * 0.80));

    float d0 = distance(uv, p0);
    float d1 = distance(uv, p1);
    float d2 = distance(uv, p2);
    float d3 = distance(uv, p3);

    float w0 = 1.0 / max(pow(d0, 1.55 / max(Intensity, 0.1)), 0.001);
    float w1 = 1.0 / max(pow(d1, 1.55 / max(Intensity, 0.1)), 0.001);
    float w2 = 1.0 / max(pow(d2, 1.55 / max(Intensity, 0.1)), 0.001);
    float w3 = 1.0 / max(pow(d3, 1.55 / max(Intensity, 0.1)), 0.001);
    float sum = w0 + w1 + w2 + w3;

    float3 color = (ColorA.rgb * w0 + ColorB.rgb * w1 + ColorC.rgb * w2 + ColorD.rgb * w3) / sum;
    float pulse = 0.04 * sin(t * 1.8 + uv.x * 4.0);
    color = saturate(color + pulse);
    return float4(ApplyBrightness(color), 1.0);
}

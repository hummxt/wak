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
    float t = Time * Speed * 0.1;
    float2 uv = AspectCorrect(input.UV);

    float angle = t * 0.25;
    float2 dir = float2(cos(angle), sin(angle));
    float g = saturate(dot(uv - 0.5, dir) * 0.5 + 0.5);
    g = smoothstep(0.0, 1.0, g);

    float pulse = 0.5 + 0.5 * sin(t);
    float3 color = lerp(ColorA.rgb, ColorB.rgb, g);
    color = lerp(color, ColorC.rgb, pulse * 0.15 * Intensity);
    return float4(ApplyBrightness(color), 1.0);
}

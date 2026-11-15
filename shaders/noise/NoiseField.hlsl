#include "../common/Common.hlsli"
#include "../common/Noise.hlsli"

PSInput VSMain(VSInput input)
{
    PSInput output;
    output.Position = float4(input.Position, 1.0);
    output.UV = input.UV;
    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float t = Time * Speed * 0.2;
    float2 uv = AspectCorrect(input.UV);
    float2 p = DomainWarp(uv * 4.0, t);

    float n = FBM(p + t);
    float n2 = FBM(p * 1.8 - t * 0.5);
    float blend = saturate(n * 0.65 + n2 * 0.35);

    float3 color = lerp(ColorA.rgb, ColorB.rgb, blend);
    color = lerp(color, ColorC.rgb, pow(n2, 2.0) * Intensity);
    color = lerp(color, ColorD.rgb, saturate(1.0 - length(uv - 0.5) * 1.4) * 0.25);
    return float4(ApplyBrightness(color), 1.0);
}

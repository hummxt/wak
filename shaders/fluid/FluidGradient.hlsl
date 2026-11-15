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
    float t = Time * Speed * 0.25;
    float2 uv = AspectCorrect(input.UV);
    float2 p = DomainWarp(uv * 2.5, t);

    float n1 = FBM(p + t);
    float n2 = FBM(p * 1.7 - t * 0.7);
    float mixValue = saturate(0.5 + 0.5 * sin((n1 + n2) * 3.14159));

    float3 c0 = lerp(ColorA.rgb, ColorB.rgb, n1);
    float3 c1 = lerp(ColorC.rgb, ColorD.rgb, n2);
    float3 color = lerp(c0, c1, mixValue);
    return float4(ApplyBrightness(color), 1.0);
}

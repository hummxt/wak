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
    float t = Time * Speed * 0.15;
    float2 uv = AspectCorrect(input.UV);
    float2 p = DomainWarp(uv * 3.0, t);

    float blob = FBM(p);
    float rim = smoothstep(0.35, 0.55, blob) - smoothstep(0.55, 0.75, blob);
    float body = smoothstep(0.25, 0.6, blob);

    float3 color = lerp(ColorA.rgb, ColorB.rgb, body);
    color = lerp(color, ColorC.rgb, rim * Intensity);
    color = lerp(color, ColorD.rgb, saturate(blob * blob));
    return float4(ApplyBrightness(color), 1.0);
}

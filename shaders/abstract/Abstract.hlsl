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
    float t = Time * Speed;
    float2 uv = AspectCorrect(input.UV) - 0.5;
    float r = length(uv);
    float a = atan2(uv.y, uv.x);

    float rings = sin(r * 18.0 - t * 1.5 + FBM(uv * 4.0 + t * 0.2) * 2.0 * Intensity);
    float spokes = sin(a * 6.0 + t * 0.8);
    float mask = saturate(rings * 0.5 + 0.5) * saturate(spokes * 0.5 + 0.5);

    float3 color = lerp(ColorA.rgb, ColorB.rgb, mask);
    color = lerp(color, ColorC.rgb, saturate(1.0 - r * 1.5));
    color += ColorD.rgb * pow(saturate(1.0 - r), 3.0) * 0.4;
    return float4(ApplyBrightness(color), 1.0);
}

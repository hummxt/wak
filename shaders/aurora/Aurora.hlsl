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
    float2 p = uv * float2(2.0, 1.2);

    float band = 0.0;
    band += 0.55 * sin(p.x * 2.0 + FBM(p + t) * 2.5 + t);
    band += 0.35 * sin(p.x * 3.5 - t * 1.3 + FBM(p * 1.5 - t) * 2.0);
    band = saturate(band * 0.5 + 0.5);
    band = pow(band, lerp(1.8, 0.8, Intensity));

    float vertical = smoothstep(0.15, 0.55, uv.y) * (1.0 - smoothstep(0.55, 0.95, uv.y));
    float glow = band * vertical;

    float3 color = lerp(ColorA.rgb, ColorB.rgb, glow);
    color = lerp(color, ColorC.rgb, glow * glow);
    color += ColorD.rgb * glow * 0.35;
    color = lerp(ColorA.rgb * 0.15, color, saturate(glow + 0.2));
    return float4(ApplyBrightness(color), 1.0);
}

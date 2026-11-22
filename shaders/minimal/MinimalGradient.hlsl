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
    float t = Time * Speed * 0.08;
    float2 uv = AspectCorrect(input.UV);

    float angle = radians(Pad0) + t * 0.15;
    float2 dir = float2(cos(angle), sin(angle));
    float g = saturate(dot(uv - 0.5 - float2(OffsetX, OffsetY) * 0.2, dir) * Zoom + 0.5);
    g = smoothstep(0.0, 1.0, g);
    g = pow(g, lerp(1.2, 0.8, saturate(Sharpness * 0.15)));

    float soft = FBM(uv * 1.5 + t * 0.1) * 0.08 * Intensity;
    float3 color = lerp(ColorA.rgb, ColorB.rgb, g);
    color = lerp(color, ColorC.rgb, soft + Amplitude * 0.08 * sin(t + uv.x));
    color = lerp(color, ColorD.rgb, saturate(1.0 - length(uv - 0.5) * 1.5) * 0.12);
    return float4(ApplyBrightness(saturate(color)), 1.0);
}

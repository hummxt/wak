#include "../common/Common.hlsli"
#include "../common/Noise.hlsli"

PSInput VSMain(VSInput input)
{
    PSInput output;
    output.Position = float4(input.Position, 1.0);
    output.UV = input.UV;
    return output;
}

float3 FilmGrain(float2 uv, float3 color)
{
    float grain = Hash21(uv * float2(ResolutionX, ResolutionY) * 0.55) - 0.5;
    return saturate(color + grain * 0.018);
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float2 uv = input.UV;
    float t = Time * Speed * 0.08;
    float angle = radians(Pad0);
    float2 dir = float2(sin(angle), -cos(angle));
    float drift = Speed > 0.001 ? 0.02 * sin(t + uv.y * 1.5) : 0.0;
    float x = saturate(dot(uv - 0.5, dir) + 0.5 + drift);

    float soft = lerp(0.01, 0.08, saturate(Intensity));
    float3 color;
    if (x < 0.333)
    {
        float u = smoothstep(0.0, 0.333 + soft, x);
        color = lerp(ColorA.rgb, ColorB.rgb, u);
    }
    else if (x < 0.666)
    {
        float u = smoothstep(0.333 - soft, 0.666 + soft, x);
        color = lerp(ColorB.rgb, ColorC.rgb, u);
    }
    else
    {
        float u = smoothstep(0.666 - soft, 1.0, x);
        color = lerp(ColorC.rgb, ColorD.rgb, u);
    }

    color = FilmGrain(uv, color);
    return float4(ApplyBrightness(color), 1.0);
}

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
    // OffsetX = time-of-day bias (0..1). Speed drives the slow cycle.
    float cycle = frac(Time * Speed * 0.02 + OffsetX);
    float2 uv = AspectCorrect(input.UV);

    float day = smoothstep(0.18, 0.32, cycle) * (1.0 - smoothstep(0.68, 0.82, cycle));
    float dusk = saturate(1.0 - abs(cycle - 0.25) * 6.0) + saturate(1.0 - abs(cycle - 0.75) * 6.0);
    dusk = saturate(dusk);

    float3 night = lerp(ColorA.rgb, ColorB.rgb * 0.35, uv.y);
    float3 dayCol = lerp(ColorC.rgb, ColorD.rgb, uv.y);
    float3 duskCol = lerp(ColorB.rgb, ColorC.rgb, uv.y);

    float3 color = lerp(night, dayCol, day);
    color = lerp(color, duskCol, dusk * Intensity);

    // Soft sun / moon disc.
    float sunY = lerp(1.15, -0.15, cycle);
    float2 sunPos = float2(0.72, sunY);
    float sun = smoothstep(0.12, 0.0, length(uv - sunPos));
    color += lerp(ColorD.rgb, ColorC.rgb, day) * sun * 0.55 * Amplitude;

    float haze = FBM(uv * 2.0 + cycle * 2.0) * 0.12 * Intensity;
    color += haze;
    return float4(ApplyBrightness(saturate(color)), 1.0);
}

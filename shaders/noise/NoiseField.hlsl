#include "../common/Common.hlsli"
#include "../common/Noise.hlsli"

// Soft grainy gradients — film-grain / premium UI noise look.
PSInput VSMain(VSInput input)
{
    PSInput output;
    output.Position = float4(input.Position, 1.0);
    output.UV = input.UV;
    return output;
}

float FilmGrain(float2 uv, float t)
{
    float2 pixel = uv * float2(ResolutionX, ResolutionY);
    // Fine, dense grain (not chunky FBM blobs).
    float g = Hash21(floor(pixel * lerp(0.85, 1.35, saturate(Freq * 0.08))) + floor(t * GrainSpeed * 0.02));
    float g2 = Hash21(floor(pixel * 0.55) + 19.7);
    return (g * 0.7 + g2 * 0.3) * 2.0 - 1.0;
}

float SoftField(float2 uv, float t)
{
    float angle = radians(Pad0);
    float2 dir = float2(cos(angle), sin(angle));
    float beam = dot(uv - 0.5 - float2(OffsetX, OffsetY) * 0.35, dir);
    beam = beam * Zoom + 0.5;

    float soft = FBM(uv * 1.6 + t * 0.05);
    float blob = FBM(uv * 2.4 - t * 0.04 + 3.1);
    float field = saturate(lerp(beam, soft, 0.35) + (blob - 0.5) * 0.25 * Intensity);
    field = smoothstep(0.05, 0.95, field);
    field = pow(field, lerp(1.35, 0.75, saturate(Sharpness * 0.2)));
    return field;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float t = Time * Speed;
    float2 uv = AspectCorrect(input.UV);

    float field = SoftField(uv, t);
    float3 color = lerp(ColorA.rgb, ColorB.rgb, field);
    color = lerp(color, ColorC.rgb, saturate(field * field) * Amplitude * 0.65);
    color = lerp(color, ColorD.rgb, saturate(1.0 - length(uv - 0.5) * 1.2) * 0.18);

    float grainAmt = (GrainIntensity > 0.001) ? GrainIntensity : 0.085;
    color += FilmGrain(input.UV, t) * grainAmt * lerp(0.6, 1.2, Intensity);

    return float4(ApplyBrightness(saturate(color)), 1.0);
}

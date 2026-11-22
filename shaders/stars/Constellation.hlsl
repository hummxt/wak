#include "../common/Common.hlsli"
#include "../common/Noise.hlsli"

PSInput VSMain(VSInput input)
{
    PSInput output;
    output.Position = float4(input.Position, 1.0);
    output.UV = input.UV;
    return output;
}

float StarField(float2 uv, float density, float t)
{
    float2 grid = uv * density;
    float2 cell = floor(grid);
    float2 f = frac(grid);
    float h = Hash21(cell);
    float2 center = float2(Hash21(cell + 17.1), Hash21(cell + 91.7));
    float dist = length(f - center);
    float twinkle = 0.65 + 0.35 * sin(t * (2.0 + h * 4.0) + h * 6.28);
    float radius = lerp(0.018, 0.045, h) * Amplitude;
    float star = smoothstep(radius, 0.0, dist) * twinkle;
    star *= step(1.0 - density * 0.04 - Intensity * 0.08, h);
    return star;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float t = Time * Speed;
    float2 uv = AspectCorrect(input.UV);

    float nebula = FBM(uv * 2.5 + t * 0.05);
    float3 bg = lerp(ColorA.rgb, ColorB.rgb, nebula);
    bg = lerp(bg, ColorC.rgb, pow(nebula, 2.0) * 0.35);

    float density = lerp(18.0, 42.0, saturate(Freq * 0.12));
    float stars = StarField(uv, density, t);
    stars += StarField(uv + 0.37, density * 0.6, t * 0.7) * 0.65;

    float3 color = bg + ColorD.rgb * stars;
    float vignette = smoothstep(1.1, 0.25, length(uv - 0.5));
    color *= lerp(0.75, 1.0, vignette);
    return float4(ApplyBrightness(color), 1.0);
}

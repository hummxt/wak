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
    float grain = Hash21(uv * float2(ResolutionX, ResolutionY) * 0.65) - 0.5;
    return saturate(color + grain * 0.035);
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float t = Time * Speed * 0.22;
    float2 uv = AspectCorrect(input.UV);
    float angle = radians(Pad0);
    float2 dir = float2(sin(angle), -cos(angle));
    float2 centered = uv - 0.5;
    float2 rotated = float2(dot(centered, dir), dot(centered, float2(-dir.y, dir.x))) + 0.5;

    float soft = lerp(1.85, 1.15, saturate(Intensity));
    float drift = Speed > 0.001 ? 1.0 : 0.0;

    float2 p0 = float2(0.18 + 0.16 * sin(t * 0.70) * drift, 0.22 + 0.14 * cos(t * 0.55) * drift);
    float2 p1 = float2(0.82 + 0.14 * cos(t * 0.60) * drift, 0.20 + 0.16 * sin(t * 0.80) * drift);
    float2 p2 = float2(0.24 + 0.18 * sin(t * 0.45) * drift, 0.78 + 0.12 * cos(t * 0.90) * drift);
    float2 p3 = float2(0.76 + 0.15 * cos(t * 0.50) * drift, 0.74 + 0.14 * sin(t * 0.65) * drift);

    float d0 = distance(rotated, p0);
    float d1 = distance(rotated, p1);
    float d2 = distance(rotated, p2);
    float d3 = distance(rotated, p3);

    float w0 = 1.0 / max(pow(d0, soft), 0.0008);
    float w1 = 1.0 / max(pow(d1, soft), 0.0008);
    float w2 = 1.0 / max(pow(d2, soft), 0.0008);
    float w3 = 1.0 / max(pow(d3, soft), 0.0008);
    float sum = w0 + w1 + w2 + w3;

    float3 color = (ColorA.rgb * w0 + ColorB.rgb * w1 + ColorC.rgb * w2 + ColorD.rgb * w3) / sum;
    float lift = 0.03 * Noise2D(uv * 3.0 + t * drift);
    color = saturate(color + lift);
    color = FilmGrain(input.UV, color);
    return float4(ApplyBrightness(color), 1.0);
}

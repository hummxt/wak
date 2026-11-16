#include "../common/Common.hlsli"

PSInput VSMain(VSInput input)
{
    PSInput output;
    output.Position = float4(input.Position, 1.0);
    output.UV = input.UV;
    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float2 uv = input.UV;
    float t = Time * Speed * 0.08;
    float angle = radians(Pad0);
    float2 dir = float2(sin(angle), -cos(angle));
    float drift = Speed > 0.001 ? 0.01 * sin(t + uv.y * 2.0) : 0.0;
    float x = saturate(dot(uv - 0.5, dir) + 0.5 + drift);

    float3 color;
    if (x < 0.333333)
    {
        color = lerp(ColorA.rgb, ColorB.rgb, saturate(x / 0.333333));
    }
    else if (x < 0.666667)
    {
        color = lerp(ColorB.rgb, ColorC.rgb, saturate((x - 0.333333) / 0.333333));
    }
    else
    {
        color = lerp(ColorC.rgb, ColorD.rgb, saturate((x - 0.666667) / 0.333333));
    }

    color = ApplyBrightness(color);
    color = Dither8Bit(uv, color);
    return float4(color, 1.0);
}

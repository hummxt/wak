Texture2D SourceTexture : register(t0);
SamplerState LinearSampler : register(s0);

cbuffer Constants : register(b0)
{
    float Time;
    float Speed;
    float Intensity;
    float Brightness;
    float ResolutionX;
    float ResolutionY;
    float Pad0;
    float Pad1;
    float4 ColorA;
    float4 ColorB;
    float4 ColorC;
    float4 ColorD;
};

struct VSInput
{
    float3 Position : POSITION;
    float2 UV : TEXCOORD0;
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD0;
};

PSInput VSMain(VSInput input)
{
    PSInput output;
    output.Position = float4(input.Position, 1.0);
    output.UV = input.UV;
    return output;
}

float Bayer4(float2 pixel)
{
    int2 p = int2(pixel) & 3;
    const float bayer[16] = {
        0.0 / 16.0,  8.0 / 16.0,  2.0 / 16.0, 10.0 / 16.0,
        12.0 / 16.0, 4.0 / 16.0, 14.0 / 16.0,  6.0 / 16.0,
        3.0 / 16.0, 11.0 / 16.0,  1.0 / 16.0,  9.0 / 16.0,
        15.0 / 16.0, 7.0 / 16.0, 13.0 / 16.0,  5.0 / 16.0
    };
    return bayer[p.y * 4 + p.x];
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float3 color = SourceTexture.Sample(LinearSampler, input.UV).rgb;
    float2 pixel = input.UV * float2(ResolutionX, ResolutionY);
    float dither = (Bayer4(pixel) - 0.5) / 255.0;
    return float4(saturate(color + dither), 1.0);
}

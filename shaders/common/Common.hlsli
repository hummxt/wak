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

float2 AspectCorrect(float2 uv)
{
    float aspect = ResolutionX / max(ResolutionY, 1.0);
    uv.x = (uv.x - 0.5) * aspect + 0.5;
    return uv;
}

float3 ApplyBrightness(float3 color)
{
    return saturate(color * Brightness);
}

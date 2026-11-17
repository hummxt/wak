cbuffer Constants : register(b0)
{
    float Time;
    float Speed;
    float Intensity;
    float Brightness;
    float ResolutionX;
    float ResolutionY;
    float Pad0; // rotation / angle (degrees)
    float Zoom;
    float4 ColorA;
    float4 ColorB;
    float4 ColorC;
    float4 ColorD;
    float Freq;
    float Sharpness;
    float Amplitude;
    float WaveWidthMod;
    float OffsetX;
    float OffsetY;
    float LocalWarpIntensity;
    float LocalWarpFreqY;
    float DitherLevels;      // <= 0 disables
    float DitherScale;
    float GrainIntensity;    // <= 0 disables
    float GrainSpeed;
    float Contrast;
    float VignetteIntensity; // <= 0 disables
    float VignetteRadius;
    float PadExtra;
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

float3 Dither8Bit(float2 uv, float3 color)
{
    float2 pixel = uv * float2(ResolutionX, ResolutionY);
    float dither = (Bayer4(pixel) - 0.5) / 255.0;
    return saturate(color + dither);
}

float Hash21(float2 p)
{
    p = frac(p * float2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return frac(p.x * p.y);
}

float Noise2D(float2 p)
{
    float2 i = floor(p);
    float2 f = frac(p);
    float a = Hash21(i);
    float b = Hash21(i + float2(1.0, 0.0));
    float c = Hash21(i + float2(0.0, 1.0));
    float d = Hash21(i + float2(1.0, 1.0));
    float2 u = f * f * (3.0 - 2.0 * f);
    return lerp(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

float FBM(float2 p)
{
    float value = 0.0;
    float amplitude = 0.5;
    [unroll]
    for (int i = 0; i < 5; ++i)
    {
        value += amplitude * Noise2D(p);
        p *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}

float2 DomainWarp(float2 p, float t)
{
    float2 q = float2(FBM(p + t), FBM(p + float2(5.2, 1.3) + t));
    return p + q * Intensity;
}

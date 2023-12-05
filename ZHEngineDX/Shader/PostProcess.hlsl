float3 ACES_Tonemapping(float3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    float3 encode_color = saturate((x * (a * x + b)) / (x * (c * x + d) + e));
    return encode_color;
}

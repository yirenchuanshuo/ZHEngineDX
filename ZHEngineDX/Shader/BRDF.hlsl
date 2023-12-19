//BRDF
static const float PI = 3.1415926535;


float3 Fresnel_Schlick(float HoV, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - HoV, 5.0);
}

float3 Fresnel_Schlick_Roughness(float HoV, float3 F0,float Roughness)
{
    return F0 + (max(float3(1.0 - Roughness, 1.0 - Roughness, 1.0 - Roughness),F0) - F0) * pow(1.0 - HoV, 5.0);
}

float Distribution_GGX(float3 N, float3 H, float Roughness)
{
    float a = Roughness * Roughness;
    float a2 = a * a;
    float NoH = saturate(dot(N, H));
    float NoH2 = NoH * NoH;

    float nominator = a2;
    float denominator = (NoH2 * (a2 - 1.0) + 1.0);
    denominator = PI * denominator * denominator;

    return nominator / max(denominator, 0.001);
}


float Geometry_Schlick_GGX(float NoV, float Roughness)
{
    
    float r = Roughness + 1.0;
    float k = r * r / 8.0;

    float nominator = NoV;
    float denominator = NoV * (1.0 - k) + k;

    return nominator / max(denominator, 0.001);
}

float Geometry_Smith(float3 N, float3 V, float3 L, float Roughness)
{
    float NoV = saturate(dot(N, V));
    float NoL = saturate(dot(N, L));
    float ggx2 = Geometry_Schlick_GGX(NoV, Roughness);
    float ggx1 = Geometry_Schlick_GGX(NoL, Roughness);

    return ggx1 * ggx2;
}





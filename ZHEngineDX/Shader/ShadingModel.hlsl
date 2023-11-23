#include "Common.hlsl"
#include "BRDF.hlsl"

float3 BRDF(float3 basecolor, float Metalic, float Roughness, float3 Radiance, float3 L, float3 V, float3 N)
{
    float3 F0 = lerp(0.04, basecolor, Metalic);
    
    float3 H = normalize(V + L);
    
    float NoL = saturate(dot(N, L));
    float HoV = saturate(dot(H, V));
    float NoV = saturate(dot(N, V));
    
    
    float NDF = Distribution_GGX(N, H, Roughness);
    float G = Geometry_Smith(N, V, L, Roughness);
    float3 F = Fresnel_Schlick(HoV, F0);
    
    float3 kS = F;
    float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;
    kD *= (1 - Metalic);
    
    float3 nominator = NDF * G * F;
    float denominator = max(4.0 * NoV * NoL, 0.001);
    float3 Specular = nominator / denominator;
    
    float3 Diffuse = kD * basecolor / PI;
    float3 finalcolor = (Diffuse + Specular) * NoL * Radiance;
    
    return finalcolor;

}

float3 Phong(float3 baseColor,float specularPow, float specularStrength,float3 L, float3 N,float3 V)
{
    float3 R = reflect(-L,N);
    //float3 R = normalize(-L + 2 * N * dot(N, L));
    float VoR = saturate(dot(V, R));
    float NoL = saturate(dot(N, L));
    float3 Specular = pow(VoR, specularPow) * specularStrength;
    float3 Diffuse = NoL*baseColor;
    return (Specular + Diffuse);
}
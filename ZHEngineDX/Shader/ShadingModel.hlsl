#include "Common.hlsl"
#include "BRDF.hlsl"

float3 BRDF(float3 basecolor, float Metallic, float Roughness,float AO, float3 Radiance, float3 L, float3 V, float3 N)
{
    
    float3 F0 = lerp(0.04, basecolor, Metallic);
    
    float3 H = normalize(V + L);
    float3 R = reflect(-V, N);
    
    float NoL = saturate(dot(N, L));
    float HoV = saturate(dot(H, V));
    float NoV = saturate(dot(N, V));
    
    //Light
    float3 Lightfinalcolor;
    
    {
        float NDF = Distribution_GGX(N, H, Roughness);
        float G = Geometry_Smith(N, V, L, Roughness);
        float3 F = Fresnel_Schlick(HoV, F0);
    
        float3 kS = F;
        float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;
        kD *= (1 - Metallic);
    
        float3 nominator = NDF * G * F;
        float denominator = max(4.0 * NoV * NoL, 0.001);
        float3 Specular = nominator / denominator;
    
        float3 Diffuse = kD * basecolor / PI;
        Lightfinalcolor = (Diffuse + Specular) * NoL * Radiance;
    }
    
    //ENV
    float3 ENVfinalColor;
    {
        float3 F = Fresnel_Schlick_Roughness(NoV, F0, Roughness);
        float3 kS = F;
        float3 kD = 1.0 - kS;
        kD *= (1.0 - Metallic);
        float3 irradiance = sky.Sample(s1, N).rgb*2;
        
        float3 diffuse = basecolor*irradiance;
        
        float3 prefilteredColor = sky.Sample(s1,R);
        float2 brdf = IBLLut.Sample(s1, float2(NoV,Roughness)).rg;
        float3 specular = prefilteredColor * (F0 * brdf.x + brdf.y);
        
        float3 ambient = (kD * diffuse + specular) * AO;
        
        ENVfinalColor = ambient;
    }
    
    float3 finalcolor = Lightfinalcolor + ENVfinalColor;
    
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

float3 Blinn_Phong(float3 baseColor, float specularPow, float specularStrength, float3 L, float3 N, float3 V)
{
    float3 H = normalize(V + L);
    float NoH = saturate(dot(N, H));
    float NoL = saturate(dot(N, L));
    float3 Diffuse = NoL * baseColor;
    float3 Specular = pow(NoH, specularPow) * specularStrength;
    return (Specular + Diffuse);

}



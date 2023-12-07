#include "ShadingModel.hlsl"
#include "PostProcess.hlsl"

Texture2D t1 : register(t0);
Texture2D t2 : register(t1);
SamplerState s1 : register(s0);
SamplerState s2 : register(s1);

cbuffer SceneConstantBuffer : register(b0)
{
    float4x4 ObjectToWorld;
    float4x4 WorldViewProj;
    float4 lightColor;
    float4 lightDirection;
    float4 cameraPosition;
}


struct VertexInput
{
    float4 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 texCoord : TEXCOORD;
    float4 color : COLOR;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 texCoord : TEXCOORD;
    float4 color : COLOR;
    float4 worldposition :POSITION;
};


PSInput VSMain(VertexInput input)
{
    PSInput result;
    result.worldposition = mul(input.position,ObjectToWorld);
    result.position = mul(input.position, WorldViewProj);
    result.normal = normalize(mul(input.normal, (float3x3) ObjectToWorld));
    result.tangent = normalize(mul(input.tangent, (float3x3) ObjectToWorld));
    result.texCoord = input.texCoord;
    result.color = input.color;

    return result;
}


float4 PSMain(PSInput input) : SV_TARGET
{
    float4 BaseColorTex = t1.Sample(s1, input.texCoord);
    float4 NormalTex = t2.Sample(s1, input.texCoord);
    
    float3 tangent = input.tangent;
    float3 normal = input.normal;
    float3 bitangent = normalize(cross(normal, tangent));
    
    float3 basecolor = pow(BaseColorTex.rgb,2.2);
    float Metalic = 0.0;
    float roughness = lerp(0.5f, 1.0f, NormalTex.a);
    float AO = BaseColorTex.a;
    //float3 N = NormalTex.rgb;
    float3 N = normalize(UnpackNormal(NormalTex.rgb));
    //N.rg *= 5;
    //N = normalize(N);
    N = TransformTangentToWorld(normal, tangent, bitangent,N);
    
    float3 Radiance = lightColor.rgb*5;
    
    float3 L = normalize(lightDirection.xyz);
    float3 V = normalize(cameraPosition.xyz - input.worldposition.xyz);
    
    float3 ambient =0.3*basecolor*AO;
    
    float3 color;
    float3 brdfcolor = BRDF(basecolor,Metalic,roughness,Radiance,L,V,N)+ambient;
    //color = Blinn_Phong(basecolor, 20, 1 - roughness, L, N, V) + ambient;
    //color = Phong(basecolor,20,1 - roughness,L,N,V)+ambient;
    color = ACES_Tonemapping(brdfcolor);

    float3 finalcolor = pow(color,1/2.2);
    
    return float4(finalcolor, 1.0);
}
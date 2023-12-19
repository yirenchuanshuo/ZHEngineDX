
Texture2D t1 : register(t0);
Texture2D t2 : register(t1);

Texture2D IBLLut : register(t2);

TextureCube sky : register(t3);
SamplerState s1 : register(s0);

cbuffer SceneConstantBuffer : register(b0)
{
    float4x4 ObjectToWorld;
    float4x4 ViewProj;
    float4x4 WorldViewProj;
    float4 lightColor;
    float4 lightDirection;
    float4 cameraPosition;
}


float3x3 TBN(float3 normal, float3 tangent, float3 bitangent)
{
    
    float3x3 TBN = float3x3(tangent, bitangent, normal);

    return TBN;
}

float3 UnpackNormal(float3 normal)
{
    return normal * 2 - 1;
}

float3 TransformTangentToWorld(float3 normal, float3 tangent, float3 bitangent,float3 tangentSpaceNormal)
{
    float3x3 TBNMatrix = TBN(tangent, bitangent, normal);
    return normalize(mul(tangentSpaceNormal, TBNMatrix));
}





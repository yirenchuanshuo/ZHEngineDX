Texture2D t1 : register(t0);
SamplerState s1 : register(s0);

cbuffer SceneConstantBuffer : register(b0)
{
    float4x4 ObjectToWorld;
    float4x4 WorldViewProj;
    float4 lightColor;
    float3 lightDirection;
    float3 viewPosition;
}
struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float4 color : COLOR;
    float4 worldposition :POSITION;
};

PSInput VSMain(float4 position : POSITION,float3 normal :NORMAL , float2 texCoord : TEXCOORD,float4 color : COLOR)
{
    PSInput result;
    result.worldposition = mul(position,ObjectToWorld);
    result.position = mul(position,WorldViewProj);
    result.normal = normal;
    result.texCoord = texCoord;
    result.color = color;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float2 texCoord = float2(input.texCoord.x, 1-input.texCoord.y);
    float4 basecolor = t1.Sample(s1, texCoord);
    float ambientStrength = 0.1;
    float3 ambient = ambientStrength * lightColor.xyz;
    
    float3 normal = normalize(input.normal);
    float lambert = max(dot(normal, normalize(lightDirection)), 0.0);
    float3 diffuse = lambert * lightColor.xyz;
    diffuse += ambient;
    
    return float4(diffuse*basecolor.xyz, 1.0);
}
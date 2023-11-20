Texture2D t1 : register(t0);
Texture2D t2 : register(t1);
SamplerState s1 : register(s0);
SamplerState s2 : register(s1);
cbuffer SceneConstantBuffer : register(b0)
{
    float4x4 ObjectToWorld;
    float4x4 WorldViewProj;
    float4 lightColor;
    float3 lightDirection;
    float3 viewPosition;
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
    result.normal = input.normal;
    result.tangent = input.tangent;
    result.texCoord = input.texCoord;
    result.color = input.color;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    
    float4 basecolor = t1.Sample(s1, input.texCoord);
    float ambientStrength = 0.2;
    float3 ambient = ambientStrength * lightColor.xyz*basecolor.xyz;
    
    float3 normal = normalize(input.normal);
    float3 tangent = input.tangent;
    float halflambert = dot(normal, lightDirection) * 0.5 + 0.5;
    //float lambert = max(dot(normal, lightDirection), 0.0);
    float3 diffuse = halflambert * lightColor.xyz;
    diffuse *= basecolor.xyz;
    
    float specularStrength = 0.5;
    float3 viewDir = normalize(viewPosition - input.worldposition.xyz);
    float3 halfDir = normalize(lightDirection + viewDir);
    float spec = pow(max(dot(halfDir, normal), 0.0), 32);
    float3 specular = specularStrength*spec*lightColor.xyz;
    
    
    float4 normalcolor = t2.Sample(s2, input.texCoord);
    normalcolor = float4(tangent, 1.0);
    float4 finalcolor = float4(ambient+diffuse+specular,1.0);
    
    return normalcolor;

}
Texture2D t1 : register(t0);
SamplerState s1 : register(s0);

cbuffer SceneConstantBuffer : register(b0)
{
    float4x4 WorldViewProj;
}
struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float4 color : COLOR;
};

PSInput VSMain(float4 position : POSITION,float3 normal :NORMAL , float2 texCoord : TEXCOORD,float4 color : COLOR)
{
    PSInput result;

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
    return basecolor;
}
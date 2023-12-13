#include"Common.hlsl"

TextureCube sky : register(t2);
SamplerState s1 : register(s0);
SamplerState s2 : register(s1);


struct VertexInput
{
    float4 position : POSITION;
    float2 texCoord : TEXCOORD;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float4 localposition : POSITION;
};
 
PSInput VSMain(VertexInput vin)
{
    PSInput result;

    result.localposition = vin.position;

    float4 worldposition = mul(vin.position,ObjectToWorld);
	
    worldposition.xyz += cameraPosition;

    result.position = mul(worldposition, ViewProj);
    
    result.texCoord = vin.texCoord;
	
    return result;
}

float4 PSMain(PSInput pin) : SV_Target
{
    return sky.Sample(s1, pin.localposition.xyz);
}


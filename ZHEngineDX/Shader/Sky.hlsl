#include "Common.hlsl"
#include "PostProcess.hlsl"


cbuffer ObjectConstantBuffer : register(b0)
{
    float4x4 ObjectToWorld;
    float4x4 WorldViewProj;
}


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
    float4 basecolor = sky.Sample(s1, pin.localposition.xyz);
    //float4 skycolor = pow(basecolor, 2.2);
    float4 skycolor = basecolor;
    //skycolor.rgb = ACES_Tonemapping(basecolor.rgb);
    //skycolor = pow(basecolor, 1 / 2.2);
    return skycolor;
}


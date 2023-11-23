


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





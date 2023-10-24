#pragma once
#include "TypeName.h"

inline FVector4 MakeFvector4(float&& x,float&& y,float&& z,float&& w)
{
	return XMVectorSet(x,y,z,w);
}

inline FMatrix4x4 LookAt(FVector4 &pos,FVector4& target,FVector4 &up)
{
	return XMMatrixLookAtLH(pos, target, up);
}

inline FMatrix4x4 MatrixIdentity()
{
	return XMMatrixIdentity();
}

inline FMatrix4x4 MatrixFov(float angle,float screenRatio,float nearPlane,float FarPlane)
{
	return XMMatrixPerspectiveFovLH(angle, screenRatio, nearPlane, FarPlane);
}

inline void MaterixToFloat4x4(XMFLOAT4X4* f,FMatrix4x4& m)
{
	XMStoreFloat4x4(f, XMMatrixTranspose(m));
}
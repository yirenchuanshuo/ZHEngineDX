#pragma once
#include "TypeName.h"

namespace ZHEngineMath
{

	template <typename X, typename Y, typename Z, typename W>
	static FVector4 MakeFvector4(X&& x, Y&& y, Z&& z, W&& w)
	{
		return XMVectorSet(std::forward<X>(x), std::forward<Y>(y), std::forward<Z>(z), std::forward<W>(w));
	}

	static FMatrix4x4 LookAt(FVector4 &pos,FVector4& target,FVector4 &up)
	{
		return XMMatrixLookAtLH(pos, target, up);
	}

	static FMatrix4x4 MatrixIdentity()
	{
		return XMMatrixIdentity();
	}

	static FMatrix4x4 MatrixFov(float angle,float screenRatio,float nearPlane,float FarPlane)
	{
		return XMMatrixPerspectiveFovLH(angle, screenRatio, nearPlane, FarPlane);
	}

	static void MaterixToFloat4x4(XMFLOAT4X4* f,FMatrix4x4& m)
	{
		XMStoreFloat4x4(f, XMMatrixTranspose(m));
	}

	template<typename T>
	static T Clamp(const T& x, const T& low, const T& high)
	{
		return x < low ? low : (x > high ? high : x);
	}
}
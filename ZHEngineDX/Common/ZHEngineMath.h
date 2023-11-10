#pragma once
#include "TypeName.h"
#include <tuple>

namespace ZMath
{
	constexpr  float PI       = 3.1415926535f;

	//FloatMake
	static Float4x4 Float4x4Identity()
	{
		static Float4x4 I(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

		return I;
	}
	
	//VectorMake
	template <typename X, typename Y, typename Z, typename W>
	static FVector4 MakeFvector4(X&& x, Y&& y, Z&& z, W&& w)
	{
		return XMVectorSet(std::forward<X>(x), std::forward<Y>(y), std::forward<Z>(z), std::forward<W>(w));
	}

	static FVector4 MakeVectorByFloat(float x)
	{
		return XMVectorReplicate(x);
	}

	static FMatrix4x4 MatrixIdentity()
	{
		return XMMatrixIdentity();
	}
	

	//VectorConvertToFloat
	static Float4 FVector4ToFloat4(FVector4& V)
	{
		Float4 Temp;
		XMStoreFloat4(&Temp, V);
		return Temp;
	}

	static Float3 FVector3ToFloat3(FVector3& V)
	{
		Float3 Temp;
		XMStoreFloat3(&Temp, V);
		return Temp;
	}

	static void MaterixToFloat4x4(XMFLOAT4X4* pSource,FMatrix4x4& m)
	{
		XMStoreFloat4x4(pSource, XMMatrixTranspose(m));
	}

	//FloatConvertToVector
	static FVector3 Float3ToFVector3(const Float3* pSource)
	{
		FVector3 Temp{0};
		XMLoadFloat3(pSource);
		return Temp;
	}

	static FVector4 Float4ToFVector4(const Float4* pSource)
	{
		FVector4 Temp{0};
		XMLoadFloat4(pSource);
		return Temp;
	}

	static FMatrix4x4 Float4x4ToMatrix4x4(const Float4x4* pSource)
	{
		 return XMLoadFloat4x4(pSource);
	}
	
	//VectorMatrix
	static FMatrix4x4 LookAt(FVector4 &pos,FVector4& target,FVector4 &up)
	{
		return XMMatrixLookAtLH(pos, target, up);
	}
	
	
	static FMatrix4x4 MatrixFov(float angle,float screenRatio,float nearPlane,float FarPlane)
	{
		return XMMatrixPerspectiveFovLH(angle, screenRatio, nearPlane, FarPlane);
	}


	//Common Float
	template<typename T>
	static T Clamp(const T& x, const T& low, const T& high)
	{
		return x < low ? low : (x > high ? high : x);
	}

	static float AngleToRadians(float Angle)
	{
		return XMConvertToRadians(Angle);
	}
	
	static Float3 Normalize(Float3& vector)
	{
		FVector3 Temp = XMLoadFloat3(&vector);
		Float3 Ret;
		XMStoreFloat3(&Ret,Temp);
		return Ret;
	}

	//Common Vector
	static FVector3 Cross(FVector3 V1,FVector3 V2)
	{
		return XMVector3Cross(V1,V2);
	}

	static FVector3 Normalize(FXMVECTOR& V)
	{
		return XMVector3Normalize(V);
	}

	static FVector4 Normalize4(FXMVECTOR& V)
	{
		return XMVector4Normalize(V);
	}

	static FVector3 TransformNormal(FVector3& pSoure,FMatrix4x4& Matrix)
	{
		return XMVector3TransformNormal(pSoure,Matrix);
	}

	static FMatrix4x4 MatrixRotateAboutAxis(FVector3& Axis,float Angle)
	{
		return XMMatrixRotationAxis(Axis, Angle);
	}

	static FMatrix4x4 MatrixRotateX(float Angle)
	{
		return XMMatrixRotationX(Angle);
	}

	static FMatrix4x4 MatrixRotateY(float Angle)
	{
		return XMMatrixRotationY(Angle);
	}

	static FMatrix4x4 MatrixRotateZ(float Angle)
	{
		return XMMatrixRotationZ(Angle);
	}

	static FVector3 QuaternionRotateAboutAxis(FVector3& Axis,float Angle)
	{
		return XMQuaternionRotationAxis(Axis, Angle);
	}

	//Common
	
	
}
#pragma once
#include "TypeName.h"
#include <tuple>

#define UPPER(A,B) ((UINT)(((A)+((B)-1))&~(B - 1)))

template<typename T>
constexpr UINT CalcConstantBufferByteSize()
{
	const UINT byteSize = sizeof(T);
	return (byteSize + 255) & ~255;
}

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
	template <typename V>
	static float GetFVectorX(V& v)
	{
		return XMVectorGetX(v);
	}

	template <typename V>
	static float GetFVectorY(V& v)
	{
		return XMVectorGetY(v);
	}

	template <typename V>
	static float GetFVectorZ(V& v)
	{
		return XMVectorGetZ(v);
	}

	template <typename V>
	static float GetFVectorW(V& v)
	{
		return XMVectorGetW(v);
	}


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

	static Float3 FVector3ToFloat3(const FVector3& V)
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
	static FVector2 Float2ToFVector2(const Float2* pSource)
	{
		return  XMLoadFloat2(pSource);
	}

	static FVector3 Float3ToFVector3(const Float3* pSource)
	{
		return XMLoadFloat3(pSource);
	}

	static FVector4 Float4ToFVector4(const Float4* pSource)
	{
		return XMLoadFloat4(pSource);
	}

	static FMatrix4x4 Float4x4ToMatrix4x4(const Float4x4* pSource)
	{
		 return XMLoadFloat4x4(pSource);
	}

	static FMatrix4x4 TransposeMatrix(const FMatrix4x4& pSource)
	{
		FMatrix4x4 Transpose = DirectX::XMMatrixTranspose(pSource);
		return Transpose;
	}

	static FMatrix4x4 InverseMatrix(const FMatrix4x4& pSource)
	{
		FMatrix4x4 Inverse = DirectX::XMMatrixInverse(nullptr, pSource);
		return Inverse;
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
		FVector3 Temp = XMVector3Normalize(XMLoadFloat3(&vector));
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

	static FVector3 TransformNormal(const FVector3& pSoure,FMatrix4x4& Matrix)
	{
		return XMVector3TransformNormal(pSoure,Matrix);
	}

	static FMatrix4x4 MatrixRotateAboutAxis(const FVector3& Axis,float Angle)
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

	static FVector4 Transform(FVector4& V,FMatrix4x4& M)
	{
		return XMVector3Transform(V, M);
	}

	static FVector4 QuaternionRotate(FVector4& Axis,float Angle)
	{
		return XMQuaternionRotationAxis(Axis, Angle);
	}

	//Common
	static FMatrix4x4 VectorToMatrix(FVector4 &v)
	{
		return XMMatrixTranslationFromVector(v);
	}
}
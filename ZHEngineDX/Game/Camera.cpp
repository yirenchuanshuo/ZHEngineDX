#include "Camera.h"

Camera::Camera()
{
	SetLens(0.25f*ZMath::PI, 1.0f, 1.0f, 1000.0f);
}

Camera::~Camera()
{
}

void Camera::Strafe(float d)
{
	FVector3 s = ZMath::MakeVectorByFloat(d);
	FVector3 r = ZMath::Float3ToFVector3(&g_Right);
	FVector3 p = ZMath::Float3ToFVector3(&g_Position);

	g_Position = ZMath::FVector3ToFloat3(s*r+p);
	g_ViewDirty = true;
}

void Camera::Walk(float d)
{
	FVector3 s = ZMath::MakeVectorByFloat(d);
	FVector3 l = ZMath::Float3ToFVector3(&g_Right);
	FVector3 p = ZMath::Float3ToFVector3(&g_Position);

	g_Position = ZMath::FVector3ToFloat3(s*l+p);

	g_ViewDirty = true;
}

void Camera::Pitch(float angle)
{
	FMatrix4x4 R = ZMath::MatrixRotateAboutAxis(ZMath::Float3ToFVector3(&g_Right),angle);

	g_Up = ZMath::FVector3ToFloat3(ZMath::TransformNormal(ZMath::Float3ToFVector3(&g_Up),R));
	g_Look = ZMath::FVector3ToFloat3(ZMath::TransformNormal(ZMath::Float3ToFVector3(&g_Look),R));

	g_ViewDirty = true;
}

void Camera::RotateY(float angle)
{
	FMatrix4x4 R = ZMath::MatrixRotateY(angle);

	g_Right = ZMath::FVector3ToFloat3(ZMath::TransformNormal(ZMath::Float3ToFVector3(&g_Right),R));
	g_Up = ZMath::FVector3ToFloat3(ZMath::TransformNormal(ZMath::Float3ToFVector3(&g_Up),R));
	g_Look = ZMath::FVector3ToFloat3(ZMath::TransformNormal(ZMath::Float3ToFVector3(&g_Look),R));

	g_ViewDirty = true;
}

void Camera::SetPosition(float x, float y, float z)
{
	g_Position = Float3(x,y,z);
	g_ViewDirty = true;
}

void Camera::SetPosition(const Float3& v)
{
	g_Position = v;
	g_ViewDirty = true;
}

FVector3 Camera::GetRight() const
{
	return  ZMath::Float3ToFVector3(&g_Right);
}

Float3 Camera::GetRight3f() const
{
	return  g_Right;
}

FVector3 Camera::GetUp() const
{
	return ZMath::Float3ToFVector3(&g_Up);
}

Float3 Camera::GetUp3f() const
{
	return g_Up;
}

FVector3 Camera::GetLook() const
{
	return ZMath::Float3ToFVector3(&g_Look);
}

Float3 Camera::GetLook3f() const
{
	return g_Look;
}

void Camera::SetLens(float fovY, float aspect, float zn, float zf)
{
	g_FovY = fovY;
	g_Aspect = aspect;
	g_NearZ = zn;
	g_FarZ = zf;

	g_NearWindowHeight = 2.0f * g_NearZ * tanf( 0.5f*g_FovY );
	g_FarWindowHeight  = 2.0f * g_FarZ * tanf( 0.5f*g_FovY );

	FMatrix4x4 P = ZMath::MatrixFov(g_FovY, g_Aspect, g_NearZ, g_FarZ);
	ZMath::MaterixToFloat4x4(&g_Proj, P);
}

void Camera::UpdateViewMatrix()
{
	if(g_ViewDirty)
	{
		XMVECTOR R = XMLoadFloat3(&g_Right);
		XMVECTOR U = XMLoadFloat3(&g_Up);
		XMVECTOR L = XMLoadFloat3(&g_Look);
		XMVECTOR P = XMLoadFloat3(&g_Position);

		// Keep camera's axes orthogonal to each other and of unit length.
		L = XMVector3Normalize(L);
		U = XMVector3Normalize(XMVector3Cross(L, R));

		// U, L already ortho-normal, so no need to normalize cross product.
		R = XMVector3Cross(U, L);

		// Fill in the view matrix entries.
		float x = -XMVectorGetX(XMVector3Dot(P, R));
		float y = -XMVectorGetX(XMVector3Dot(P, U));
		float z = -XMVectorGetX(XMVector3Dot(P, L));

		XMStoreFloat3(&g_Right, R);
		XMStoreFloat3(&g_Up, U);
		XMStoreFloat3(&g_Look, L);

		g_View(0, 0) = g_Right.x;
		g_View(1, 0) = g_Right.y;
		g_View(2, 0) = g_Right.z;
		g_View(3, 0) = x;

		g_View(0, 1) = g_Up.x;
		g_View(1, 1) = g_Up.y;
		g_View(2, 1) = g_Up.z;
		g_View(3, 1) = y;

		g_View(0, 2) = g_Look.x;
		g_View(1, 2) = g_Look.y;
		g_View(2, 2) = g_Look.z;
		g_View(3, 2) = z;

		g_View(0, 3) = 0.0f;
		g_View(1, 3) = 0.0f;
		g_View(2, 3) = 0.0f;
		g_View(3, 3) = 1.0f;

		g_ViewDirty = false;
	}
}

void Camera::LookAt(FVector3 pos, FVector3 target, FVector3 worldUp)
{
	FVector3 L = ZMath::Normalize(target-pos);
	FVector3 R = ZMath::Normalize(ZMath::Cross(worldUp, L));
	FVector3 U = ZMath::Cross(L, R);

	g_Position = ZMath::FVector3ToFloat3(pos);
	g_Look = ZMath::FVector3ToFloat3(L);
	g_Right = ZMath::FVector3ToFloat3(R);
	g_Up = ZMath::FVector3ToFloat3(U);
	
	g_ViewDirty = true;
}

void Camera::LookAt(const Float3& pos, const Float3& target, const Float3& up)
{
	FVector3 P = ZMath::Float3ToFVector3(&pos);
	FVector3 T = ZMath::Float3ToFVector3(&target);
	FVector3 U = ZMath::Float3ToFVector3(&up);

	ZMath::LookAt(P,T,U);

	g_ViewDirty=true;
}

FMatrix4x4 Camera::GetView() const
{
	assert(!g_ViewDirty);
	return ZMath::Float4x4ToMatrix4x4(&g_View);
}

FMatrix4x4 Camera::GetProj() const
{
	return ZMath::Float4x4ToMatrix4x4(&g_Proj);
}

Float4x4 Camera::GetView4x4f() const
{
	assert(!g_ViewDirty);
	return g_View;
}

Float4x4 Camera::GetProj4x4f() const
{
	return g_Proj;
}

FVector3 Camera::GetPosition()const
{
	return ZMath::Float3ToFVector3(&g_Position);
}

Float3 Camera::GetPosition3f() const
{
	return g_Position;
}

float Camera::GetNearZ() const
{
	return g_NearZ;
}

float Camera::GetFarZ() const
{
	return g_FarZ;
}

float Camera::GetAspect() const
{
	return g_Aspect;
}

float Camera::GetFovY() const
{
	return g_FovY;
}

float Camera::GetFovX() const
{
	float halfWidth = 0.5f * GetNearWindowWidth();
	return 2.0f * atan(halfWidth / g_NearZ);
}

float Camera::GetNearWindowWidth() const
{
	return g_Aspect * g_NearWindowHeight;
}

float Camera::GetNearWindowHeight() const
{
	return g_NearWindowHeight;
}

float Camera::GetFarWindowWidth() const
{
	return g_Aspect * g_FarWindowHeight;
}

float Camera::GetFarWindowHeight() const
{
	return g_FarWindowHeight;
}

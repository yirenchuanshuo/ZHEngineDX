#include "Camera.h"

Camera::Camera()
{
	SetLens(0.25f*ZHEngineMath::PI, 1.0f, 1.0f, 1000.0f);
}

Camera::~Camera()
{

}

void Camera::SetLens(float fovY, float aspect, float zn, float zf)
{
	g_FovY = fovY;
	g_Aspect = aspect;
	g_NearZ = zn;
	g_FarZ = zf;

	g_NearWindowHeight = 2.0f * g_NearZ * tanf( 0.5f*g_FovY );
	g_FarWindowHeight  = 2.0f * g_FarZ * tanf( 0.5f*g_FovY );

	FMatrix4x4 P = ZHEngineMath::MatrixFov(g_FovY, g_Aspect, g_NearZ, g_FarZ);
	ZHEngineMath::MaterixToFloat4x4(&g_Proj, P);
}

void Camera::LookAt(FVector3 pos, FVector3 target, FVector3 worldUp)
{
	FVector3 L = XMVector3Normalize(XMVectorSubtract(target, pos));
	FVector3 R = XMVector3Normalize(XMVector3Cross(worldUp, L));
	FVector3 U = XMVector3Cross(L, R);

	g_Position = ZHEngineMath::FVector3ToFloat3(pos);
	g_Look = ZHEngineMath::FVector3ToFloat3(L);
	g_Right = ZHEngineMath::FVector3ToFloat3(R);
	g_Up = ZHEngineMath::FVector3ToFloat3(U);
	
	g_ViewDirty = true;
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

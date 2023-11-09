#pragma once
#include "../Common/TypeName.h"
#include <Windows.h>

#include "../Common/ZHEngineMath.h"

class Camera
{
public:
	Camera();
	~Camera();

	void SetLens(float fovY, float aspect, float zn, float zf);

	void LookAt(FVector3 pos, FVector3 target, FVector3 worldUp);

public:
	float g_Theta = 1.5f * PI;
	float g_Phi = PIDIV4;
	float g_Radius = 5.0f;

	POINT g_LastMousePos;

	Float3 g_Position =  { 0.0f, 0.0f, 0.0f };
	Float3 g_Right = { 1.0f, 0.0f, 0.0f };
	Float3 g_Up = { 0.0f, 1.0f, 0.0f };
	Float3 g_Look = { 0.0f, 0.0f, 1.0f };
	
	float GetNearZ()const;
	float GetFarZ()const;
	float GetAspect()const;
	float GetFovY()const;
	float GetFovX()const;


	float GetNearWindowWidth()const;
	float GetNearWindowHeight()const;
	float GetFarWindowWidth()const;
	float GetFarWindowHeight()const;

private:
	float g_NearZ = 0.0f;
	float g_FarZ = 0.0f;
	float g_Aspect = 0.0f;
	float g_FovY = 0.0f;
	float g_NearWindowHeight = 0.0f;
	float g_FarWindowHeight = 0.0f;

	bool g_ViewDirty = true;

	Float4x4 g_View = ZHEngineMath::Float4x4Identity();
	Float4x4 g_Proj = ZHEngineMath::Float4x4Identity();
};

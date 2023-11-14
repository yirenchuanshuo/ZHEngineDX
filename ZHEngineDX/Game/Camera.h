#pragma once
#include "../Common/TypeName.h"
#include <Windows.h>

#include "../Common/ZHEngineMath.h"

class Camera
{
public:
	Camera();
	~Camera();

	void Init(Float3 position);
	void Strafe(float d);
	void Walk(float d);

	void Pitch(float angle);
	void RotateY(float angle);
	
	void SetLens(float fovY, float aspect, float zn, float zf);
	void UpdateViewMatrix();

	void LookAt(FVector3 pos, FVector3 target, FVector3 worldUp);
	void LookAt(const Float3& pos,const Float3& target,const Float3& up);

	FMatrix4x4 GetView()const;
	FMatrix4x4 GetProj()const;

	Float4x4 GetView4x4f()const;
	Float4x4 GetProj4x4f()const;

public:
	float g_Theta = 1.5f * PI;
	float g_Phi = PIDIV4;
	float g_Radius = 5.0f;

	POINT g_LastMousePos;

	Float3 g_InitPosition = { 0.0f, 0.0f, 0.0f };
	Float3 g_Position;
	Float3 g_Right = { 1.0f, 0.0f, 0.0f };
	Float3 g_Up = { 0.0f, 1.0f, 0.0f };
	Float3 g_Look = { 0.0f, 0.0f, -1.0f };

	FVector3 GetPosition()const;
	Float3 GetPosition3f()const;
	void SetPosition(float x,float y,float z);
	void SetPosition(const Float3& v);

	FVector3 GetRight()const;
	Float3 GetRight3f()const;
	FVector3 GetUp()const;
	Float3 GetUp3f()const;
	FVector3 GetLook()const;
	Float3 GetLook3f()const;
	
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
	void Reset();


	float g_NearZ = 0.0f;
	float g_FarZ = 0.0f;
	float g_Aspect = 0.0f;
	float g_FovY = 0.0f;
	float g_NearWindowHeight = 0.0f;
	float g_FarWindowHeight = 0.0f;

	bool g_ViewDirty = true;

	Float4x4 g_View = ZMath::Float4x4Identity();
	Float4x4 g_Proj = ZMath::Float4x4Identity();
};

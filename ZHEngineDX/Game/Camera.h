#pragma once
#include "../Common/TypeName.h"
#include <Windows.h>

class Camera
{

public:
	float g_Theta = 1.5f * PI;
	float g_Phi = PIDIV4;
	float g_Radius = 5.0f;

	POINT g_LastMousePos;
};

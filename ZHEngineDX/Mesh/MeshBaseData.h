#pragma once
#include <Windows.h>
#include "../Common/TypeName.h"

//Data
struct Vertex
{
    Float3 position;
    Float3 normal;
    Float3 tangent;
    Float2 texcoord;
    FLinearColor color = {1.0f,1.0f,1.0f,1.0f};
};

struct Face
{
    UINT A;
    UINT B;
    UINT C;
};
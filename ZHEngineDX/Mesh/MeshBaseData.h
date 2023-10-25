#pragma once
#include <Windows.h>
#include "../Common/TypeName.h"

//Data
struct Vertex
{
    Float3 position;
    FLinearColor color;
};

struct Face
{
    UINT A;
    UINT B;
    UINT C;
};
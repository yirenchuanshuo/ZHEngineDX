#pragma once
#include"../Game/Game.h"


using Microsoft::WRL::ComPtr;

class HelloGame :public Game
{
public:
	HelloGame(UINT width, UINT height, std::wstring name);

    virtual void OnInit();
    virtual void OnUpdate();
    virtual void OnRender();
    virtual void OnDestroy();
};
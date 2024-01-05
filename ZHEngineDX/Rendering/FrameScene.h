#pragma once
#include "RenderActor.h"

class UFrameScene
{
public:
	UFrameScene(ID3D12Device* pDevice);
	~UFrameScene();

public:
	std::vector<RenderActor> SceneAcotrs;
	ComPtr<ID3D12CommandAllocator> g_commandAllocator;


private:

};
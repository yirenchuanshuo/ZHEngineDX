#pragma once
#include "RenderActorInterface.h"

class UFrameScene
{
public:
	UFrameScene(ID3D12Device* pDevice);
	~UFrameScene();

public:
	void RegisiterRenderInstance(std::shared_ptr<URenderActorInterface>& RenderInstance);

public:
	std::vector<std::shared_ptr<URenderActorInterface>> SceneAcotrs;
	ComPtr<ID3D12CommandAllocator> g_commandAllocator;
	UINT64 g_fenceValue;

private:

};
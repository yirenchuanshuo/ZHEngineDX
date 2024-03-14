#pragma once
#include "RenderActorInterface.h"
#include "PostRenderActorInterface.h"

class UFrameScene
{
public:
	UFrameScene(ID3D12Device* pDevice);
	~UFrameScene();

public:
	void RegisiterRenderInstance(std::shared_ptr<URenderActorInterface>& RenderInstance);

	void RegisiterRenderPostProcess(std::shared_ptr<UPostRenderActorInterface>& PostRenderInstance);
public:
	std::vector<std::shared_ptr<URenderActorInterface>> SceneAcotrs;
	std::shared_ptr<UPostRenderActorInterface> PostActors;
	ComPtr<ID3D12CommandAllocator> g_commandAllocator;
	UINT64 g_fenceValue;

private:

};
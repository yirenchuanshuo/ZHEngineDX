#include "FrameScene.h"

UFrameScene::UFrameScene(ID3D12Device* pDevice):g_fenceValue(0)
{
	ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_commandAllocator)));
}

UFrameScene::~UFrameScene()
{
}

void UFrameScene::RegisiterRenderInstance(std::shared_ptr<URenderActorInterface>& RenderInstance)
{
	SceneAcotrs.emplace_back(std::move(RenderInstance));
}

void UFrameScene::RegisiterRenderPostProcess(std::shared_ptr<UPostRenderActorInterface>& PostRenderInstance)
{
	PostActors = PostRenderInstance;
}


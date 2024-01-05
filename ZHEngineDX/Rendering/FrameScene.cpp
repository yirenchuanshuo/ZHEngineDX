#include "FrameScene.h"

UFrameScene::UFrameScene(ID3D12Device* pDevice)
{
	ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_commandAllocator)));
}

UFrameScene::~UFrameScene()
{
}

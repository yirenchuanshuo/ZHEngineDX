#pragma once
#include "RenderActor.h"

class URenderActorInterface
{
public:
	URenderActorInterface(std::shared_ptr<RenderActor>& RenderActorBase);

public:
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> g_bundleAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> g_bundle;

public:
	void RecordCommands(ID3D12Device* pDevice, ID3D12DescriptorHeap* pSamplerDescriptorHeap, UINT frameIndex, UINT cbvSrvDescriptorSize) const;

private:
	std::shared_ptr<RenderActor> pRenderActor;

};
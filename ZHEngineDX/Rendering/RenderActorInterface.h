#pragma once
#include "RenderActor.h"

class RenderActor;
class URenderActorInterface
{
public:
	URenderActorInterface(std::shared_ptr<RenderActor>& RenderActorBase);
	~URenderActorInterface();

public:
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> g_bundleAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> g_bundle;

public:
	ID3D12GraphicsCommandList* GetBundle()const { return g_bundle.Get(); }

public:
	void Init(ID3D12Device* pDevice);

	void RecordCommands(ID3D12Device* pDevice, ID3D12DescriptorHeap* pSamplerDescriptorHeap, UINT frameIndex, UINT cbvSrvDescriptorSize) const;

private:
	std::shared_ptr<RenderActor> pRenderActor;

};
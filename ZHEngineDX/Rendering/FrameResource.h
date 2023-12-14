#pragma once
#include "../GameHelper/GameHelper.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

class FrameResource
{
public:
	FrameResource(ID3D12Device* pDevice);
	~FrameResource();

public:
	void InitBundle(ID3D12Device* pDevice, ID3D12PipelineState* pPso1, ID3D12PipelineState* pPso2,
		UINT frameResourceIndex, UINT numIndices, D3D12_INDEX_BUFFER_VIEW* pIndexBufferViewDesc, D3D12_VERTEX_BUFFER_VIEW* pVertexBufferViewDesc,
		ID3D12DescriptorHeap* pCbvSrvDescriptorHeap, UINT cbvSrvDescriptorSize, ID3D12DescriptorHeap* pSamplerDescriptorHeap, ID3D12RootSignature* pRootSignature);

	void PopulateCommandList(ID3D12GraphicsCommandList* pCommandList, ID3D12PipelineState* pPso1, ID3D12PipelineState* pPso2,
		UINT frameResourceIndex, UINT numIndices, D3D12_INDEX_BUFFER_VIEW* pIndexBufferViewDesc, D3D12_VERTEX_BUFFER_VIEW* pVertexBufferViewDesc,
		ID3D12DescriptorHeap* pCbvSrvDescriptorHeap, UINT cbvSrvDescriptorSize, ID3D12DescriptorHeap* pSamplerDescriptorHeap, ID3D12RootSignature* pRootSignature);

public:
	ComPtr<ID3D12CommandAllocator> g_commandAllocator;
	ComPtr<ID3D12CommandAllocator> g_bundleAllocator;
	ComPtr<ID3D12GraphicsCommandList> g_bundle;


	UINT64 g_fenceValue;

private:
	

};
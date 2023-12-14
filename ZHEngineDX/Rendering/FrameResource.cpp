#include "FrameResource.h"

FrameResource::FrameResource(ID3D12Device* pDevice)
{
}

FrameResource::~FrameResource()
{
}

void FrameResource::InitBundle(ID3D12Device* pDevice, ID3D12PipelineState* pPso1, ID3D12PipelineState* pPso2, UINT frameResourceIndex, UINT numIndices, D3D12_INDEX_BUFFER_VIEW* pIndexBufferViewDesc, D3D12_VERTEX_BUFFER_VIEW* pVertexBufferViewDesc, ID3D12DescriptorHeap* pCbvSrvDescriptorHeap, UINT cbvSrvDescriptorSize, ID3D12DescriptorHeap* pSamplerDescriptorHeap, ID3D12RootSignature* pRootSignature)
{

}

void FrameResource::PopulateCommandList(ID3D12GraphicsCommandList* pCommandList, ID3D12PipelineState* pPso1, ID3D12PipelineState* pPso2, UINT frameResourceIndex, UINT numIndices, D3D12_INDEX_BUFFER_VIEW* pIndexBufferViewDesc, D3D12_VERTEX_BUFFER_VIEW* pVertexBufferViewDesc, ID3D12DescriptorHeap* pCbvSrvDescriptorHeap, UINT cbvSrvDescriptorSize, ID3D12DescriptorHeap* pSamplerDescriptorHeap, ID3D12RootSignature* pRootSignature)
{
	pCommandList->SetComputeRootSignature(pRootSignature);

	ID3D12DescriptorHeap* ppHeaps[] = { pCbvSrvDescriptorHeap };
}

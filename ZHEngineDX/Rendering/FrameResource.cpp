#include "FrameResource.h"

FrameResource::FrameResource(ID3D12Device* pDevice):
	g_fenceValue(0)
{
	ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_commandAllocator)));
	ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_BUNDLE, IID_PPV_ARGS(&g_bundleAllocator)));
}

FrameResource::~FrameResource()
{
}

void FrameResource::InitBundle(ID3D12Device* pDevice, ID3D12PipelineState* pipelineState,  UINT frameResourceIndex, UINT numIndices, D3D12_INDEX_BUFFER_VIEW* pIndexBufferViewDesc, D3D12_VERTEX_BUFFER_VIEW* pVertexBufferViewDesc, ID3D12DescriptorHeap* pCbvSrvDescriptorHeap, UINT cbvSrvDescriptorSize, ID3D12DescriptorHeap* pSamplerDescriptorHeap, ID3D12RootSignature* pRootSignature)
{
	ThrowIfFailed(pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, g_bundleAllocator.Get(), pipelineState, IID_PPV_ARGS(&g_bundle)));
	PopulateCommandList(g_bundle.Get(),pipelineState,frameResourceIndex,numIndices,pIndexBufferViewDesc,
	pVertexBufferViewDesc,pCbvSrvDescriptorHeap,cbvSrvDescriptorSize,pSamplerDescriptorHeap,pRootSignature);
	ThrowIfFailed(g_bundle->Close());
}

void FrameResource::PopulateCommandList(ID3D12GraphicsCommandList* pCommandList, ID3D12PipelineState* pipelineState,  UINT frameResourceIndex, UINT numIndices, D3D12_INDEX_BUFFER_VIEW* pIndexBufferViewDesc, D3D12_VERTEX_BUFFER_VIEW* pVertexBufferViewDesc, ID3D12DescriptorHeap* pCbvSrvDescriptorHeap, UINT cbvSrvDescriptorSize, ID3D12DescriptorHeap* pSamplerDescriptorHeap, ID3D12RootSignature* pRootSignature)
{
	pCommandList->SetComputeRootSignature(pRootSignature);
	

	ID3D12DescriptorHeap* ppHeaps[] = { pCbvSrvDescriptorHeap,pSamplerDescriptorHeap };
	pCommandList->SetDescriptorHeaps(_countof(ppHeaps),ppHeaps);

	pCommandList->SetGraphicsRootDescriptorTable(0, pSamplerDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandle = pCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandleForSecondDescriptor = { gpuDescriptorHandle.ptr + cbvSrvDescriptorSize };
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandleForThirdDescriptor = { gpuDescriptorHandle.ptr + cbvSrvDescriptorSize * 3 };
	pCommandList->SetGraphicsRootDescriptorTable(1, gpuDescriptorHandle);
	pCommandList->SetGraphicsRootDescriptorTable(2, gpuDescriptorHandleForSecondDescriptor);
	pCommandList->SetGraphicsRootDescriptorTable(3, gpuDescriptorHandleForThirdDescriptor);

	//UINT frameResourceDescriptorOffset = 4*frameResourceIndex;
	//CD3DX12_GPU_DESCRIPTOR_HANDLE cbvSrvHandle(pCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), frameResourceDescriptorOffset, cbvSrvDescriptorSize);
	pCommandList->SetPipelineState(pipelineState);

	pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pCommandList->IASetIndexBuffer(pIndexBufferViewDesc);
	pCommandList->IASetVertexBuffers(0, 1, pVertexBufferViewDesc);
	
	pCommandList->DrawIndexedInstanced(numIndices, 1, 0, 0, 0);
}

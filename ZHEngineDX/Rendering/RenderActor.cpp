#include "RenderActor.h"

RenderActor::RenderActor()
	:g_vertexBufferView{}, g_indexBufferView{}
{

}

void RenderActor::Init(ID3D12Device* pDevice)
{
	Mesh = std::make_unique<StaticMesh>();
	
	ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_BUNDLE,
		IID_PPV_ARGS(&g_bundleAllocator)));

	ThrowIfFailed(pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, g_bundleAllocator.Get(),
		nullptr, IID_PPV_ARGS(g_bundle.ReleaseAndGetAddressOf())));
}

void RenderActor::RecordCommands(ID3D12Device* pDevice,UINT FrameIndex,ID3D12RootSignature* pRootSignature, ID3D12PipelineState* pPipleLineState, ID3D12DescriptorHeap* pCbvSrvDescriptorHeap, ID3D12DescriptorHeap* pSamplerDescriptorHeap, UINT cbvSrvDescriptorSize)
{
	
	//设置根描述符表，上传参数
	g_bundle->SetGraphicsRootSignature(pRootSignature);
	g_bundle->SetPipelineState(pPipleLineState);

	//设置常量缓冲区描述堆，提交到渲染命令
	ID3D12DescriptorHeap* ppHeaps[] = { pCbvSrvDescriptorHeap ,pSamplerDescriptorHeap};
	g_bundle->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	//设置根描述符表，上传参数
	g_bundle->SetGraphicsRootDescriptorTable(0, pSamplerDescriptorHeap->GetGPUDescriptorHandleForHeapStart());


	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandle = pCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandleForSecondDescriptor = { gpuDescriptorHandle.ptr + cbvSrvDescriptorSize };
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandleForThirdDescriptor = { gpuDescriptorHandle.ptr + cbvSrvDescriptorSize * 3 };
	g_bundle->SetGraphicsRootDescriptorTable(1, gpuDescriptorHandle);
	g_bundle->SetGraphicsRootDescriptorTable(2, gpuDescriptorHandleForSecondDescriptor);
	g_bundle->SetGraphicsRootDescriptorTable(3, gpuDescriptorHandleForThirdDescriptor);

	//图元拓扑模式
	g_bundle->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//顶点装配
	g_bundle->IASetIndexBuffer(&g_indexBufferView);
	g_bundle->IASetVertexBuffers(0, 1, &g_vertexBufferView);

	//画图
	g_bundle->DrawIndexedInstanced(Mesh->GetIndicesSize(), 1, 0, 0, 0);

	g_bundle->Close();
}



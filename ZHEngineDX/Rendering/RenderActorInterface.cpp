#include "RenderActorInterface.h"

URenderActorInterface::URenderActorInterface(std::shared_ptr<RenderActor>& RenderActorBase)
	:pRenderActor(RenderActorBase)
{
	
}

void URenderActorInterface::RecordCommands(ID3D12Device* pDevice, ID3D12DescriptorHeap* pSamplerDescriptorHeap, UINT frameIndex, UINT cbvSrvDescriptorSize) const
{
	//设置根描述符表，上传参数
	g_bundle->SetPipelineState(pRenderActor->PipeLineState.Get());
	g_bundle->SetGraphicsRootSignature(pRenderActor->rootSignature.Get());


	//设置常量缓冲区描述堆，提交到渲染命令
	ID3D12DescriptorHeap* ppHeaps[] = { pRenderActor->cbvsrvHeap.Get() ,pSamplerDescriptorHeap };
	g_bundle->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	//设置根描述符表，上传参数

	UINT GraphicsRootDescriptorPos = 0;

	D3D12_GPU_DESCRIPTOR_HANDLE SampleDescriptorHeapHandle = pSamplerDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	D3D12_DESCRIPTOR_HEAP_DESC SampleheapDesc = pSamplerDescriptorHeap->GetDesc();
	const UINT SampleDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(SampleheapDesc.Type);
	const UINT SampleNums = SampleheapDesc.NumDescriptors;
	for (UINT i = 0; i < SampleNums; i++)
	{
		g_bundle->SetGraphicsRootDescriptorTable(GraphicsRootDescriptorPos, SampleDescriptorHeapHandle);
		SampleDescriptorHeapHandle.ptr += SampleDescriptorSize;
		GraphicsRootDescriptorPos++;
	}



	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandle = pRenderActor->cbvsrvHeap->GetGPUDescriptorHandleForHeapStart();

	for (UINT i = 0; i < pRenderActor->SrvNums; i++)
	{
		g_bundle->SetGraphicsRootDescriptorTable(GraphicsRootDescriptorPos, gpuDescriptorHandle);
		gpuDescriptorHandle.ptr += cbvSrvDescriptorSize;
		GraphicsRootDescriptorPos++;
	}

	UINT CbvNums = pRenderActor->OneFrameCbvSrvNums - pRenderActor->SrvNums;
	gpuDescriptorHandle.ptr += frameIndex * CbvNums * cbvSrvDescriptorSize;

	for (UINT i = 0; i < CbvNums; i++)
	{
		g_bundle->SetGraphicsRootDescriptorTable(GraphicsRootDescriptorPos, gpuDescriptorHandle);
		gpuDescriptorHandle.ptr += cbvSrvDescriptorSize;
		GraphicsRootDescriptorPos++;
	}

	//图元拓扑模式
	g_bundle->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//顶点装配
	g_bundle->IASetIndexBuffer(&pRenderActor->g_indexBufferView);
	g_bundle->IASetVertexBuffers(0, 1, &pRenderActor->g_vertexBufferView);

	//画图
	g_bundle->DrawIndexedInstanced(static_cast<UINT>(pRenderActor->Mesh->GetIndicesSize()), 1, 0, 0, 0);

	g_bundle->Close();
}

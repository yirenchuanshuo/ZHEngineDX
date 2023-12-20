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

void RenderActor::RecordCommands(ID3D12Device* pDevice,UINT FrameIndex,ID3D12RootSignature* pRootSignature, ID3D12PipelineState* pPipleLineState,
	ID3D12DescriptorHeap* pCbvSrvDescriptorHeap, ID3D12DescriptorHeap* pSamplerDescriptorHeap, UINT cbvSrvDescriptorSize)const
{
	
	//���ø����������ϴ�����
	g_bundle->SetGraphicsRootSignature(pRootSignature);
	g_bundle->SetPipelineState(pPipleLineState);

	//���ó��������������ѣ��ύ����Ⱦ����
	ID3D12DescriptorHeap* ppHeaps[] = { pCbvSrvDescriptorHeap ,pSamplerDescriptorHeap};
	g_bundle->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	//���ø����������ϴ�����

	UINT GraphicsRootDescriptorPos = 0;
	
	D3D12_GPU_DESCRIPTOR_HANDLE SampleDescriptorHeapHandle = pSamplerDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	D3D12_DESCRIPTOR_HEAP_DESC SampleheapDesc = pSamplerDescriptorHeap->GetDesc();
	UINT SampleDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(SampleheapDesc.Type);
	UINT SampleNums = SampleheapDesc.NumDescriptors;
	for(UINT i=0;i<SampleNums;i++)
	{
		g_bundle->SetGraphicsRootDescriptorTable(GraphicsRootDescriptorPos,SampleDescriptorHeapHandle);
		SampleDescriptorHeapHandle.ptr+=SampleDescriptorSize;
		GraphicsRootDescriptorPos++;
	}

	D3D12_DESCRIPTOR_HEAP_DESC CbvSrvHeapDesc = pCbvSrvDescriptorHeap->GetDesc();
	UINT CbvSrvNums = CbvSrvHeapDesc.NumDescriptors;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandle = pCbvSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	
	for(UINT i=0;i<CbvSrvNums;i++)
	{
		g_bundle->SetGraphicsRootDescriptorTable(GraphicsRootDescriptorPos, gpuDescriptorHandle);
		gpuDescriptorHandle.ptr+=cbvSrvDescriptorSize;
		GraphicsRootDescriptorPos++;
	}
	
	//ͼԪ����ģʽ
	g_bundle->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//����װ��
	g_bundle->IASetIndexBuffer(&g_indexBufferView);
	g_bundle->IASetVertexBuffers(0, 1, &g_vertexBufferView);

	//��ͼ
	g_bundle->DrawIndexedInstanced(Mesh->GetIndicesSize(), 1, 0, 0, 0);

	g_bundle->Close();
}



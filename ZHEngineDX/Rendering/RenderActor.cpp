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

void RenderActor::LoadMesh(std::string filepath)
{
	Mesh->Load(filepath);
}

void RenderActor::RecordCommands(ID3D12Device* pDevice,ID3D12DescriptorHeap* pSamplerDescriptorHeap, UINT cbvSrvDescriptorSize)const
{
	
	//设置根描述符表，上传参数
	g_bundle->SetPipelineState(PipeLineState.Get());
	g_bundle->SetGraphicsRootSignature(rootSignature.Get());
	

	//设置常量缓冲区描述堆，提交到渲染命令
	ID3D12DescriptorHeap* ppHeaps[] = { cbvsrvHeap.Get() ,pSamplerDescriptorHeap};
	g_bundle->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	//设置根描述符表，上传参数

	UINT GraphicsRootDescriptorPos = 0;
	
	D3D12_GPU_DESCRIPTOR_HANDLE SampleDescriptorHeapHandle = pSamplerDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	D3D12_DESCRIPTOR_HEAP_DESC SampleheapDesc = pSamplerDescriptorHeap->GetDesc();
	const UINT SampleDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(SampleheapDesc.Type);
	const UINT SampleNums = SampleheapDesc.NumDescriptors;
	for(UINT i=0;i<SampleNums;i++)
	{
		g_bundle->SetGraphicsRootDescriptorTable(GraphicsRootDescriptorPos,SampleDescriptorHeapHandle);
		SampleDescriptorHeapHandle.ptr+=SampleDescriptorSize;
		GraphicsRootDescriptorPos++;
	}

	const D3D12_DESCRIPTOR_HEAP_DESC CbvSrvHeapDesc = cbvsrvHeap->GetDesc();
	const UINT CbvSrvNums = CbvSrvHeapDesc.NumDescriptors;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandle = cbvsrvHeap->GetGPUDescriptorHandleForHeapStart();
	
	for(UINT i=0;i<CbvSrvNums;i++)
	{
		g_bundle->SetGraphicsRootDescriptorTable(GraphicsRootDescriptorPos, gpuDescriptorHandle);
		gpuDescriptorHandle.ptr+=cbvSrvDescriptorSize;
		GraphicsRootDescriptorPos++;
	}
	
	//图元拓扑模式
	g_bundle->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//顶点装配
	g_bundle->IASetIndexBuffer(&g_indexBufferView);
	g_bundle->IASetVertexBuffers(0, 1, &g_vertexBufferView);

	//画图
	g_bundle->DrawIndexedInstanced(Mesh->GetIndicesSize(), 1, 0, 0, 0);

	g_bundle->Close();
}

void RenderActor::SetPipleLineState(ID3D12Device* pDevice, D3D12_GRAPHICS_PIPELINE_STATE_DESC& PSODesc)
{
	PSODesc.pRootSignature = rootSignature.Get();


	ThrowIfFailed(pDevice->CreateGraphicsPipelineState(&PSODesc, IID_PPV_ARGS(&PipeLineState)));
	NAME_D3D12_OBJECT(PipeLineState);
}

void RenderActor::SetRootSignature(ID3D12Device* pDevice, CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& rootSignatureDesc)
{
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	if (FAILED(pDevice->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}


	//创建根签名
	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
	ThrowIfFailed(pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));
	NAME_D3D12_OBJECT(rootSignature);
}



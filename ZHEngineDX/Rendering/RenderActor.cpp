#include "RenderActor.h"

RenderActor::RenderActor()
	:g_vertexBufferView{}, g_indexBufferView{}, HandleOffsetNum(0),cbvsrvDescriptorSize(0), Position{0,0,0,1}
{
	
}



void RenderActor::Init(ID3D12Device* pDevice, const wchar_t* shaderfile, const char* vsout, const char* psout, EBlendMode blend)
{
	Mesh = std::make_unique<StaticMesh>();
	Material = std::make_unique<UMaterial>();
	pObjectCbvDataBegin = std::make_shared<UINT8>();
	Material->CompileShader(shaderfile, vsout, psout, blend);
	
	cbvsrvDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_BUNDLE,
		IID_PPV_ARGS(&g_bundleAllocator)));

	ThrowIfFailed(pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, g_bundleAllocator.Get(),
		nullptr, IID_PPV_ARGS(g_bundle.ReleaseAndGetAddressOf())));
}

void RenderActor::LoadMesh(std::string filepath)
{
	Mesh->Load(filepath);
}

void RenderActor::SetTextures(UTexture& Texture)
{
	Material->textures.push_back(Texture);
}

void RenderActor::UpdateMVP(FMatrix4x4& VP)
{
	FMatrix4x4 m = ZMath::VectorToMatrix(Position);
	FMatrix4x4 mvp = m * VP;
	ZMath::MaterixToFloat4x4(&g_ObjectConstantBufferData.ObjectToWorld, m);
	ZMath::MaterixToFloat4x4(&g_ObjectConstantBufferData.ObjectToClip, mvp);
}

void RenderActor::AddHandleOffsetNum()
{
	HandleOffsetNum += 1;
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
	PSODesc.VS = CD3DX12_SHADER_BYTECODE(Material->shader->GetVertexShader());
	PSODesc.PS = CD3DX12_SHADER_BYTECODE(Material->shader->GetPixelShader());

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

void RenderActor::UpLoadShaderResource(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList,D3D12_SHADER_RESOURCE_VIEW_DESC& SrvDesc)
{
	//获取着色器资源视图起始地址
	CD3DX12_CPU_DESCRIPTOR_HANDLE CbvSrvHandle(GetCbvSrvAvailableHandle());
	//创建纹理资源的堆
	CD3DX12_HEAP_PROPERTIES TexResourceheap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	//创建数据上传堆 CPUGPU都可访问
	CD3DX12_HEAP_PROPERTIES DataUpLoadheap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	size_t TextureNums = Material->textures.size();
	for (int i = 0; i < TextureNums; i++)
	{
		auto& TextureData = Material->textures[i];

		ThrowIfFailed(pDevice->CreateCommittedResource(
			&TexResourceheap,
			D3D12_HEAP_FLAG_NONE,
			&TextureData.texDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&TextureData.Resource)));

		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(TextureData.Resource.Get(), 0, 1);
		CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);


		ThrowIfFailed(pDevice->CreateCommittedResource(
			&DataUpLoadheap,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&TextureData.UploadHeap)));

		//把资源从上传堆拷贝到默认堆，描述该堆作用
		UpdateSubresources(pCommandList, TextureData.Resource.Get(), TextureData.UploadHeap.Get(), 0, 0, 1, &TextureData.texData);
		CD3DX12_RESOURCE_BARRIER Barrier = CD3DX12_RESOURCE_BARRIER::Transition(TextureData.Resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		pCommandList->ResourceBarrier(1, &Barrier);


		SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		SrvDesc.Format = TextureData.texDesc.Format;
		SrvDesc.Texture2D.MipLevels = TextureData.texDesc.MipLevels;
		pDevice->CreateShaderResourceView(TextureData.Resource.Get(), &SrvDesc, CbvSrvHandle);

		CbvSrvHandle.Offset(1, cbvsrvDescriptorSize);
		HandleOffsetNum += 1;
	}
}

void RenderActor::CreateConstantBufferView(ID3D12Device* pDevice)
{
	//数据上传堆
	CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RANGE readRange(0, 0);

	//创建常量缓冲区资源描述符
	constexpr UINT constantBufferSize = CalcConstantBufferByteSize<ObjectConstantBuffer>();
	CD3DX12_RESOURCE_DESC constantResourceDes = CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize);
	//创建常量缓冲区资源
	ThrowIfFailed(pDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &constantResourceDes,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&g_ObjectConstantBuffer)));

	//创建常量缓冲区视图描述符
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = g_ObjectConstantBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = constantBufferSize;


	pDevice->CreateConstantBufferView(&cbvDesc, GetCbvSrvAvailableHandle());
	ThrowIfFailed(g_ObjectConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pObjectCbvDataBegin)));
	HandleOffsetNum++;
	UINT A = HandleOffsetNum;
}

void RenderActor::UpLoadConstantBuffer()
{
	memcpy(pObjectCbvDataBegin.get(), &g_ObjectConstantBufferData, sizeof(g_ObjectConstantBufferData));
}

CD3DX12_CPU_DESCRIPTOR_HANDLE RenderActor::GetCbvSrvAvailableHandle()
{
	
	CD3DX12_CPU_DESCRIPTOR_HANDLE CbvSrvHandle(cbvsrvHeap->GetCPUDescriptorHandleForHeapStart(),HandleOffsetNum,cbvsrvDescriptorSize);
	return CbvSrvHandle;
}





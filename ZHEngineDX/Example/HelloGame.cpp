#include "HelloGame.h"



#define MODEPATH(ModeName) "Content/Mesh/"#ModeName".uasset"
#define MODEASSETPATH(ModeName) "Content/Mesh/"#ModeName".obj"
#define WRITEMODE 0


void DebugMessage(std::wstring strToDisplay)
{
	wcsncpy_s(DebugToDisplay, strToDisplay.c_str(), std::size(strToDisplay));
}

template<typename T>
constexpr UINT CalcConstantBufferByteSize()
{
	const UINT byteSize = sizeof(T);
	return (byteSize + 255) & ~255;
}


HelloGame::HelloGame(UINT width, UINT height, const std::wstring& name):
	GameRHI(width, height, name, DXGI_FORMAT_R8G8B8A8_UNORM),
	g_UniformconstantBufferData{},
	light{Float4(1,1,1,0),FLinearColor(1,1,1,1)}
{

}

void HelloGame::OnInit()
{
	SetWindow();
	LoadPipeline();
	LoadAsset();
}

void HelloGame::OnUpdate(ZHEngineTimer const& timer)
{
	float time = float(timer.GetTotalSeconds());
	//UpdateBackGround();
	UpdateConstantBuffer();
}

void HelloGame::OnRender()
{
	if (g_timer.GetFrameCount() == 0)
	{
		return;
	}
	PopulateCommandList();
	//将命令列表提交给命令队列
	ID3D12CommandList* ppCommandLists[] = { g_commandList.Get() };
	g_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	//显示图像
	ThrowIfFailed(g_swapChain->Present(1, 0));
	MoveToNextFrame();
}

void HelloGame::OnDestroy()
{
	WaitForGPU();
}

void HelloGame::Tick()
{
	g_timer.Tick([&]() {OnUpdate(g_timer); });
	OnRender();
}

void HelloGame::LoadPipeline()
{
#if defined(_DEBUG)
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
		}
	}
#endif 

	//创建D3D资源
	CreateDeviceResources();

	ModeActor = std::make_unique<RenderActor>();
	SkyActor = std::make_unique<RenderActor>();
	ModeActor->Init(GetD3DDevice());
	SkyActor->Init(GetD3DDevice());

	//创建常量缓冲区描述符堆
	CreateConstantBufferDesCribeHeap();
	
	//创建采样器堆描述符
	CreateSamplerDescribeHeap();
	
	//创建窗口资源
	CreateWindowResources();
	
}

void HelloGame::LoadAsset()
{
	//创建根签名，选择合适的版本
	CreateRootSignature();

	//创建着色器资源
	PreperShader();

	//创建PSO
	CreatePSO();

	//创建命令列表，用命令分配器给命令列表分配对象
	ThrowIfFailed(g_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_commandAllocators[g_frameIndex].Get(), nullptr, IID_PPV_ARGS(&g_commandList)));
	
	//ThrowIfFailed(g_commandList->Close());

	//准备渲染对象
	PreperRenderActor();

	//加载纹理数据并创建着色器资源
	UpLoadShaderResource();
	LoadSkyCubeMap();

	

	//关闭命令列表准备渲染
	ThrowIfFailed(g_commandList->Close());
	ID3D12CommandList* ppCommandLists[] = { g_commandList.Get() };
	g_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	//设置围栏
	SetFence();

	

	ModeActor->RecordCommands(GetD3DDevice(),  g_rootSignature.Get(), g_pipelineState.Get(), 
		g_cbvsrvHeap.Get(), g_samplerHeap.Get(), g_cbvsrvDescriptorSize);

	SkyActor->RecordCommands(GetD3DDevice(), g_rootSignature.Get(), g_skyPipelineState.Get(), 
		g_cbvsrvHeap.Get(), g_samplerHeap.Get(), g_cbvsrvDescriptorSize);
	
}

void HelloGame::PopulateCommandList()
{
	ThrowIfFailed(g_commandAllocators[g_frameIndex]->Reset());
	ThrowIfFailed(g_commandList->Reset(g_commandAllocators[g_frameIndex].Get(), nullptr));

	if (g_MSAA)
	{
		//渲染目标转解析状态
		D3D12_RESOURCE_BARRIER resBarrier = CD3DX12_RESOURCE_BARRIER::Transition(g_msaaRenderTarget.Get(), D3D12_RESOURCE_STATE_RESOLVE_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
		g_commandList->ResourceBarrier(1, &resBarrier);

		CD3DX12_CPU_DESCRIPTOR_HANDLE msaartvHandle(g_msaaRTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		CD3DX12_CPU_DESCRIPTOR_HANDLE msaadsvHandle(g_msaaDSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

		//设置渲染目标
		g_commandList->OMSetRenderTargets(1, &msaartvHandle, FALSE, &msaadsvHandle);
		g_commandList->ClearRenderTargetView(msaartvHandle, clearColor, 0, nullptr);
		g_commandList->ClearDepthStencilView(msaadsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}
	else
	{
		//执行资源转换状态(呈现状态转为渲染目标状态，执行绘制操作)
		D3D12_RESOURCE_BARRIER resBarrier = CD3DX12_RESOURCE_BARRIER::Transition(g_renderTargets[g_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		g_commandList->ResourceBarrier(1, &resBarrier);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(g_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), g_frameIndex, g_rtvDescriptorSize);
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(g_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

		//设置渲染目标
		g_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

		//清除渲染目标
		g_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		g_commandList->ClearDepthStencilView(g_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	}

	g_commandList->RSSetViewports(1, &g_viewport);
	g_commandList->RSSetScissorRects(1, &g_scissorRect);

	//设置必要状态
	g_commandList->SetGraphicsRootSignature(g_rootSignature.Get());
	g_commandList->SetPipelineState(g_pipelineState.Get());

	//设置常量缓冲区描述堆，提交到渲染命令
	ID3D12DescriptorHeap* ppHeaps[] = { g_cbvsrvHeap.Get(),g_samplerHeap.Get()};
	g_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	//执行Actor渲染捆绑包
	g_commandList->ExecuteBundle(ModeActor->GetBundle());
	g_commandList->ExecuteBundle(SkyActor->GetBundle());


	if (g_MSAA)
	{
		{
			D3D12_RESOURCE_BARRIER barriers[2] =
			{
				CD3DX12_RESOURCE_BARRIER::Transition(
					g_msaaRenderTarget.Get(),
					D3D12_RESOURCE_STATE_RENDER_TARGET,
					D3D12_RESOURCE_STATE_RESOLVE_SOURCE),
				CD3DX12_RESOURCE_BARRIER::Transition(
					g_renderTargets[g_frameIndex].Get(),
					D3D12_RESOURCE_STATE_PRESENT,
					D3D12_RESOURCE_STATE_RESOLVE_DEST)
			};
			g_commandList->ResourceBarrier(2, barriers);
		}
		
		g_commandList->ResolveSubresource(g_renderTargets[g_frameIndex].Get(), 0, g_msaaRenderTarget.Get(), 0, g_backBufferFormat);
	
	}
	
	//执行资源转换状态(渲染目标状态转为呈现状态)
	D3D12_RESOURCE_BARRIER resBarrier = CD3DX12_RESOURCE_BARRIER::Transition(g_renderTargets[g_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	g_commandList->ResourceBarrier(1, &resBarrier);

	//关闭命令列表，结束命令提交并开始渲染
	ThrowIfFailed(g_commandList->Close());
}



void HelloGame::PreperShader()
{
	UMaterial NormalMat;
	NormalMat.CompileShader(L"Shader/Model.hlsl", "VSMain", "PSMain", EBlendMode::Opaque);
	g_materials.push_back(NormalMat);

	UMaterial SkyMat;
	SkyMat.CompileShader(L"Shader/Sky.hlsl", "VSMain", "PSMain", EBlendMode::Opaque);
	g_skyMaterial = SkyMat;
}

void HelloGame::PreperRenderActor()
{

	//创建顶点Buffer
#if WRITEMODE
	OBJ Mode;
	Mode.Load(MODEASSETPATH(Cube));
	OBJ Sky;
	Sky.Load(MODEASSETPATH(Sky));
#endif

	ModeActor->Mesh->Load(MODEPATH(Cube));
	SkyActor->Mesh->Load(MODEPATH(Sky));
	

	//上传顶点和顶点索引信息
	UpLoadVertexAndIndexToHeap(ModeActor);
	UpLoadVertexAndIndexToHeap(SkyActor);
	//建立并上传数据到常量缓冲区
	UpLoadConstantBuffer();
}



void HelloGame::CreateConstantBufferDesCribeHeap()
{
	LoadTexture();
	
	//创建常量缓冲描述符堆描述
	D3D12_DESCRIPTOR_HEAP_DESC cbvsrvHeapDesc = {};
	cbvsrvHeapDesc.NumDescriptors = g_textures.size()+g_Uniformtextures.size()+2;
	cbvsrvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvsrvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	//创建常量缓存描述符堆
	ThrowIfFailed(g_device->CreateDescriptorHeap(&cbvsrvHeapDesc, IID_PPV_ARGS(&g_cbvsrvHeap)));
	NAME_D3D12_OBJECT(g_cbvsrvHeap);
	g_cbvsrvDescriptorSize = g_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	
	//天空球cbvsrv描述符堆
	cbvsrvHeapDesc.NumDescriptors = 2+g_Uniformtextures.size();
	ThrowIfFailed(g_device->CreateDescriptorHeap(&cbvsrvHeapDesc, IID_PPV_ARGS(&g_UniformcbvsrvHeap)));
	NAME_D3D12_OBJECT(g_UniformcbvsrvHeap);
}

void HelloGame::CreateSamplerDescribeHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
	samplerHeapDesc.NumDescriptors = 1;
	samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(g_device->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&g_samplerHeap)));
}


void HelloGame::CreateRootSignature()
{
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	if (FAILED(g_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}
	//创建对根参数的描述和根参数
	CD3DX12_DESCRIPTOR_RANGE1 skyrange;
	CD3DX12_DESCRIPTOR_RANGE1 EnvBRDFrange;
	CD3DX12_DESCRIPTOR_RANGE1 samplerRanges;
	CD3DX12_DESCRIPTOR_RANGE1 cbvRanges;
	CD3DX12_DESCRIPTOR_RANGE1 normalSrvRanges[2];
	

	//指定该根参数为常量缓冲区视图
	samplerRanges.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);
	cbvRanges.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	normalSrvRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	normalSrvRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	EnvBRDFrange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	skyrange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

	std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters(6);
	
	rootParameters[0].InitAsDescriptorTable(1, &samplerRanges, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[1].InitAsDescriptorTable(1, &cbvRanges, D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[2].InitAsDescriptorTable(1, &normalSrvRanges[0], D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[3].InitAsDescriptorTable(1, &normalSrvRanges[1], D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[4].InitAsDescriptorTable(1, &EnvBRDFrange, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[5].InitAsDescriptorTable(1, &skyrange, D3D12_SHADER_VISIBILITY_PIXEL);
	

	//定义根签名属性
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	//创建动态采样器
	
	D3D12_SAMPLER_DESC sampler0 = CreateSamplerDesCribe(0);
	g_device->CreateSampler(&sampler0,g_samplerHeap->GetCPUDescriptorHandleForHeapStart());
	

	//创建根签名描述符
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_1(static_cast<UINT>(rootParameters.size()), rootParameters.data(), 0, nullptr, rootSignatureFlags);

	
	//创建根签名
	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
	ThrowIfFailed(g_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&g_rootSignature)));
	NAME_D3D12_OBJECT(g_rootSignature);
	
}

D3D12_SAMPLER_DESC HelloGame::CreateSamplerDesCribe(UINT index)
{
	D3D12_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_ANISOTROPIC;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.MipLODBias = 0;
	sampler.MaxAnisotropy = 8;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	//sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	//sampler.ShaderRegister = index;
	//sampler.RegisterSpace = 0;
	//sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	return sampler;
}
void HelloGame::CreateShader(ComPtr<ID3DBlob>& vertexShader, ComPtr<ID3DBlob>& pixelShader, std::wstring VSFileName, std::wstring PSFileName)
{
	UINT compileFlags = 0;

#if defined(_DEBUG)
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ThrowIfFailed(D3DCompileFromFile(VSFileName.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
	ThrowIfFailed(D3DCompileFromFile(PSFileName.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));
}

void HelloGame::CreatePSO()
{
	//创建管线状态对象PSO，绑定渲染资源
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	//PSO描述
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

	if (g_materials[0].GetMateriBlendMode() == EBlendMode::Opaque)
	{
		psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		psoDesc.pRootSignature = g_rootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(g_materials[0].shader->GetVertexShader());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(g_materials[0].shader->GetPixelShader());
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = g_backBufferFormat;
		psoDesc.SampleDesc.Count = g_MSAA ? 4 : 1;
		psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

		//创建PSO
		ThrowIfFailed(g_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&g_pipelineState)));
		NAME_D3D12_OBJECT(g_pipelineState);
	}
	
	D3D12_GRAPHICS_PIPELINE_STATE_DESC SkypsoDesc = psoDesc;
	SkypsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	SkypsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	SkypsoDesc.pRootSignature = g_rootSignature.Get();
	SkypsoDesc.VS = CD3DX12_SHADER_BYTECODE(g_skyMaterial.shader->GetVertexShader());
	SkypsoDesc.PS = CD3DX12_SHADER_BYTECODE(g_skyMaterial.shader->GetPixelShader());
	ThrowIfFailed(g_device->CreateGraphicsPipelineState(&SkypsoDesc, IID_PPV_ARGS(&g_skyPipelineState)));
}

void HelloGame::UpLoadVertexAndIndexToHeap(const std::unique_ptr<RenderActor>& Actor) const
{
	/*ComPtr<ID3D12Resource> vertexBufferUpLoadHeap;
	ComPtr<ID3D12Resource> indexBufferUpLoadHeap;*/

	const UINT vertexBufferSize = static_cast<UINT>(Actor->Mesh->GetVerticesByteSize());
	const UINT indexBufferSize = static_cast<UINT>(Actor->Mesh->GetIndicesByteSize());

	//资源描述符
	const CD3DX12_RESOURCE_DESC vertexResourceDes = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
	const CD3DX12_RESOURCE_DESC indexResourceDes = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);

	//CD3DX12_HEAP_PROPERTIES defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	const CD3DX12_HEAP_PROPERTIES uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	
	//资源创建
	/*ThrowIfFailed(g_device->CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &vertexResourceDes,
		D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&Actor->g_vertexBuffer)));*/

	ThrowIfFailed(g_device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &vertexResourceDes,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&Actor->g_vertexBuffer)));

	NAME_D3D12_OBJECT(Actor->g_vertexBuffer);

	ThrowIfFailed(g_device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &indexResourceDes,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&Actor->g_indexBuffer)));

	const CD3DX12_RANGE readRange(0, 0);


	//复制顶点资源数据到GPUBuffer
	UINT8* pVertexDataBegin;

	/*D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = pVertexDataBegin;
	vertexData.RowPitch = vertexBufferSize;
	vertexData.SlicePitch = vertexData.RowPitch;

	CD3DX12_RESOURCE_BARRIER vertexBarrier = CD3DX12_RESOURCE_BARRIER::Transition(Actor->g_vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	UpdateSubresources<1>(g_commandList.Get(), Actor->g_vertexBuffer.Get(), vertexBufferUpLoadHeap.Get(), 0, 0, 1, &vertexData);
	g_commandList->ResourceBarrier(1, &vertexBarrier);*/


	ThrowIfFailed(Actor->g_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
	memcpy(pVertexDataBegin, Actor->Mesh->GetVerticesData(), vertexBufferSize);
	Actor->g_vertexBuffer->Unmap(0, nullptr);

	ThrowIfFailed(Actor->g_indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
	memcpy(pVertexDataBegin, Actor->Mesh->GetIndicesData(), indexBufferSize);
	Actor->g_indexBuffer->Unmap(0, nullptr);

	//初始化资源缓冲区视图
	Actor->g_vertexBufferView.BufferLocation = Actor->g_vertexBuffer->GetGPUVirtualAddress();
	Actor->g_vertexBufferView.StrideInBytes = sizeof(Vertex);
	Actor->g_vertexBufferView.SizeInBytes = vertexBufferSize;

	Actor->g_indexBufferView.BufferLocation = Actor->g_indexBuffer->GetGPUVirtualAddress();
	Actor->g_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	Actor->g_indexBufferView.SizeInBytes = indexBufferSize;
}


void HelloGame::UpLoadConstantBuffer()
{
	//数据上传堆
	CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RANGE readRange(0, 0);

	//创建常量缓冲区资源描述符
	constexpr UINT constantBufferSize = CalcConstantBufferByteSize<SceneConstantBuffer>();
	CD3DX12_RESOURCE_DESC constantResourceDes = CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize);
	//创建常量缓冲区资源
	ThrowIfFailed(g_device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &constantResourceDes,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&g_UniformconstantBuffer)));

	//创建常量缓冲区视图描述符
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = g_UniformconstantBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = constantBufferSize;

	//创建常量缓冲区视图
	g_device->CreateConstantBufferView(&cbvDesc, g_cbvsrvHeap->GetCPUDescriptorHandleForHeapStart());
	g_device->CreateConstantBufferView(&cbvDesc, g_UniformcbvsrvHeap->GetCPUDescriptorHandleForHeapStart());
	//复制常量缓冲区数据到GPU
	ThrowIfFailed(g_UniformconstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&g_pCbvDataBegin)));
	memcpy(g_pCbvDataBegin, &g_UniformconstantBufferData, sizeof(g_UniformconstantBufferData));
}

void HelloGame::UpLoadShaderResource()
{
	
	
	//创建纹理资源的堆
	CD3DX12_HEAP_PROPERTIES TexResourceheap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	
	//创建数据上传堆 CPUGPU都可访问
	CD3DX12_HEAP_PROPERTIES DataUpLoadheap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	

	//创建着色器资源视图描述
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	//获取着色器资源视图起始地址
	CD3DX12_CPU_DESCRIPTOR_HANDLE cbvsrvHandle(g_cbvsrvHeap->GetCPUDescriptorHandleForHeapStart());
	CD3DX12_CPU_DESCRIPTOR_HANDLE UniformcbvsrvHandle(g_UniformcbvsrvHeap->GetCPUDescriptorHandleForHeapStart());
	size_t TextureNums = g_textures.size();
	for (int i = 0; i < TextureNums; i++)
	{
		ThrowIfFailed(g_device->CreateCommittedResource(
			&TexResourceheap,
			D3D12_HEAP_FLAG_NONE,
			&g_textures[i].texDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&g_textures[i].Resource)));

		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(g_textures[i].Resource.Get(), 0, 1);
		CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);


		ThrowIfFailed(g_device->CreateCommittedResource(
			&DataUpLoadheap,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&g_textures[i].UploadHeap)));

		//把资源从上传堆拷贝到默认堆，描述该堆作用
		UpdateSubresources(g_commandList.Get(), g_textures[i].Resource.Get(), g_textures[i].UploadHeap.Get(), 0, 0, 1, &g_textures[i].texData);
		CD3DX12_RESOURCE_BARRIER Barrier = CD3DX12_RESOURCE_BARRIER::Transition(g_textures[i].Resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		g_commandList->ResourceBarrier(1, &Barrier);

		
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Format = g_textures[i].texDesc.Format;
		srvDesc.Texture2D.MipLevels = g_textures[i].texDesc.MipLevels;
		cbvsrvHandle.Offset(1, g_cbvsrvDescriptorSize);
		g_device->CreateShaderResourceView(g_textures[i].Resource.Get(), &srvDesc, cbvsrvHandle);
	}

	size_t UniformTextureNums = g_Uniformtextures.size();
	
	for (int i = 0; i < UniformTextureNums; i++)
	{
		ThrowIfFailed(g_device->CreateCommittedResource(
			&TexResourceheap,
			D3D12_HEAP_FLAG_NONE,
			&g_Uniformtextures[i].texDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&g_Uniformtextures[i].Resource)));

		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(g_Uniformtextures[i].Resource.Get(), 0, 1);
		CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);


		ThrowIfFailed(g_device->CreateCommittedResource(
			&DataUpLoadheap,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&g_Uniformtextures[i].UploadHeap)));

		//把资源从上传堆拷贝到默认堆，描述该堆作用
		UpdateSubresources(g_commandList.Get(), g_Uniformtextures[i].Resource.Get(), g_Uniformtextures[i].UploadHeap.Get(), 0, 0, 1, &g_Uniformtextures[i].texData);
		CD3DX12_RESOURCE_BARRIER Barrier = CD3DX12_RESOURCE_BARRIER::Transition(g_Uniformtextures[i].Resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		g_commandList->ResourceBarrier(1, &Barrier);

		
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Format = g_Uniformtextures[i].texDesc.Format;
		srvDesc.Texture2D.MipLevels = g_Uniformtextures[i].texDesc.MipLevels;
		cbvsrvHandle.Offset(1, g_cbvsrvDescriptorSize);
		g_device->CreateShaderResourceView(g_Uniformtextures[i].Resource.Get(), &srvDesc, cbvsrvHandle);
	}
}

void HelloGame::UpdateLight()
{
	FVector4 lightDirV = ZMath::Normalize4(FVector4{1,0,1,0});
	constexpr  float angle = 0.5f;
	lightangle += angle;
	const float angleInRadians = ZMath::AngleToRadians(lightangle);
	FMatrix4x4 rotationMatrix = ZMath::MatrixRotateY(angleInRadians);
	lightDirV = -ZMath::Normalize4(ZMath::Transform(lightDirV,rotationMatrix));
	const Float4 lightDir = ZMath::FVector4ToFloat4(lightDirV);
	light.direction = lightDir;
	g_UniformconstantBufferData.lightColor = light.color;
	g_UniformconstantBufferData.lightDirection = light.direction;
	
}

void HelloGame::UpdateConstantBuffer()
{
	UpdateMVP();
	UpdateLight();
	//GPU复制数据
	memcpy(g_pCbvDataBegin, &g_UniformconstantBufferData, sizeof(g_UniformconstantBufferData));
}

void HelloGame::UpdateMVP()
{
	float x = g_camera.g_Radius * sinf(g_camera.g_Phi) * cosf(g_camera.g_Theta);
	float z = g_camera.g_Radius * sinf(g_camera.g_Phi) * sinf(g_camera.g_Theta);
	float y = g_camera.g_Radius * cosf(g_camera.g_Phi);

	FVector4 pos = ZMath::MakeFvector4(x, y, z, 1.0f);
	FVector4 target = ZMath::MakeFvector4(0.0f, 0.0f, 0.0f, 1.0f);
	FVector4 up = ZMath::MakeFvector4(0.0f, 1.0f, 0.0f, 0.0f);

	FMatrix4x4 v = ZMath::LookAt(pos, target, up);

	FMatrix4x4 m = ZMath::MatrixIdentity();
	FMatrix4x4 p = ZMath::MatrixFov(PIDIV4, AspectRatio(), 1.0f, 1000.0f);
	FMatrix4x4 VP = v * p;
	FMatrix4x4 MVP = m * v * p;

	ZMath::MaterixToFloat4x4(&g_UniformconstantBufferData.ObjectToWorld, m);
	ZMath::MaterixToFloat4x4(&g_UniformconstantBufferData.VP, VP);
	ZMath::MaterixToFloat4x4(&g_UniformconstantBufferData.MVP, MVP);
	g_UniformconstantBufferData.cameraPosition = { x,y,z,1 };
	
}

void HelloGame::LoadTexture()
{
	std::vector<LPCWSTR> TextureFiles;
	TextureFiles.push_back(L"Content/Tex/Wall_00_BaseColorAO.png");
	TextureFiles.push_back(L"Content/Tex/Wall_00_NormalR.png");
	
	size_t TextureNums = TextureFiles.size();
	for (size_t i = 0; i < TextureNums; i++)
	{
		UTexture Temptex = UTexture();
		Temptex.Data = std::make_shared<BYTE>();
		Temptex.Filename = TextureFiles[i];

		Temptex.texSize = Texture::LoadImageDataFromFile(Temptex.Data, Temptex.texDesc, Temptex.Filename, Temptex.texBytesPerRow);
		Temptex.GenerateTextureData();
		g_textures.push_back(Temptex);
	}

	std::vector<LPCWSTR> UniformTextureFiles;
	UniformTextureFiles.push_back(L"Content/Tex/BRDFLut.png");
	size_t UniformTextureNums = UniformTextureFiles.size();
	for (size_t i = 0; i < UniformTextureNums; i++)
	{
		UTexture Temptex = UTexture();
		Temptex.Data = std::make_shared<BYTE>();
		Temptex.Filename = UniformTextureFiles[i];

		Temptex.texSize = Texture::LoadImageDataFromFile(Temptex.Data, Temptex.texDesc, Temptex.Filename, Temptex.texBytesPerRow);
		Temptex.GenerateTextureData();
		g_Uniformtextures.push_back(Temptex);
	}
}

void HelloGame::LoadSkyCubeMap()
{
	std::vector<LPCWSTR> UniformTextureFiles;
	UniformTextureFiles.push_back(L"Content/Tex/SkyCubeMap_03/SkyPX.png");
	UniformTextureFiles.push_back(L"Content/Tex/SkyCubeMap_03/SkyNX.png");
	UniformTextureFiles.push_back(L"Content/Tex/SkyCubeMap_03/SkyPY.png");
	UniformTextureFiles.push_back(L"Content/Tex/SkyCubeMap_03/SkyNY.png");
	UniformTextureFiles.push_back(L"Content/Tex/SkyCubeMap_03/SkyPZ.png");
	UniformTextureFiles.push_back(L"Content/Tex/SkyCubeMap_03/SkyNZ.png");
	size_t UniformTextureNums = UniformTextureFiles.size();
	std::vector<UTexture> skytextures;
	for (int i = 0; i < UniformTextureNums; i++)
	{
		UTexture Temptex = UTexture();
		Temptex.Data = std::make_shared<BYTE>();
		Temptex.Filename = UniformTextureFiles[i];

		Temptex.texSize = Texture::LoadImageDataFromFile(Temptex.Data, Temptex.texDesc, Temptex.Filename, Temptex.texBytesPerRow);
		Temptex.GenerateTextureData();
		skytextures.push_back(Temptex);
	}

	D3D12_RESOURCE_DESC skyResourceDesc = {};
	skyResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	skyResourceDesc.MipLevels = 1;
	skyResourceDesc.Format = skytextures[0].texDesc.Format;
	skyResourceDesc.Width = skytextures[0].texDesc.Width;
	skyResourceDesc.Height = skytextures[0].texDesc.Height;
	skyResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	skyResourceDesc.DepthOrArraySize = 6;
	skyResourceDesc.SampleDesc.Count = 1;
	skyResourceDesc.SampleDesc.Quality = 0;


	//创建纹理资源的堆
	CD3DX12_HEAP_PROPERTIES TexResourceheap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	//创建数据上传堆 CPUGPU都可访问
	CD3DX12_HEAP_PROPERTIES DataUpLoadheap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);



	ThrowIfFailed(g_device->CreateCommittedResource(
		&TexResourceheap,
		D3D12_HEAP_FLAG_NONE,
		&skyResourceDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&g_SkyCubeMap.Resource)));

	D3D12_RESOURCE_DESC   skyCubeMapResourceDesc = g_SkyCubeMap.Resource->GetDesc();
	const UINT SubresourcesNum = 6;
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT SkyCubeMapLayout[6] = {};
	UINT RowNum[6] = {};
	UINT64 RowNumByteSize[6] = {};
	UINT64 TotalByte = 0;

	g_device->GetCopyableFootprints(&skyCubeMapResourceDesc,0,SubresourcesNum,0,SkyCubeMapLayout,RowNum,RowNumByteSize,&TotalByte);

	g_ResourceBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	g_ResourceBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	g_ResourceBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	g_ResourceBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	g_ResourceBufferDesc.Width = TotalByte;
	g_ResourceBufferDesc.Height = 1;
	g_ResourceBufferDesc.DepthOrArraySize = 1;
	g_ResourceBufferDesc.MipLevels = 1;
	g_ResourceBufferDesc.SampleDesc.Count = 1;
	g_ResourceBufferDesc.SampleDesc.Quality = 0;

	ThrowIfFailed(g_device->CreateCommittedResource(
		&DataUpLoadheap,
		D3D12_HEAP_FLAG_NONE,
		&g_ResourceBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&g_SkyCubeMap.UploadHeap)));

	BYTE* pData = nullptr;
	ThrowIfFailed(g_SkyCubeMap.UploadHeap->Map(0, nullptr, reinterpret_cast<void**>(&pData)));

	for (UINT i = 0; i < SubresourcesNum; i++)
	{
		if (RowNumByteSize[i] > (SIZE_T)-1)
		{
			ThrowIfFailed(E_FAIL);
		}

		D3D12_MEMCPY_DEST CopyDestData = { pData + SkyCubeMapLayout[i].Offset
			, SkyCubeMapLayout[i].Footprint.RowPitch
			, SkyCubeMapLayout[i].Footprint.RowPitch * RowNum[i]
		};

		for (UINT z = 0; z < SkyCubeMapLayout[i].Footprint.Depth; ++z)
		{// Mipmap
			BYTE* pDestSlice = reinterpret_cast<BYTE*>(CopyDestData.pData) + CopyDestData.SlicePitch * z;
			const BYTE* pSrcSlice = reinterpret_cast<const BYTE*>(skytextures[i].Data.get());

			for (UINT y = 0; y < RowNum[i]; ++y)
			{// Rows
				memcpy(pDestSlice + CopyDestData.RowPitch * y,
					pSrcSlice + skytextures[i].texBytesPerRow * y,
					(SIZE_T)RowNumByteSize[i]);
			}
		}
	}

	g_SkyCubeMap.UploadHeap->Unmap(0,nullptr);

	for (UINT i = 0; i < SubresourcesNum; i++)
	{
		D3D12_TEXTURE_COPY_LOCATION DstCopyLocation = {};
		DstCopyLocation.pResource = g_SkyCubeMap.Resource.Get();
		DstCopyLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		DstCopyLocation.SubresourceIndex = i;

		D3D12_TEXTURE_COPY_LOCATION SrcCopyLocation = {};
		SrcCopyLocation.pResource = g_SkyCubeMap.UploadHeap.Get();
		SrcCopyLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		SrcCopyLocation.PlacedFootprint = SkyCubeMapLayout[i];

		g_commandList->CopyTextureRegion(&DstCopyLocation, 0, 0, 0, &SrcCopyLocation, nullptr);
	}

	D3D12_RESOURCE_BARRIER skyBarrier = {};
	skyBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	skyBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	skyBarrier.Transition.pResource = g_SkyCubeMap.Resource.Get();
	skyBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	skyBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	skyBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	g_commandList->ResourceBarrier(1, &skyBarrier);

	//创建着色器资源视图描述
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = skyCubeMapResourceDesc.Format;
	srvDesc.TextureCube.MipLevels = skyCubeMapResourceDesc.MipLevels;
	 
	UINT offset = static_cast<UINT>(g_Uniformtextures.size()+g_textures.size()+1);

	//获取着色器资源视图起始地址
	CD3DX12_CPU_DESCRIPTOR_HANDLE cbvsrvHandle(g_cbvsrvHeap->GetCPUDescriptorHandleForHeapStart());
	cbvsrvHandle.ptr += (offset * g_cbvsrvDescriptorSize);
	//cbvsrvHandle.Offset(offset, g_cbvsrvDescriptorSize);

	g_device->CreateShaderResourceView(g_SkyCubeMap.Resource.Get(), &srvDesc, cbvsrvHandle);
}


void HelloGame::UpdateBackGround()
{
	if (clearColor[0] <= 1.0f && isRAdd)
	{
		clearColor[0] += 0.001f;
		isRAdd = true;
	}
	else
	{
		clearColor[0] -= 0.002f;
		clearColor[0] <= 0 ? isRAdd = true : isRAdd = false;

	}

	if (clearColor[1] <= 1.0f && isGAdd)
	{
		clearColor[1] += 0.002f;
		isGAdd = true;
	}
	else
	{
		clearColor[1] -= 0.001f;
		clearColor[1] <= 0 ? isGAdd = true : isGAdd = false;

	}

	if (clearColor[2] <= 1.0f && isBAdd)
	{
		clearColor[2] += 0.001f;
		isBAdd = true;
	}
	else
	{
		clearColor[2] -= 0.001f;
		clearColor[2] <= 0 ? isBAdd = true : isBAdd = false;

	}

	clearColor[3] = 1.0f;
}

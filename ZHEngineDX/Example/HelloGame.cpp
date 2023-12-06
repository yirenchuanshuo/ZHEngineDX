#include "HelloGame.h"

#define MODEPATH "Content/Mesh/Mode.uasset"
#define MODEASSETPATH "Content/Mesh/Cube.obj"
#define WRITEMODE 0


void DebugMessage(std::wstring strToDisplay)
{
	wcsncpy_s(DebugToDisplay, strToDisplay.c_str(), sizeof(DebugToDisplay) / sizeof(DebugToDisplay[0]));
}

template<typename T>
constexpr UINT CalcConstantBufferByteSize()
{
	UINT byteSize = sizeof(T);
	return (byteSize + 255) & ~255;
}


HelloGame::HelloGame(UINT width, UINT height, std::wstring name):
	GameRHI(width, height, name),
	g_constantBufferData{},
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
	UpdateBackGround();
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
	//CloseHandle(g_fenceEvent.Get());
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

	//创建窗口资源
	CreateWindowResources();
	
	//创建常量缓冲区描述符堆
	CreateConstantBufferDesCribeHeap();
	
    
}

void HelloGame::LoadAsset()
{
	//创建根签名，选择合适的版本
	CreateRootSignature();

	
	//创建着色器资源
	ComPtr<ID3DBlob> vertexShader;
	ComPtr<ID3DBlob> pixelShader;
	UINT compileFlags = 0;

#if defined(_DEBUG)
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	
	ThrowIfFailed(D3DCompileFromFile(std::wstring(L"Shader/Triangles.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
	ThrowIfFailed(D3DCompileFromFile(std::wstring(L"Shader/Triangles.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));
	

	//创建PSO
	CreatePSO(vertexShader,pixelShader);


	//创建命令列表，用命令分配器给命令列表分配对象
	ThrowIfFailed(g_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_commandAllocator[g_frameIndex].Get(), g_pipelineState.Get(), IID_PPV_ARGS(&g_commandList)));

	
	//ThrowIfFailed(g_commandList->Close());

	//创建顶点Buffer
#if WRITEMODE
	OBJ Mode;
	Mode.Load(MODEASSETPATH);
#endif

	Mesh.Load(MODEPATH);

	UINT ModeVertexSize = (UINT)Mesh.vertices.size() * sizeof(Vertex);
	UINT ModeIndexSize = (UINT)Mesh.indices.size() * sizeof(UINT);

	//数据上传堆
	CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RANGE readRange(0, 0);

	//上传顶点和顶点索引信息
	UpLoadVertexAndIndexToHeap(heapProperties,readRange,ModeVertexSize, ModeIndexSize);
	
	//建立并上传数据到常量缓冲区
	UpLoadConstantBuffer(heapProperties,readRange);

	//加载纹理数据并创建着色器资源
	UpLoadShaderResource();

	//关闭命令列表准备渲染
	ThrowIfFailed(g_commandList->Close());
	ID3D12CommandList* ppCommandLists[] = { g_commandList.Get() };
	g_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	//设置围栏
	SetFence();
}

void HelloGame::PopulateCommandList()
{
	ThrowIfFailed(g_commandAllocator[g_frameIndex]->Reset());
	ThrowIfFailed(g_commandList->Reset(g_commandAllocator[g_frameIndex].Get(), g_pipelineState.Get()));

	//设置必要状态
	g_commandList->SetGraphicsRootSignature(g_rootSignature.Get());

	//设置常量缓冲区描述堆，提交到渲染命令
	ID3D12DescriptorHeap* ppHeaps[] = { g_cbvsrvHeap.Get() };
	g_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	//设置根描述符表，上传参数
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandle = g_cbvsrvHeap->GetGPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandleForSecondDescriptor = { gpuDescriptorHandle .ptr+ g_cbvsrvDescriptorSize };
	g_commandList->SetGraphicsRootDescriptorTable(0, gpuDescriptorHandle);
	g_commandList->SetGraphicsRootDescriptorTable(1, gpuDescriptorHandleForSecondDescriptor);
	g_commandList->RSSetViewports(1, &g_viewport);
	g_commandList->RSSetScissorRects(1, &g_scissorRect);

	//执行资源转换状态(呈现状态转为渲染目标状态，执行绘制操作)
	D3D12_RESOURCE_BARRIER resBarrier = CD3DX12_RESOURCE_BARRIER::Transition(g_renderTargets[g_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	g_commandList->ResourceBarrier(1, &resBarrier);
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(g_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), g_frameIndex, g_rtvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(g_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	//设置渲染目标
	g_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	//清除渲染目标
	float BackGroundColor[4] = {0,0,0,1};
	g_commandList->ClearRenderTargetView(rtvHandle, BackGroundColor, 0, nullptr);
	g_commandList->ClearDepthStencilView(g_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	
	//图元拓扑模式
	g_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//顶点装配
	g_commandList->IASetVertexBuffers(0, 1, &g_vertexBufferView);
	g_commandList->IASetIndexBuffer(&g_indexBufferView);

	//画图
	g_commandList->DrawIndexedInstanced((UINT)Mesh.indices.size(), 1, 0, 0, 0);

	//执行资源转换状态(渲染目标状态转为呈现状态)
	resBarrier = CD3DX12_RESOURCE_BARRIER::Transition(g_renderTargets[g_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	g_commandList->ResourceBarrier(1, &resBarrier);

	//关闭命令列表，结束命令提交并开始渲染
	ThrowIfFailed(g_commandList->Close());
}






void HelloGame::CreateConstantBufferDesCribeHeap()
{
	//创建常量缓冲描述符堆描述
	D3D12_DESCRIPTOR_HEAP_DESC cbvsrvHeapDesc = {};
	cbvsrvHeapDesc.NumDescriptors = 3;
	cbvsrvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvsrvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	//创建常量缓存描述符堆
	ThrowIfFailed(g_device->CreateDescriptorHeap(&cbvsrvHeapDesc, IID_PPV_ARGS(&g_cbvsrvHeap)));
	g_cbvsrvDescriptorSize = g_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
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
	CD3DX12_DESCRIPTOR_RANGE1 ranges[3];
	CD3DX12_ROOT_PARAMETER1 rootParameters[2];

	//指定该根参数为常量缓冲区视图
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);
	ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	rootParameters[1].InitAsDescriptorTable(2, &ranges[1], D3D12_SHADER_VISIBILITY_ALL);
	
	

	//定义根签名属性
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	//创建静态采样器
	
	D3D12_STATIC_SAMPLER_DESC sampler0 = CreateSamplerDesCribe(0);
	D3D12_STATIC_SAMPLER_DESC sampler1 = CreateSamplerDesCribe(1);
	std::vector<D3D12_STATIC_SAMPLER_DESC>samplers;
	samplers.push_back(sampler0);
	samplers.push_back(sampler1);

	//创建根签名描述符
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, static_cast<UINT>(samplers.size()), samplers.data(), rootSignatureFlags);

	
	//创建根签名
	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
	ThrowIfFailed(g_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&g_rootSignature)));
	
}

D3D12_STATIC_SAMPLER_DESC HelloGame::CreateSamplerDesCribe(UINT index)
{
	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.MipLODBias = 0;
	sampler.MaxAnisotropy = 0;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = index;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	return sampler;
}

void HelloGame::CreatePSO(ComPtr<ID3DBlob>& vertexShader, ComPtr<ID3DBlob>& pixelShader)
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
	psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	psoDesc.pRootSignature = g_rootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	//创建PSO
	ThrowIfFailed(g_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&g_pipelineState)));
}

void HelloGame::UpLoadVertexAndIndexToHeap(CD3DX12_HEAP_PROPERTIES& heapProperties, CD3DX12_RANGE& readRange,const UINT vertexBufferSize, const UINT indexBufferSize)
{
	
	//资源描述符
	CD3DX12_RESOURCE_DESC vertexResourceDes = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
	CD3DX12_RESOURCE_DESC indexResourceDes = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);


	//资源创建
	ThrowIfFailed(g_device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &vertexResourceDes,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&g_vertexBuffer)));

	ThrowIfFailed(g_device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &indexResourceDes,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&g_indexBuffer)));

	//复制顶点资源数据到GPUBuffer
	UINT8* pVertexDataBegin;
	ThrowIfFailed(g_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
	memcpy(pVertexDataBegin, Mesh.vertices.data(), vertexBufferSize);
	g_vertexBuffer->Unmap(0, nullptr);

	ThrowIfFailed(g_indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
	memcpy(pVertexDataBegin, Mesh.indices.data(), indexBufferSize);
	g_indexBuffer->Unmap(0, nullptr);

	//初始化资源缓冲区视图
	g_vertexBufferView.BufferLocation = g_vertexBuffer->GetGPUVirtualAddress();
	g_vertexBufferView.StrideInBytes = sizeof(Vertex);
	g_vertexBufferView.SizeInBytes = vertexBufferSize;

	g_indexBufferView.BufferLocation = g_indexBuffer->GetGPUVirtualAddress();
	g_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	g_indexBufferView.SizeInBytes = indexBufferSize;
}


void HelloGame::UpLoadConstantBuffer(CD3DX12_HEAP_PROPERTIES& heapProperties, CD3DX12_RANGE& readRange)
{
	//创建常量缓冲区资源描述符
	constexpr UINT constantBufferSize = CalcConstantBufferByteSize<SceneConstantBuffer>();
	CD3DX12_RESOURCE_DESC constantResourceDes = CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize);
	//创建常量缓冲区资源
	ThrowIfFailed(g_device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &constantResourceDes,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&g_constantBuffer)));

	//创建常量缓冲区视图描述符
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = g_constantBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = constantBufferSize;

	//创建常量缓冲区视图
	g_device->CreateConstantBufferView(&cbvDesc, g_cbvsrvHeap->GetCPUDescriptorHandleForHeapStart());
	//复制常量缓冲区数据到GPU
	ThrowIfFailed(g_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&g_pCbvDataBegin)));
	memcpy(g_pCbvDataBegin, &g_constantBufferData, sizeof(g_constantBufferData));
}

void HelloGame::UpLoadShaderResource()
{
	
	LoadTexture();
	
	//创建纹理资源的堆
	CD3DX12_HEAP_PROPERTIES TexResourceheap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	//创建数据上传堆 CPUGPU都可访问
	CD3DX12_HEAP_PROPERTIES DataUpLoadheap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	//创建着色器资源视图描述
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	//获取着色器资源视图起始地址
	CD3DX12_CPU_DESCRIPTOR_HANDLE cbvsrvHandle(g_cbvsrvHeap->GetCPUDescriptorHandleForHeapStart());
	
	for (auto& Texture : g_textures)
	{
		ThrowIfFailed(g_device->CreateCommittedResource(
			&TexResourceheap,
			D3D12_HEAP_FLAG_NONE,
			&Texture.texDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&Texture.Resource)));

		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(Texture.Resource.Get(), 0, 1);
		CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);


		ThrowIfFailed(g_device->CreateCommittedResource(
			&DataUpLoadheap,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&Texture.UploadHeap)));

		//把资源从上传堆拷贝到默认堆，描述该堆作用
		UpdateSubresources(g_commandList.Get(), Texture.Resource.Get(), Texture.UploadHeap.Get(), 0, 0, 1, &Texture.texData);
		CD3DX12_RESOURCE_BARRIER Barrier = CD3DX12_RESOURCE_BARRIER::Transition(Texture.Resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		g_commandList->ResourceBarrier(1, &Barrier);

		srvDesc.Format = Texture.texDesc.Format;
		cbvsrvHandle.Offset(1, g_cbvsrvDescriptorSize);
		g_device->CreateShaderResourceView(Texture.Resource.Get(), &srvDesc, cbvsrvHandle);
	}
	
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

void HelloGame::UpdateLight()
{
	FVector4 lightDirV = ZMath::Normalize4(FVector4{1,1,0,0});
	float angle = 0.5f;
	lightangle += angle;
	float angleInRadians = ZMath::AngleToRadians(lightangle);
	FMatrix4x4 rotationMatrix = ZMath::MatrixRotateY(angleInRadians);
	lightDirV = -ZMath::Normalize4(ZMath::Transform(lightDirV,rotationMatrix));
	Float4 lightDir = ZMath::FVector4ToFloat4(lightDirV);
	light.direction = lightDir;
	g_constantBufferData.lightColor = light.color;
	g_constantBufferData.lightDirection = light.direction;
	
}

void HelloGame::UpdateConstantBuffer()
{

	UpdateMVP();
	UpdateLight();
	//GPU复制数据
	memcpy(g_pCbvDataBegin, &g_constantBufferData, sizeof(g_constantBufferData));
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
	FMatrix4x4 MVP = m * v * p;

	ZMath::MaterixToFloat4x4(&g_constantBufferData.ObjectToWorld, m);
	ZMath::MaterixToFloat4x4(&g_constantBufferData.MVP, MVP);
	g_constantBufferData.cameraPosition = { x,y,z,1 };
	
}

void HelloGame::LoadTexture()
{
	std::vector<LPCWSTR> TextureFiles;
	TextureFiles.push_back(L"Content/Tex/Wall_00_BaseColorAO.png");
	TextureFiles.push_back(L"Content/Tex/Wall_00_NormalR.png");
	size_t n = TextureFiles.size();
	for (size_t i = 0; i < n; i++)
	{
		UTexture Temptex;
		Temptex.Data = std::make_shared<BYTE>();
		Temptex.Filename = TextureFiles[i];
		Temptex.texSize = LoadImageDataFromFile(Temptex.Data, Temptex.texDesc, Temptex.Filename, Temptex.texBytesPerRow);
		Temptex.GenerateTextureData();
		g_textures.push_back(Temptex);
	}

}

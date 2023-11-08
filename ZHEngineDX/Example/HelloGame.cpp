#include "HelloGame.h"


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
	Game(width, height, name),
	g_frameIndex(0),
	g_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
	g_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
	g_rtvDescriptorSize(0),
	g_constantBufferData{},
	light{Float3(1,1,1),FLinearColor(1,1,1,1)}
{

}

void HelloGame::OnInit()
{
	LoadPipeline();
	LoadAsset();
}

void HelloGame::OnUpdate()
{
	UpdateBackGround();
	UpdateConstantBuffer();
}

void HelloGame::OnRender()
{
	PopulateCommandList();
	//�������б��ύ���������
	ID3D12CommandList* ppCommandLists[] = { g_commandList.Get() };
	g_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	//��ʾͼ��
	ThrowIfFailed(g_swapChain->Present(1, 0));
	WaitForPreviousFrame();
}

void HelloGame::OnDestroy()
{
	WaitForPreviousFrame();
	CloseHandle(g_fenceEvent);
	delete g_texData;
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
	

	//��������
	ComPtr<IDXGIFactory6> gDxgiFactory;

	//����GPU��������
	CreateGPUElement(gDxgiFactory);

	//�����������
	CreateCommandQueue();

	//������������ָ������
	ComPtr<IDXGISwapChain1> swapChain;
	CreateSwapChain(swapChain,gDxgiFactory);
	
	//������ȾĿ����ͼ������
	CreateRenderTargetViewDesCripHeap();
	
	//�������ģ����������
	CreateDepthStencilViewDesCripHeap();
	

	//������������������������
	D3D12_DESCRIPTOR_HEAP_DESC cbvsrvHeapDesc = {};
	cbvsrvHeapDesc.NumDescriptors = 2;
	cbvsrvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvsrvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	//��������������������
	ThrowIfFailed(g_device->CreateDescriptorHeap(&cbvsrvHeapDesc, IID_PPV_ARGS(&g_cbvsrvHeap)));
	g_cbvsrvDescriptorSize = g_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//��ȡ��Ⱦ��ͼ����ʼ��ַ
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(g_rtvHeap->GetCPUDescriptorHandleForHeapStart());

	for (UINT n = 0; n < FrameCount; n++)
	{
		ThrowIfFailed(swapChain->GetBuffer(n, IID_PPV_ARGS(&g_renderTargets[n])));
		g_device->CreateRenderTargetView(g_renderTargets[n].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, g_rtvDescriptorSize);
	}

	//�������������
	ThrowIfFailed(g_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_commandAllocator)));
}

void HelloGame::LoadAsset()
{
	//������ǩ����ѡ����ʵİ汾
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	if (FAILED(g_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	//�����Ը������������͸�����
	CD3DX12_DESCRIPTOR_RANGE1 ranges[2];
	CD3DX12_ROOT_PARAMETER1 rootParameters[1];

	//ָ���ø�����Ϊ������������ͼ
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	rootParameters[0].InitAsDescriptorTable(2, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);

	//�����ǩ������
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	//������̬������
	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.MipLODBias = 0;
	sampler.MaxAnisotropy = 0;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	//������ǩ��������
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, rootSignatureFlags);

	//������ǩ��
	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
	ThrowIfFailed(g_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&g_rootSignature)));

	
	//������ɫ����Դ
	ComPtr<ID3DBlob> vertexShader;
	ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif
	
	ThrowIfFailed(D3DCompileFromFile(std::wstring(L"Asset/Triangles.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
	ThrowIfFailed(D3DCompileFromFile(std::wstring(L"Asset/Triangles.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));
	



	//��������״̬����PSO������Ⱦ��Դ
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	//PSO����
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	psoDesc.pRootSignature = g_rootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState= CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	//����PSO
	ThrowIfFailed(g_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&g_pipelineState)));


	//���������б�������������������б�������
	ThrowIfFailed(g_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_commandAllocator.Get(), g_pipelineState.Get(), IID_PPV_ARGS(&g_commandList)));

	
	//ThrowIfFailed(g_commandList->Close());

	//��������Buffer
	Mode.Load("Asset/Monkey2.obj");
	/*Vertex triangleVertices[] =
	{
		{ { -1.0f, -1.0f , -1.0f }, {0.0f, 1.0f}, { 0.99f, 0.5f, 0.99f, 1.0f } },
		{ { -1.0f, +1.0f , -1.0f }, {0.0f, 0.0f},{ 0.01f, 0.8f, 1.0f, 1.0f } },
		{ { +1.0f, +1.0f , -1.0f }, {1.0f, 0.0f},{ 0.98f, 0.75f, 0.667f, 1.0f } },
		{ { +1.0f, -1.0f , -1.0f }, {1.0f, 1.0f},{ 0.99f, 0.99f, 0.4f, 1.0f } },


		{ { -1.0f, -1.0f , +1.0f }, {1.0f, 1.0f},{ 1.0f, 0.811f, 0.917f, 1.0f } },
		{ { +1.0f, -1.0f , +1.0f }, {0.0f, 1.0f},{ 0.796f, 1.0f, 0.9f, 1.0f } },
		{ { +1.0f, +1.0f , +1.0f }, {0.0f, 0.0f},{ 0.686f, 0.913f, 1.0f, 1.0f } },
		{ { -1.0f, +1.0f , +1.0f }, {1.0f, 0.0f},{ 0.99f, 1.0f, 0.745f, 1.0f } },


		{ { -1.0f, +1.0f , -1.0f }, {0.0f, 1.0f},{ 0.01f, 0.8f, 1.0f, 1.0f } },
		{ { -1.0f, +1.0f , +1.0f }, {0.0f, 0.0f},{ 0.99f, 1.0f, 0.745f, 1.0f } },
		{ { +1.0f, +1.0f , +1.0f }, {1.0f, 0.0f},{ 0.686f, 0.913f, 1.0f, 1.0f } },
		{ { +1.0f, +1.0f , -1.0f }, {1.0f, 1.0f},{ 0.98f, 0.75f, 0.667f, 1.0f } },


		{ { -1.0f, -1.0f , -1.0f }, {1.0f, 1.0f}, { 0.99f, 0.5f, 0.99f, 1.0f } },
		{ { +1.0f, -1.0f , -1.0f }, {0.0f, 1.0f},{ 0.99f, 0.99f, 0.4f, 1.0f } },
		{ { +1.0f, -1.0f , +1.0f }, {0.0f, 0.0f},{ 0.796f, 1.0f, 0.9f, 1.0f } },
		{ { -1.0f, -1.0f , +1.0f }, {1.0f, 0.0f},{ 1.0f, 0.811f, 0.917f, 1.0f } },

		{ { -1.0f, -1.0f , +1.0f }, {0.0f, 1.0f},{ 1.0f, 0.811f, 0.917f, 1.0f } },
		{ { -1.0f, +1.0f , +1.0f }, {0.0f, 0.0f},{ 0.99f, 1.0f, 0.745f, 1.0f } },
		{ { -1.0f, +1.0f , -1.0f }, {1.0f, 0.0f},{ 0.01f, 0.8f, 1.0f, 1.0f } },
		{ { -1.0f, -1.0f , -1.0f }, {1.0f, 1.0f}, { 0.99f, 0.5f, 0.99f, 1.0f } },

		{ { +1.0f, -1.0f , -1.0f }, {0.0f, 1.0f},{ 0.99f, 0.99f, 0.4f, 1.0f } },
		{ { +1.0f, +1.0f , -1.0f }, {0.0f, 0.0f},{ 0.98f, 0.75f, 0.667f, 1.0f } },
		{ { +1.0f, +1.0f , +1.0f }, {1.0f, 0.0f},{ 0.686f, 0.913f, 1.0f, 1.0f } },
		{ { +1.0f, -1.0f , +1.0f }, {1.0f, 1.0f},{ 0.796f, 1.0f, 0.9f, 1.0f } }
	};

	//����Buffer
	DWORD triangleIndex[]
	{
		0, 1, 2,
		0, 2, 3,


		4, 5, 6,
		4, 6, 7,

		8, 9, 10,
		8, 10, 11,

		12, 13, 14,
		12, 14, 15,

		16, 17, 18,
		16, 18, 19,

		20, 21, 22,
		20, 22, 23
	};

	UINT TriangleVertexSize = sizeof(triangleVertices);
	UINT TriangleIndexSize = sizeof(triangleIndex);*/

	UINT ModeVertexSize = (UINT)Mode.vertices.size() * sizeof(Mode.vertices[0]);
	UINT ModeIndexSize = (UINT)Mode.indices.size() * sizeof(Mode.indices[0]);

	
	
	const UINT vertexBufferSize = ModeVertexSize;
	const UINT indexBufferSize = ModeIndexSize;

	//�����ϴ���
	CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	//��Դ������
	CD3DX12_RESOURCE_DESC vertexResourceDes = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
	CD3DX12_RESOURCE_DESC indexResourceDes = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);
	

	//��Դ����
	ThrowIfFailed(g_device->CreateCommittedResource(&heapProperties,D3D12_HEAP_FLAG_NONE,&vertexResourceDes,
		D3D12_RESOURCE_STATE_GENERIC_READ,nullptr,IID_PPV_ARGS(&g_vertexBuffer)));

	ThrowIfFailed(g_device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &indexResourceDes,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&g_indexBuffer)));

	//���ƶ�����Դ���ݵ�GPUBuffer
	UINT8* pVertexDataBegin;
	CD3DX12_RANGE readRange(0, 0);
	ThrowIfFailed(g_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
	memcpy(pVertexDataBegin, Mode.vertices.data(), ModeVertexSize);
	g_vertexBuffer->Unmap(0, nullptr);

	ThrowIfFailed(g_indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
	memcpy(pVertexDataBegin, Mode.indices.data(), ModeIndexSize);
	g_indexBuffer->Unmap(0, nullptr);

	//��ʼ����Դ��������ͼ
	g_vertexBufferView.BufferLocation = g_vertexBuffer->GetGPUVirtualAddress();
	g_vertexBufferView.StrideInBytes = sizeof(Vertex);
	g_vertexBufferView.SizeInBytes = vertexBufferSize;

	g_indexBufferView.BufferLocation = g_indexBuffer->GetGPUVirtualAddress();
	g_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	g_indexBufferView.SizeInBytes = indexBufferSize;


	//�������ģ�建����������
	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

	//����������ģ�建����������
	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	/*D3D12_HEAP_TYPE_DEFAULT�����ڴ洢 GPU ����Ƶ������Դ��
	D3D12_HEAP_TYPE_UPLOAD�����ڴ洢 CPU �� GPU �ϴ����ݵ���Դ��
	D3D12_HEAP_TYPE_READBACK�����ڴ洢 GPU �� CPU �������ݵ���Դ��*/

	//����GPUƵ�����ʵĶѣ�����Ȼ�������ͼ����ö�
	CD3DX12_HEAP_PROPERTIES heapProperties2 = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	//������Դ�������������������ģ����ͼ
	CD3DX12_RESOURCE_DESC depthtex2D = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, g_width, g_height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

	//�ύ���ģ�建������Դ
	ThrowIfFailed(g_device->CreateCommittedResource(
		&heapProperties2,
		D3D12_HEAP_FLAG_NONE,
		&depthtex2D,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(&g_depthStencilBuffer)));

	//�������ģ�建����
	g_device->CreateDepthStencilView(g_depthStencilBuffer.Get(), &depthStencilDesc, g_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	
	//����������������Դ������
	constexpr UINT constantBufferSize = CalcConstantBufferByteSize<SceneConstantBuffer>();
	CD3DX12_RESOURCE_DESC constantResourceDes = CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize);
	//����������������Դ
	ThrowIfFailed(g_device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &constantResourceDes,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&g_constantBuffer)));

	//����������������ͼ������
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = g_constantBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = constantBufferSize;

	//����������������ͼ
	g_device->CreateConstantBufferView(&cbvDesc, g_cbvsrvHeap->GetCPUDescriptorHandleForHeapStart());
	//���Ƴ������������ݵ�GPU
	ThrowIfFailed(g_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&g_pCbvDataBegin)));
	memcpy(g_pCbvDataBegin, &g_constantBufferData, sizeof(g_constantBufferData));

	//������������
	D3D12_RESOURCE_DESC textureDesc;
	int texBytesPerRow;
	int texSize = LoadImageDataFromFile(&g_texData, textureDesc, L"Asset/BaseColor2.jpg", texBytesPerRow);

	//����������Դ�Ķ�
	CD3DX12_HEAP_PROPERTIES heapProperties3 = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	ThrowIfFailed(g_device->CreateCommittedResource(
		&heapProperties3,
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&g_textureBuffer)));

	//��¼������Դ�Ѵ�С
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(g_textureBuffer.Get(), 0, 1);

	//���������ϴ��� CPUGPU���ɷ���
	CD3DX12_HEAP_PROPERTIES heapProperties4 = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
	ThrowIfFailed(g_device->CreateCommittedResource(
		&heapProperties4,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&g_textureBufferUploadHeap)));

	//������������
	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = &g_texData[0];
	textureData.RowPitch = texBytesPerRow;
	textureData.SlicePitch = texBytesPerRow * textureDesc.Height;

	//����Դ���ϴ��ѿ�����Ĭ�϶ѣ������ö�����
	UpdateSubresources(g_commandList.Get(), g_textureBuffer.Get(), g_textureBufferUploadHeap.Get(), 0, 0, 1, &textureData);
	CD3DX12_RESOURCE_BARRIER resBarrier = CD3DX12_RESOURCE_BARRIER::Transition(g_textureBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	g_commandList->ResourceBarrier(1, &resBarrier);

	//������ɫ����Դ��ͼ����
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	//������ɫ����Դ��ͼ
	CD3DX12_CPU_DESCRIPTOR_HANDLE cbvsrvHandle(g_cbvsrvHeap->GetCPUDescriptorHandleForHeapStart());
	cbvsrvHandle.Offset(1, g_cbvsrvDescriptorSize);
	g_device->CreateShaderResourceView(g_textureBuffer.Get(), &srvDesc, cbvsrvHandle);

	//�ر������б�׼����Ⱦ
	ThrowIfFailed(g_commandList->Close());
	ID3D12CommandList* ppCommandLists[] = { g_commandList.Get() };
	g_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	//����һ��Χ��ͬ��CPU��GPU
	ThrowIfFailed(g_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_fence)));
	g_fenceValue = 1;
	g_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (g_fenceEvent == nullptr)
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}
	
}

void HelloGame::PopulateCommandList()
{
	ThrowIfFailed(g_commandAllocator->Reset());
	ThrowIfFailed(g_commandList->Reset(g_commandAllocator.Get(), g_pipelineState.Get()));

	//���ñ�Ҫ״̬
	g_commandList->SetGraphicsRootSignature(g_rootSignature.Get());

	//���ó��������������ѣ��ύ����Ⱦ����
	ID3D12DescriptorHeap* ppHeaps[] = { g_cbvsrvHeap.Get() };
	g_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	//���ø����������ϴ�����
	g_commandList->SetGraphicsRootDescriptorTable(0, g_cbvsrvHeap->GetGPUDescriptorHandleForHeapStart());
	g_commandList->RSSetViewports(1, &g_viewport);
	g_commandList->RSSetScissorRects(1, &g_scissorRect);

	//ִ����Դת��״̬(����״̬תΪ��ȾĿ��״̬��ִ�л��Ʋ���)
	D3D12_RESOURCE_BARRIER resBarrier = CD3DX12_RESOURCE_BARRIER::Transition(g_renderTargets[g_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	g_commandList->ResourceBarrier(1, &resBarrier);
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(g_rtvHeap->GetCPUDescriptorHandleForHeapStart(), g_frameIndex, g_rtvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(g_dsvHeap->GetCPUDescriptorHandleForHeapStart());

	//������ȾĿ��
	g_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	//�����ȾĿ��
	g_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	g_commandList->ClearDepthStencilView(g_dsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	
	//ͼԪ����ģʽ
	g_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//����װ��
	g_commandList->IASetVertexBuffers(0, 1, &g_vertexBufferView);
	g_commandList->IASetIndexBuffer(&g_indexBufferView);

	//��ͼ
	g_commandList->DrawIndexedInstanced((UINT)Mode.indices.size(), 1, 0, 0, 0);

	//ִ����Դת��״̬(��ȾĿ��״̬תΪ����״̬)
	resBarrier = CD3DX12_RESOURCE_BARRIER::Transition(g_renderTargets[g_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	g_commandList->ResourceBarrier(1, &resBarrier);

	//�ر������б����������ύ����ʼ��Ⱦ
	ThrowIfFailed(g_commandList->Close());
}

void HelloGame::WaitForPreviousFrame()
{
	//����Χ��ֵ���ƽ�GPU
	const UINT64 fence = g_fenceValue;
	ThrowIfFailed(g_commandQueue->Signal(g_fence.Get(), fence));
	//����Χ��ֵ��������һ֡��Ⱦ
	g_fenceValue++;

	//�ȴ���Ⱦ���
	if (g_fence->GetCompletedValue() < fence)
	{
		ThrowIfFailed(g_fence->SetEventOnCompletion(fence, g_fenceEvent));
		WaitForSingleObject(g_fenceEvent, INFINITE);
	}

	g_frameIndex = g_swapChain->GetCurrentBackBufferIndex();
}

void HelloGame::CreateGPUElement(ComPtr<IDXGIFactory6>& gDxgiFactory)
{
	

	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&gDxgiFactory)));

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	//�ҵ���ʾ�豸
	IDXGIAdapter1* adapter = nullptr;
	for (std::uint32_t i = 0U; i < _countof(featureLevels); ++i)
	{
		adapter = GetSupportedAdapter(gDxgiFactory, featureLevels[i]);
		if (adapter != nullptr)
		{
			break;
		}
	}

	//����GPU��������
	if (adapter != nullptr)
	{
		D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&g_device));
	}
}

void HelloGame::CreateCommandQueue()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(g_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&g_commandQueue)));
}

void HelloGame::CreateSwapChain(ComPtr<IDXGISwapChain1>& swapChain, ComPtr<IDXGIFactory6>& gDxgiFactory)
{
	//����������
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = FrameCount;
	swapChainDesc.Width = g_width;
	swapChainDesc.Height = g_height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	//ָ������
	ThrowIfFailed(gDxgiFactory->CreateSwapChainForHwnd(
		g_commandQueue.Get(),
		GameWindowApplication::GetHwnd(),
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));

	//ת����������Ϣ����ȡ����������
	ThrowIfFailed(swapChain.As(&g_swapChain));
	g_frameIndex = g_swapChain->GetCurrentBackBufferIndex();
}

void HelloGame::CreateRenderTargetViewDesCripHeap()
{
	//������ȾĿ����ͼ������������
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = FrameCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	//������ȾĿ����ͼ������
	ThrowIfFailed(g_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&g_rtvHeap)));
	g_rtvDescriptorSize = g_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	//std::wstring strToDisplay = std::to_wstring(g_rtvDescriptorSize);
	//wcsncpy_s(DebugToDisplay, strToDisplay.c_str(), sizeof(DebugToDisplay) / sizeof(DebugToDisplay[0]));
}

void HelloGame::CreateDepthStencilViewDesCripHeap()
{
	//�������ģ������������
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	//�������ģ����������
	ThrowIfFailed(g_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&g_dsvHeap)));
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
	FVector3 RotationAxis {1,0,0};
	float angle = 1.0f;
	lightangle += angle;
	float angleInRadians = ZHEngineMath::AngleToRadians(lightangle);
	FVector4 rotationAxisVector = XMVector4Normalize(XMQuaternionRotationAxis(RotationAxis, angleInRadians));
	Float4 lightDir = ZHEngineMath::FVector4ToFloat4(rotationAxisVector);
	light.direction = {lightDir.x,lightDir.y,lightDir.z};
	g_constantBufferData.lightColor = light.color;
	g_constantBufferData.lightDirection = ZHEngineMath::Normalize(light.direction);
	
}

void HelloGame::UpdateConstantBuffer()
{

	UpdateMVP();
	UpdateLight();
	/*const float translationSpeed = 0.005f;
	const float offsetBounds = 1.75f;
	g_constantBufferData.offset.x += translationSpeed;
	if (g_constantBufferData.offset.x > offsetBounds)
	{
		g_constantBufferData.offset.x = -offsetBounds;
	}*/
	//GPU��������
	memcpy(g_pCbvDataBegin, &g_constantBufferData, sizeof(g_constantBufferData));
}

void HelloGame::UpdateMVP()
{
	float x = g_camera.g_Radius * sinf(g_camera.g_Phi) * cosf(g_camera.g_Theta);
	float z = g_camera.g_Radius * sinf(g_camera.g_Phi) * sinf(g_camera.g_Theta);
	float y = g_camera.g_Radius * cosf(g_camera.g_Phi);

	FVector4 pos = ZHEngineMath::MakeFvector4(x, y, z, 1.0f);
	FVector4 target = ZHEngineMath::MakeFvector4(0.0f, 0.0f, 0.0f, 1.0f);
	FVector4 up = ZHEngineMath::MakeFvector4(0.0f, 1.0f, 0.0f, 0.0f);

	FMatrix4x4 v = ZHEngineMath::LookAt(pos, target, up);

	FMatrix4x4 m = ZHEngineMath::MatrixIdentity();
	FMatrix4x4 p = ZHEngineMath::MatrixFov(PIDIV4, AspectRatio(), 1.0f, 1000.0f);
	FMatrix4x4 MVP = m * v * p;

	ZHEngineMath::MaterixToFloat4x4(&g_constantBufferData.ObjectToWorld, m);
	ZHEngineMath::MaterixToFloat4x4(&g_constantBufferData.MVP, MVP);
	g_constantBufferData.viewPosition = { x,y,z };
	
}
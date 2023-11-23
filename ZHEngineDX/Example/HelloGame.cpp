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
	Game(width, height, name),
	g_frameIndex(0),
	g_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
	g_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
	g_rtvDescriptorSize(0),
	g_constantBufferData{},
	light{Float4(1,1,1,0),FLinearColor(1,1,1,1)}
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
	MoveToNextFrame();
}

void HelloGame::OnDestroy()
{
	WaitForGPU();
	CloseHandle(g_fenceEvent);
	
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
	CreateRenderTargetViewDesCribeHeap();
	
	//�������ģ����������
	CreateDepthStencilViewDesCribeHeap();
	
	//����������������������
	CreateConstantBufferDesCribeHeap();
	
	//��ȡ��Ⱦ��ͼ����ʼ��ַ
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(g_rtvHeap->GetCPUDescriptorHandleForHeapStart());

	//����Χ��
	CreateFrameResource(swapChain, rtvHandle);
	
	
}

void HelloGame::LoadAsset()
{
	//������ǩ����ѡ����ʵİ汾
	CreateRootSignature();

	
	//������ɫ����Դ
	ComPtr<ID3DBlob> vertexShader;
	ComPtr<ID3DBlob> pixelShader;
	UINT compileFlags = 0;

#if defined(_DEBUG)
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	
	ThrowIfFailed(D3DCompileFromFile(std::wstring(L"Shader/Triangles.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
	ThrowIfFailed(D3DCompileFromFile(std::wstring(L"Shader/Triangles.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));
	

	//����PSO
	CreatePSO(vertexShader,pixelShader);


	//���������б�������������������б�������
	ThrowIfFailed(g_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_commandAllocator[g_frameIndex].Get(), g_pipelineState.Get(), IID_PPV_ARGS(&g_commandList)));

	
	//ThrowIfFailed(g_commandList->Close());

	//��������Buffer
#if WRITEMODE
	OBJ Mode;
	Mode.Load(MODEASSETPATH);
#endif

	Mesh.Load(MODEPATH);

	UINT ModeVertexSize = (UINT)Mesh.vertices.size() * sizeof(Vertex);
	UINT ModeIndexSize = (UINT)Mesh.indices.size() * sizeof(UINT);

	
	
	//const UINT vertexBufferSize = ModeVertexSize;
	//const UINT indexBufferSize = ModeIndexSize;

	//�����ϴ���
	CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RANGE readRange(0, 0);

	//�ϴ�����Ͷ���������Ϣ
	UpLoadVertexAndIndexToHeap(heapProperties,readRange,ModeVertexSize, ModeIndexSize);

	//�������ģ�建����
	UpLoadDepthStencialBuffer();
	
	//�������ϴ����ݵ�����������
	UpLoadConstantBuffer(heapProperties,readRange);

	//�����������ݲ�������ɫ����Դ
	UpLoadShaderResource();

	//�ر������б�׼����Ⱦ
	ThrowIfFailed(g_commandList->Close());
	ID3D12CommandList* ppCommandLists[] = { g_commandList.Get() };
	g_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	//����Χ��
	SetFence();
	
}

void HelloGame::PopulateCommandList()
{
	ThrowIfFailed(g_commandAllocator[g_frameIndex]->Reset());
	ThrowIfFailed(g_commandList->Reset(g_commandAllocator[g_frameIndex].Get(), g_pipelineState.Get()));

	//���ñ�Ҫ״̬
	g_commandList->SetGraphicsRootSignature(g_rootSignature.Get());

	//���ó��������������ѣ��ύ����Ⱦ����
	ID3D12DescriptorHeap* ppHeaps[] = { g_cbvsrvHeap.Get() };
	g_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	//���ø����������ϴ�����
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandle = g_cbvsrvHeap->GetGPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandleForSecondDescriptor = { gpuDescriptorHandle .ptr+ g_cbvsrvDescriptorSize };
	g_commandList->SetGraphicsRootDescriptorTable(0, gpuDescriptorHandle);
	g_commandList->SetGraphicsRootDescriptorTable(1, gpuDescriptorHandleForSecondDescriptor);
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
	float BackGroundColor[4] = {0,0,0,1};
	g_commandList->ClearRenderTargetView(rtvHandle, BackGroundColor, 0, nullptr);
	g_commandList->ClearDepthStencilView(g_dsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	
	//ͼԪ����ģʽ
	g_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//����װ��
	g_commandList->IASetVertexBuffers(0, 1, &g_vertexBufferView);
	g_commandList->IASetIndexBuffer(&g_indexBufferView);

	//��ͼ
	g_commandList->DrawIndexedInstanced((UINT)Mesh.indices.size(), 1, 0, 0, 0);

	//ִ����Դת��״̬(��ȾĿ��״̬תΪ����״̬)
	resBarrier = CD3DX12_RESOURCE_BARRIER::Transition(g_renderTargets[g_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	g_commandList->ResourceBarrier(1, &resBarrier);

	//�ر������б����������ύ����ʼ��Ⱦ
	ThrowIfFailed(g_commandList->Close());
}

void HelloGame::MoveToNextFrame()
{
	const UINT64 currentFenceValue = g_fenceValues[g_frameIndex];
	ThrowIfFailed(g_commandQueue->Signal(g_fence.Get(), currentFenceValue));

	// ����֡����
	g_frameIndex = g_swapChain->GetCurrentBackBufferIndex();

	// �ȴ�GPU
	if (g_fence->GetCompletedValue() < g_fenceValues[g_frameIndex])
	{
		ThrowIfFailed(g_fence->SetEventOnCompletion(g_fenceValues[g_frameIndex], g_fenceEvent));
		WaitForSingleObjectEx(g_fenceEvent, INFINITE, FALSE);
	}

	// Set the fence value for the next frame.
	g_fenceValues[g_frameIndex] = currentFenceValue + 1;
}

void HelloGame::WaitForGPU()
{
	//����Χ��ֵ���ƽ�GPU
	ThrowIfFailed(g_commandQueue->Signal(g_fence.Get(), g_fenceValues[g_frameIndex]));
	//����Χ��ֵ��������һ֡��Ⱦ
	

	//�ȴ���Ⱦ���
	
	ThrowIfFailed(g_fence->SetEventOnCompletion(g_fenceValues[g_frameIndex], g_fenceEvent));
	WaitForSingleObject(g_fenceEvent, INFINITE);
	g_fenceValues[g_frameIndex]++;

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

	ThrowIfFailed(gDxgiFactory->MakeWindowAssociation(GameWindowApplication::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));
	//ת����������Ϣ����ȡ����������
	ThrowIfFailed(swapChain.As(&g_swapChain));
	g_frameIndex = g_swapChain->GetCurrentBackBufferIndex();
}

void HelloGame::CreateRenderTargetViewDesCribeHeap()
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

void HelloGame::CreateDepthStencilViewDesCribeHeap()
{
	//�������ģ������������
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	//�������ģ����������
	ThrowIfFailed(g_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&g_dsvHeap)));
}

void HelloGame::CreateConstantBufferDesCribeHeap()
{
	//������������������������
	D3D12_DESCRIPTOR_HEAP_DESC cbvsrvHeapDesc = {};
	cbvsrvHeapDesc.NumDescriptors = 3;
	cbvsrvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvsrvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	//��������������������
	ThrowIfFailed(g_device->CreateDescriptorHeap(&cbvsrvHeapDesc, IID_PPV_ARGS(&g_cbvsrvHeap)));
	g_cbvsrvDescriptorSize = g_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void HelloGame::CreateFrameResource(ComPtr<IDXGISwapChain1>& swapChain, CD3DX12_CPU_DESCRIPTOR_HANDLE& rtvHandle)
{
	for (UINT n = 0; n < FrameCount; n++)
	{
		ThrowIfFailed(swapChain->GetBuffer(n, IID_PPV_ARGS(&g_renderTargets[n])));
		g_device->CreateRenderTargetView(g_renderTargets[n].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, g_rtvDescriptorSize);

		//�������������
		ThrowIfFailed(g_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_commandAllocator[n])));
	}
}

void HelloGame::CreateRootSignature()
{
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	if (FAILED(g_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}
	//�����Ը������������͸�����
	CD3DX12_DESCRIPTOR_RANGE1 ranges[3];
	CD3DX12_ROOT_PARAMETER1 rootParameters[2];

	//ָ���ø�����Ϊ������������ͼ
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);
	ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	rootParameters[1].InitAsDescriptorTable(2, &ranges[1], D3D12_SHADER_VISIBILITY_ALL);
	
	

	//�����ǩ������
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	//������̬������
	
	D3D12_STATIC_SAMPLER_DESC sampler0 = CreateSamplerDesCribe(0);
	D3D12_STATIC_SAMPLER_DESC sampler1 = CreateSamplerDesCribe(1);
	std::vector<D3D12_STATIC_SAMPLER_DESC>samplers;
	samplers.push_back(sampler0);
	samplers.push_back(sampler1);

	//������ǩ��������
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, static_cast<UINT>(samplers.size()), samplers.data(), rootSignatureFlags);

	
	//������ǩ��
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
	//��������״̬����PSO������Ⱦ��Դ
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	//PSO����
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
	//����PSO
	ThrowIfFailed(g_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&g_pipelineState)));
}

void HelloGame::UpLoadVertexAndIndexToHeap(CD3DX12_HEAP_PROPERTIES& heapProperties, CD3DX12_RANGE& readRange,const UINT vertexBufferSize, const UINT indexBufferSize)
{
	
	//��Դ������
	CD3DX12_RESOURCE_DESC vertexResourceDes = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
	CD3DX12_RESOURCE_DESC indexResourceDes = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);


	//��Դ����
	ThrowIfFailed(g_device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &vertexResourceDes,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&g_vertexBuffer)));

	ThrowIfFailed(g_device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &indexResourceDes,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&g_indexBuffer)));

	//���ƶ�����Դ���ݵ�GPUBuffer
	UINT8* pVertexDataBegin;
	ThrowIfFailed(g_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
	memcpy(pVertexDataBegin, Mesh.vertices.data(), vertexBufferSize);
	g_vertexBuffer->Unmap(0, nullptr);

	ThrowIfFailed(g_indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
	memcpy(pVertexDataBegin, Mesh.indices.data(), indexBufferSize);
	g_indexBuffer->Unmap(0, nullptr);

	//��ʼ����Դ��������ͼ
	g_vertexBufferView.BufferLocation = g_vertexBuffer->GetGPUVirtualAddress();
	g_vertexBufferView.StrideInBytes = sizeof(Vertex);
	g_vertexBufferView.SizeInBytes = vertexBufferSize;

	g_indexBufferView.BufferLocation = g_indexBuffer->GetGPUVirtualAddress();
	g_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	g_indexBufferView.SizeInBytes = indexBufferSize;
}

void HelloGame::UpLoadDepthStencialBuffer()
{
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
}

void HelloGame::UpLoadConstantBuffer(CD3DX12_HEAP_PROPERTIES& heapProperties, CD3DX12_RANGE& readRange)
{
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
}

void HelloGame::UpLoadShaderResource()
{
	
	LoadTexture();
	
	//����������Դ�Ķ�
	CD3DX12_HEAP_PROPERTIES TexResourceheap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	//���������ϴ��� CPUGPU���ɷ���
	CD3DX12_HEAP_PROPERTIES DataUpLoadheap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	//������ɫ����Դ��ͼ����
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	//��ȡ��ɫ����Դ��ͼ��ʼ��ַ
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

		//����Դ���ϴ��ѿ�����Ĭ�϶ѣ������ö�����
		UpdateSubresources(g_commandList.Get(), Texture.Resource.Get(), Texture.UploadHeap.Get(), 0, 0, 1, &Texture.texData);
		CD3DX12_RESOURCE_BARRIER Barrier = CD3DX12_RESOURCE_BARRIER::Transition(Texture.Resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		g_commandList->ResourceBarrier(1, &Barrier);

		srvDesc.Format = Texture.texDesc.Format;
		cbvsrvHandle.Offset(1, g_cbvsrvDescriptorSize);
		g_device->CreateShaderResourceView(Texture.Resource.Get(), &srvDesc, cbvsrvHandle);
	}
	
}

void HelloGame::SetFence()
{
	//����һ��Χ��ͬ��CPU��GPU
	ThrowIfFailed(g_device->CreateFence(g_fenceValues[g_frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_fence)));
	g_fenceValues[g_frameIndex]++;
	g_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (g_fenceEvent == nullptr)
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}
	WaitForGPU();
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
	float angle = 0.25f;
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
	//GPU��������
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
	TextureFiles.push_back(L"Content/Tex/Sand_00_BaseColorAO.png");
	TextureFiles.push_back(L"Content/Tex/Sand_00_NormalR.png");
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

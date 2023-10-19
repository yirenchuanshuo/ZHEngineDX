#include "HelloGame.h"

void DebugMessage(std::wstring strToDisplay)
{
	wcsncpy_s(DebugToDisplay, strToDisplay.c_str(), sizeof(DebugToDisplay) / sizeof(DebugToDisplay[0]));
}


HelloGame::HelloGame(UINT width, UINT height, std::wstring name):
	Game(width, height, name),
	g_frameIndex(0),
	g_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
	g_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
	g_rtvDescriptorSize(0)
{

}

void HelloGame::OnInit()
{
	LoadPipeline();
	LoadAsset();
}

void HelloGame::OnUpdate()
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
	ComPtr<IDXGIFactory4> gDxgiFactory;
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


	//�����������
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(g_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&g_commandQueue)));

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = FrameCount;
	swapChainDesc.Width = g_width;
	swapChainDesc.Height = g_height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	//������������ָ������
	ComPtr<IDXGISwapChain1> swapChain;
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

	//������ȾĿ����ͼ������������
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = FrameCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	//������ȾĿ����ͼ
	ThrowIfFailed(g_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&g_rtvHeap)));
	g_rtvDescriptorSize = g_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	//std::wstring strToDisplay = std::to_wstring(g_rtvDescriptorSize);
	//wcsncpy_s(DebugToDisplay, strToDisplay.c_str(), sizeof(DebugToDisplay) / sizeof(DebugToDisplay[0]));

	//�������ģ������������
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	//�������ģ����������
	ThrowIfFailed(g_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&g_dsvHeap)));


	//������ȾĿ����ͼ
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
	//������ǩ��������
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	//������ǩ��
	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
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
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
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
	//����PSO
	ThrowIfFailed(g_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&g_pipelineState)));


	//���������б�������������������б�������
	ThrowIfFailed(g_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_commandAllocator.Get(), g_pipelineState.Get(), IID_PPV_ARGS(&g_commandList)));

	//�ر������б�׼����Ⱦ
	ThrowIfFailed(g_commandList->Close());

	//��������Buffer
	Vertex triangleVertices[] =
	{
		{ { -0.5f, 0.5f, 0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
		{ {  0.5f,-0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
		{ { -0.5f,-0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
		{ {  0.5f, 0.5f, 0.5f }, { 1.0f, 1.0f, 0.0f, 1.0f } },

		{ { -0.75f, 0.75f, 0.7f }, { 1.0f, 1.0f, 1.0f, 1.0f }},
		{ { 0.0f, 0.0f, 0.7f }, { 1.0f, 0.0f, 1.0f, 1.0f } },
		{ { -0.75f, 0.0f, 0.7f }, { 0.0f, 1.0f, 1.0f, 1.0f }},
		{ { 0.0f, 0.75f, 0.7f }, { 1.0f, 1.0f, 1.0f, 1.0f }}
	};

	//����Buffer
	DWORD triangleIndex[]
	{
		0,1,2,
		0,3,1
	};

	const UINT vertexBufferSize = sizeof(triangleVertices);
	const UINT indexBufferSize = sizeof(triangleIndex);

	//�����ϴ���
	CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	//��Դ������
	CD3DX12_RESOURCE_DESC resourceDes = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
	
	

	//�ύ��Դ����
	ThrowIfFailed(g_device->CreateCommittedResource(&heapProperties,D3D12_HEAP_FLAG_NONE,&resourceDes,
		D3D12_RESOURCE_STATE_GENERIC_READ,nullptr,IID_PPV_ARGS(&g_vertexBuffer)));

	ThrowIfFailed(g_device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDes,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&g_indexBuffer)));

	//������Դ���ݵ�GPUBuffer
	UINT8* pDataBegin;
	CD3DX12_RANGE readRange(0, 0);
	ThrowIfFailed(g_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pDataBegin)));
	memcpy(pDataBegin, triangleVertices, sizeof(triangleVertices));
	g_vertexBuffer->Unmap(0, nullptr);

	ThrowIfFailed(g_indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pDataBegin)));
	memcpy(pDataBegin, triangleIndex, sizeof(triangleIndex));
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
	g_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	//ͼԪ����ģʽ
	g_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//����װ��
	g_commandList->IASetVertexBuffers(0, 1, &g_vertexBufferView);
	g_commandList->IASetIndexBuffer(&g_indexBufferView);
	//��ͼ
	g_commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
	g_commandList->DrawIndexedInstanced(6, 1, 0, 4, 0);

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

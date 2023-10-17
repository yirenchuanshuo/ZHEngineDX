#include "HelloGame.h"
#include <iostream>



HelloGame::HelloGame(UINT width, UINT height, std::wstring name):
	Game(width, height, name)
{

}

void HelloGame::OnInit()
{
	LoadPipeline();
	LoadAsset();
}

void HelloGame::OnUpdate()
{
}

void HelloGame::OnRender()
{
}

void HelloGame::OnDestroy()
{
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
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(gDxgiFactory.GetAddressOf())));

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
		D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(g_device.GetAddressOf()));
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

	//������ȾĿ����ͼ����
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = FrameCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(g_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&g_rtvHeap)));
	g_rtvDescriptorSize = g_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	//std::wstring strToDisplay = std::to_wstring(g_rtvDescriptorSize);
	//wcsncpy_s(DebugToDisplay, strToDisplay.c_str(), sizeof(DebugToDisplay) / sizeof(DebugToDisplay[0]));


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
	//���������б�������������������б�������
	ThrowIfFailed(g_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&g_commandList)));
	
	//�ر������б�׼����Ⱦ
	ThrowIfFailed(g_commandList->Close());

	{
		//����һ��Χ��ͬ��CPU��GPU
		ThrowIfFailed(g_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_fence)));
		g_fenceValue = 1;

		g_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (g_fenceEvent == nullptr)
		{
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}
	}
}

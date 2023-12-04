#include "DXRHI.h"


DXRHI::DXRHI()
{

}

DXRHI::~DXRHI()
{
}

void DXRHI::CreateDeviceElement(UINT WindowWidth, UINT WindowHeight,HWND& hwnd)
{
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&g_DxgiFactory)));

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	//找到显示设备
	IDXGIAdapter1* adapter = nullptr;
	for (std::uint32_t i = 0U; i < _countof(featureLevels); ++i)
	{
		adapter = GetSupportedAdapter(g_DxgiFactory, featureLevels[i]);
		if (adapter != nullptr)
		{
			break;
		}
	}

	//创建GPU交互对象
	if (adapter != nullptr)
	{
		D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&g_Device));
	}

	CreateCommandQueue();

	//创建交换链
	ComPtr<IDXGISwapChain1> swapChain;
	//交换链描述
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = FrameCount;
	swapChainDesc.Width = WindowWidth;
	swapChainDesc.Height = WindowHeight;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	//指定窗口
	ThrowIfFailed(g_DxgiFactory->CreateSwapChainForHwnd(
		g_CommandQueue.Get(),
		hwnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));

	ThrowIfFailed(g_DxgiFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));
	//转换交换链信息，获取交换链索引
	ThrowIfFailed(swapChain.As(&g_SwapChain));
	g_frameIndex = g_SwapChain->GetCurrentBackBufferIndex();

	//创建渲染目标视图描述堆
	CreateRenderTargetViewDesCribeHeap();

	//创建深度模板描述符堆
	CreateDepthStencilViewDesCribeHeap();

	//设置围栏
	CreateFrameResource();
}

IDXGIAdapter1* DXRHI::GetSupportedAdapter(ComPtr<IDXGIFactory6>& dxgiFactory, const D3D_FEATURE_LEVEL featureLevel)
{
	IDXGIAdapter1* adapter = nullptr;
	for (std::uint32_t adapterIndex = 0U; ; ++adapterIndex)
	{
		IDXGIAdapter1* currentAdapter = nullptr;
		if (DXGI_ERROR_NOT_FOUND == dxgiFactory->EnumAdapters1(adapterIndex, &currentAdapter))
		{
			break;
		}

		const HRESULT hres = D3D12CreateDevice(currentAdapter, featureLevel, _uuidof(ID3D12Device), nullptr);
		if (SUCCEEDED(hres))
		{
			adapter = currentAdapter;
			break;
		}

		currentAdapter->Release();
	}

	return adapter;
}

void DXRHI::CreateCommandQueue()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(g_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&g_CommandQueue)));
}



void DXRHI::CreateRenderTargetViewDesCribeHeap()
{
	//创建渲染目标视图描述符堆描述
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = FrameCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	//创建渲染目标视图描述堆
	ThrowIfFailed(g_Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&g_rtvDescriptorHeap)));
	g_rtvDescriptorSize = g_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	//std::wstring strToDisplay = std::to_wstring(g_rtvDescriptorSize);
	//wcsncpy_s(DebugToDisplay, strToDisplay.c_str(), sizeof(DebugToDisplay) / sizeof(DebugToDisplay[0]));
}

void DXRHI::CreateDepthStencilViewDesCribeHeap()
{
	//创建深度模板描述堆描述
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	//创建深度模板描述符堆
	ThrowIfFailed(g_Device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&g_dsvDescriptorHeap)));
}

void DXRHI::CreateFrameResource()
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(g_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	for (UINT n = 0; n < FrameCount; n++)
	{
		ThrowIfFailed(g_SwapChain->GetBuffer(n, IID_PPV_ARGS(&g_renderTargets[n])));
		g_Device->CreateRenderTargetView(g_renderTargets[n].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, g_rtvDescriptorSize);

		//创建命令分配器
		ThrowIfFailed(g_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_CommandAllocators[n])));
	}
}

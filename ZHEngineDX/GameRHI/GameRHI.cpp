#include"GameRHI.h"


namespace
{
	inline DXGI_FORMAT NoSRGB(DXGI_FORMAT fmt)
	{
		switch (fmt)
		{
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:   return DXGI_FORMAT_R8G8B8A8_UNORM;
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:   return DXGI_FORMAT_B8G8R8A8_UNORM;
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:   return DXGI_FORMAT_B8G8R8X8_UNORM;
		default:                                return fmt;
		}
	}
};

GameRHI::GameRHI(UINT width, UINT height,  std::wstring name,
	DXGI_FORMAT backBufferFormat, DXGI_FORMAT depthBufferFormat):
    g_width(width),
    g_height(height),
    g_title(name),
	g_frameIndex(0),
	g_fenceValues{},
	g_rtvDescriptorSize(0),
	g_backBufferFormat(backBufferFormat),
	g_depthstencilBufferFormat(depthBufferFormat),
	g_useWarpDevice(false),
	g_Drag(false),
	hwnd(nullptr),
	g_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
	g_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height))
{
	WCHAR assetsPath[512];
	GetAssetsPath(assetsPath, _countof(assetsPath));
	g_assetsPath = assetsPath;


	g_aspectRatio = static_cast<float>(width) / static_cast<float>(height);
}

GameRHI::~GameRHI()
{
	
}

void GameRHI::SetWindow()
{
	hwnd = GameWindowApplication::GetHwnd();
}

void GameRHI::CreateDeviceResources()
{
	//创建GPU交互对象
	CreateGPUElement();

	//创建命令队列
	CreateCommandQueue();

	//创建渲染目标视图描述堆
	CreateRenderTargetViewDesCribeHeap();

	//创建深度模板描述符堆
	if (g_depthstencilBufferFormat != DXGI_FORMAT_UNKNOWN)
	{
		CreateDepthStencilViewDesCribeHeap();
	}
	
	//创建命令分配器
	for (UINT n = 0; n < FrameCount; n++)
	{
		ThrowIfFailed(g_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(g_commandAllocators[n].ReleaseAndGetAddressOf())));
	}

	//创建命令列表，用命令分配器给命令列表分配对象
	ThrowIfFailed(g_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_commandAllocators[0].Get(), nullptr, IID_PPV_ARGS(g_commandList.ReleaseAndGetAddressOf())));
	NAME_D3D12_OBJECT(g_commandList);
	ThrowIfFailed(g_commandList->Close());

	
	ThrowIfFailed(g_device->CreateFence(g_fenceValues[g_frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(g_fence.ReleaseAndGetAddressOf())));
	g_fenceValues[g_frameIndex]++;

	g_fenceEvent.Attach(CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE));

	if (g_MSAA)
	{
		CreateMSAAResource();
	}

}

void GameRHI::CreateWindowResources()
{
	if (!hwnd)
	{
		throw std::exception("Call SetWindow with a valid Win32 window handle");
	}

	WaitForGPU();

	for (UINT n = 0; n < FrameCount; n++)
	{
		g_renderTargets[n].Reset();
		g_fenceValues[n] = g_fenceValues[g_frameIndex];
	}

	DXGI_FORMAT backBufferFormat = NoSRGB(g_backBufferFormat);

	if (g_swapChain)
	{
		HRESULT hr = g_swapChain->ResizeBuffers(
			FrameCount,
			g_width,
			g_height,
			backBufferFormat,
			0u
		);
		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
#ifdef _DEBUG
			char buff[64] = {};
			sprintf_s(buff, "Device Lost on ResizeBuffers: Reason code 0x%08X\n", (hr == DXGI_ERROR_DEVICE_REMOVED) ? g_device->GetDeviceRemovedReason() : hr);
			OutputDebugStringA(buff);
#endif
			return;
		}
		else
		{
			ThrowIfFailed(hr);
		}
	}
	else
	{
		CreateSwapChain();
	}

	CreateFrameResource();
	g_frameIndex = g_swapChain->GetCurrentBackBufferIndex();

	if (g_depthstencilBufferFormat != DXGI_FORMAT_UNKNOWN)
	{
		CreateDepthStencialBuffer();
	}

	g_viewport.MinDepth = D3D12_MIN_DEPTH;
	g_viewport.MaxDepth = D3D12_MAX_DEPTH;

	if (g_MSAA)
	{
		CreateMSAAWindowResource();
	}
}

void GameRHI::CreateMSAAResource()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc = {};
	rtvDescriptorHeapDesc.NumDescriptors = 1;
	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	ThrowIfFailed(g_device->CreateDescriptorHeap(&rtvDescriptorHeapDesc,
		IID_PPV_ARGS(g_msaaRTVDescriptorHeap.ReleaseAndGetAddressOf())));


	D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc = {};
	dsvDescriptorHeapDesc.NumDescriptors = 1;
	dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

	ThrowIfFailed(g_device->CreateDescriptorHeap(&dsvDescriptorHeapDesc,
		IID_PPV_ARGS(g_msaaDSVDescriptorHeap.ReleaseAndGetAddressOf())));
}

void GameRHI::CreateMSAAWindowResource()
{
	CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);

	D3D12_RESOURCE_DESC msaaRTDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		g_backBufferFormat,
		g_width,
		g_height,
		1, // This render target view has only one texture.
		1, // Use a single mipmap level
		4
	);
	msaaRTDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_CLEAR_VALUE msaaOptimizedClearValue = {};
	msaaOptimizedClearValue.Format = g_backBufferFormat;
	memcpy(msaaOptimizedClearValue.Color, clearColor, sizeof(float) * 4);

	ThrowIfFailed(g_device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&msaaRTDesc,
		D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
		&msaaOptimizedClearValue,
		IID_PPV_ARGS(g_msaaRenderTarget.ReleaseAndGetAddressOf())));

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = g_backBufferFormat;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;

	g_device->CreateRenderTargetView(
		g_msaaRenderTarget.Get(), &rtvDesc,
		g_msaaRTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	D3D12_RESOURCE_DESC depthStencilDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		g_depthstencilBufferFormat,
		g_width,
		g_height,
		1, // This depth stencil view has only one texture.
		1, // Use a single mipmap level.
		4
	);
	depthStencilDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = g_depthstencilBufferFormat;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	ThrowIfFailed(g_device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(g_msaaDepthStencil.ReleaseAndGetAddressOf())
	));

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = g_depthstencilBufferFormat;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;

	g_device->CreateDepthStencilView(
		g_msaaDepthStencil.Get(), &dsvDesc,
		g_msaaDSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

void GameRHI::WaitForGPU()
{
	if (g_commandQueue && g_fence && g_fenceEvent.IsValid())
	{
		UINT64 fenceValue = g_fenceValues[g_frameIndex];
		//储存围栏值，移交GPU
		ThrowIfFailed(g_commandQueue->Signal(g_fence.Get(), fenceValue));
		//更新围栏值，用于下一帧渲染

		//等待渲染完成
		ThrowIfFailed(g_fence->SetEventOnCompletion(fenceValue, g_fenceEvent.Get()));
		WaitForSingleObject(g_fenceEvent.Get(), INFINITE);
		g_fenceValues[g_frameIndex]++;
	}
	
}

void GameRHI::MoveToNextFrame()
{
	const UINT64 currentFenceValue = g_fenceValues[g_frameIndex];
	ThrowIfFailed(g_commandQueue->Signal(g_fence.Get(), currentFenceValue));

	// 更新帧索引
	g_frameIndex = g_swapChain->GetCurrentBackBufferIndex();

	// 等待GPU
	if (g_fence->GetCompletedValue() < g_fenceValues[g_frameIndex])
	{
		ThrowIfFailed(g_fence->SetEventOnCompletion(g_fenceValues[g_frameIndex], g_fenceEvent.Get()));
		WaitForSingleObjectEx(g_fenceEvent.Get(), INFINITE, FALSE);
	}

	// Set the fence value for the next frame.
	g_fenceValues[g_frameIndex] = currentFenceValue + 1;
}

void GameRHI::CreateFrameResource()
{
	for (UINT n = 0; n < FrameCount; n++)
	{
		ThrowIfFailed(g_swapChain->GetBuffer(n, IID_PPV_ARGS(g_renderTargets[n].GetAddressOf())));

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = g_backBufferFormat;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

		//获取渲染视图的地址
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptor(
			g_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
			static_cast<INT>(n), g_rtvDescriptorSize);

		g_device->CreateRenderTargetView(g_renderTargets[n].Get(), &rtvDesc, rtvDescriptor);
		NAME_D3D12_OBJECT_INDEXED(g_renderTargets, n);

		//创建命令分配器
		//ThrowIfFailed(g_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_commandAllocator[n])));
	}

	
}

void GameRHI::SetMSAA()
{
	if (g_MSAA)
	{
		g_MSAA = false;
	}
	else
	{
		g_MSAA = true;
	}
}



void GameRHI::SetFence()
{
	//创建一个围栏同步CPU与GPU
	ThrowIfFailed(g_device->CreateFence(g_fenceValues[g_frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_fence)));
	g_fenceValues[g_frameIndex]++;
	//g_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (g_fenceEvent.Get() == nullptr)
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}
	WaitForGPU();
}

std::wstring GameRHI::GetGameAssetPath()
{
	return g_assetsPath;
}

_Use_decl_annotations_
void GameRHI::ParseCommandLineArgs(WCHAR* argv[], int argc)
{
	for (int i = 1; i < argc; ++i)
	{
		if (_wcsnicmp(argv[i], L"-warp", wcslen(argv[i])) == 0 ||
			_wcsnicmp(argv[i], L"/warp", wcslen(argv[i])) == 0)
		{
			g_useWarpDevice = true;
			g_title = g_title + L" (WARP)";
		}
	}
}

std::wstring GameRHI::GetAssetFullPath(LPCWSTR assetName)
{
	return g_assetsPath + assetName;
}

IDXGIAdapter1* GameRHI::GetSupportedAdapter(ComPtr<IDXGIFactory6>& dxgiFactory, const D3D_FEATURE_LEVEL featureLevel)
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

void GameRHI::OnResize()
{
	assert(g_device);
	assert(g_swapChain);
	assert(g_commandAllocators);
	WaitForGPU();

	CreateWindowResources();
	UpdateViewport();
}

void GameRHI::OnMouseDown(WPARAM btnState, int x, int y)
{
	g_camera.g_LastMousePos.x = x;
	g_camera.g_LastMousePos.y = y;

	SetCapture(hwnd);
}

void GameRHI::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void GameRHI::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - g_camera.g_LastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - g_camera.g_LastMousePos.y));

		// Update angles based on input to orbit camera around box.
		g_camera.g_Theta += dx;
		g_camera.g_Phi += dy;

		// Restrict the angle mPhi.
		g_camera.g_Phi = ZMath::Clamp(g_camera.g_Phi, 0.1f, PI - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to 0.005 unit in the scene.
		float dx = 0.005f * static_cast<float>(x - g_camera.g_LastMousePos.x);
		float dy = 0.005f * static_cast<float>(y - g_camera.g_LastMousePos.y);

		// Update the camera radius based on input.
		g_camera.g_Radius += dx - dy;

		// Restrict the radius.
		g_camera.g_Radius = ZMath::Clamp(g_camera.g_Radius, 3.0f, 15.0f);
	}

	g_camera.g_LastMousePos.x = x;
	g_camera.g_LastMousePos.y = y;
}

float GameRHI::AspectRatio() const
{
	
	return static_cast<float>(g_width) / g_height;
	
}

void GameRHI::UpdateViewport()
{
	g_viewport.TopLeftX = 0;
	g_viewport.TopLeftY = 0;
	g_viewport.Width = static_cast<float>(g_width);
	g_viewport.Height = static_cast<float>(g_height);

	g_viewport.MinDepth = D3D12_MIN_DEPTH;
	g_viewport.MaxDepth = D3D12_MAX_DEPTH;

	g_scissorRect.left = 0;
	g_scissorRect.top = 0;
	g_scissorRect.right = g_width;
	g_scissorRect.bottom = g_height;
}

void GameRHI::IsDragging()
{
	g_Drag = true;
}

void GameRHI::NotDragging()
{
	g_Drag = false;
}

bool GameRHI::GetDragSate()const
{
	return g_Drag;
}



void GameRHI::CreateGPUElement()
{
	ThrowIfFailed(CreateDXGIFactory2(0,IID_PPV_ARGS(g_dxgiFactory.ReleaseAndGetAddressOf())));

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
		adapter = GetSupportedAdapter(g_dxgiFactory, featureLevels[i]);
		if (adapter != nullptr)
		{
			break;
		}
	}

	//创建GPU交互对象
	if (adapter != nullptr)
	{
		D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(g_device.ReleaseAndGetAddressOf()));
	}
}

void GameRHI::CreateCommandQueue()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(g_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(g_commandQueue.ReleaseAndGetAddressOf())));
}

void GameRHI::CreateSwapChain()
{
	ComPtr<IDXGISwapChain1> swapChain;
	DXGI_FORMAT backBufferFormat = NoSRGB(g_backBufferFormat);
	//交换链描述
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = FrameCount;
	swapChainDesc.Width = g_width;
	swapChainDesc.Height = g_height;
	swapChainDesc.Format = backBufferFormat;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	//指定窗口
	
	ThrowIfFailed(g_dxgiFactory->CreateSwapChainForHwnd(
		g_commandQueue.Get(),
		hwnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		swapChain.GetAddressOf()
	));

	ThrowIfFailed(g_dxgiFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));
	//转换交换链信息，获取交换链索引
	ThrowIfFailed(swapChain.As(&g_swapChain));
}

void GameRHI::CreateRenderTargetViewDesCribeHeap()
{
	//创建渲染目标视图描述符堆描述
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = FrameCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	//创建渲染目标视图描述堆
	ThrowIfFailed(g_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(g_rtvDescriptorHeap.ReleaseAndGetAddressOf())));
	g_rtvDescriptorSize = g_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	//std::wstring strToDisplay = std::to_wstring(g_rtvDescriptorSize);
	//wcsncpy_s(DebugToDisplay, strToDisplay.c_str(), sizeof(DebugToDisplay) / sizeof(DebugToDisplay[0]));
}

void GameRHI::CreateDepthStencilViewDesCribeHeap()
{
	//创建深度模板描述堆描述
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	//创建深度模板描述符堆
	ThrowIfFailed(g_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(g_dsvDescriptorHeap.ReleaseAndGetAddressOf())));
}

void GameRHI::CreateDepthStencialBuffer()
{
	//创建清除深度模板缓冲区描述符
	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = g_depthstencilBufferFormat;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	/*D3D12_HEAP_TYPE_DEFAULT：用于存储 GPU 访问频繁的资源。
	D3D12_HEAP_TYPE_UPLOAD：用于存储 CPU 向 GPU 上传数据的资源。
	D3D12_HEAP_TYPE_READBACK：用于存储 GPU 向 CPU 读回数据的资源。*/

	//创建GPU频繁访问的堆，将深度缓冲区视图放入该堆
	CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	//创建资源描述，描述其用于深度模板视图
	CD3DX12_RESOURCE_DESC depthtex2D = CD3DX12_RESOURCE_DESC::Tex2D(g_depthstencilBufferFormat, g_width, g_height, 1, 1);
	depthtex2D.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	//提交深度模板缓冲区资源
	ThrowIfFailed(g_device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&depthtex2D,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(g_depthStencilBuffer.ReleaseAndGetAddressOf())));

	//创建深度模板缓冲区描述符
	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc{};
	depthStencilDesc.Format = g_depthstencilBufferFormat;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

	//创建深度模板缓冲区
	g_device->CreateDepthStencilView(g_depthStencilBuffer.Get(), &depthStencilDesc, g_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

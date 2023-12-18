#pragma once
#include "../GameHelper/GameHelper.h"
#include "../Common/ZHEngineTimer.h"
#include "Camera.h"
#include "GameWindowApplication.h"




class GameRHI
{
public:
	GameRHI(UINT width, UINT height, std::wstring name, 
		DXGI_FORMAT backBufferFormat= DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT);
	virtual ~GameRHI();

	//Windows


	//DXRHI
public:
	ID3D12Device*				GetD3DDevice() const { return g_device.Get(); }
	IDXGISwapChain3*			GetSwapChain() const { return g_swapChain.Get(); }
	IDXGIFactory6*				GetDXGIFactory() const { return g_dxgiFactory.Get(); }
	ID3D12Resource*				GetRenderTarget() const { return g_renderTargets[g_frameIndex].Get(); }
	ID3D12Resource*				GetDepthStencil() const { return g_depthStencilBuffer.Get(); }
	ID3D12CommandQueue*			GetCommandQueue() const { return g_commandQueue.Get(); }
	ID3D12CommandAllocator*		GetCommandAllocator() const { return g_commandAllocators[g_frameIndex].Get(); }
	ID3D12GraphicsCommandList*	GetCommandList() const { return g_commandList.Get(); }
	D3D12_VIEWPORT              GetScreenViewport() const { return g_viewport; }
	D3D12_RECT                  GetScissorRect() const { return g_scissorRect; }
	UINT                        GetCurrentFrameIndex() const { return g_frameIndex; }

	CD3DX12_CPU_DESCRIPTOR_HANDLE GetRenderTargetView() const
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(
			g_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
			static_cast<INT>(g_frameIndex), g_rtvDescriptorSize);
	}
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(g_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	}
public:
	//State
	bool g_MSAA = true;
	static const UINT FrameCount = 3;
	//BackGround
	float clearColor[4] = { 0,0,0,1 };

public:
	void SetWindow();
	void CreateDeviceResources();
	void CreateWindowResources();
	void WaitForGPU();
	void MoveToNextFrame();
	void CreateFrameResource();
	void SetMSAA();
	

protected:
	void SetFence();

protected:
	
	UINT g_frameIndex;

	//D3DOBJ
	ComPtr<ID3D12Device> g_device;
	ComPtr<ID3D12CommandQueue> g_commandQueue;
	ComPtr<ID3D12GraphicsCommandList> g_commandList;
	ComPtr<ID3D12CommandAllocator> g_commandAllocators[FrameCount];

	//SwapChain
	ComPtr<IDXGIFactory6> g_dxgiFactory;
	ComPtr<IDXGISwapChain3> g_swapChain;
	ComPtr<ID3D12Resource> g_renderTargets[FrameCount];
	ComPtr<ID3D12Resource> g_depthStencilBuffer;
	//MSAA
	ComPtr<ID3D12Resource>          g_msaaRenderTarget;
	ComPtr<ID3D12Resource>          g_msaaDepthStencil;

	//Window Properties
	DXGI_FORMAT                  g_backBufferFormat;
	DXGI_FORMAT                  g_depthstencilBufferFormat;

	//RenderingOBJ
	ComPtr<ID3D12DescriptorHeap> g_rtvDescriptorHeap;
	ComPtr<ID3D12DescriptorHeap> g_dsvDescriptorHeap;
	//MSAA
	ComPtr<ID3D12DescriptorHeap>    g_msaaRTVDescriptorHeap;
	ComPtr<ID3D12DescriptorHeap>    g_msaaDSVDescriptorHeap;


	UINT g_rtvDescriptorSize;
	CD3DX12_VIEWPORT g_viewport;
	CD3DX12_RECT g_scissorRect;

	
	

	// Î§À¸
	Microsoft::WRL::Wrappers::Event g_fenceEvent;
	ComPtr<ID3D12Fence> g_fence;
	UINT64 g_fenceValues[FrameCount];

private:
	//TwoLevel
	void CreateMSAAResource();
	void CreateMSAAWindowResource();


	void CreateGPUElement();
	void CreateCommandQueue();
	void CreateSwapChain();
	void CreateRenderTargetViewDesCribeHeap();
	void CreateDepthStencilViewDesCribeHeap();
	void CreateDepthStencialBuffer();


	//Windows
public:
	HWND hwnd;


	virtual void OnInit() = 0;
	virtual void OnUpdate(ZHEngineTimer const& timer) = 0;
	virtual void OnRender() = 0;
	virtual void OnDestroy() = 0;
	virtual void Tick() = 0;

	virtual void OnMouseDown(WPARAM btnState, int x, int y);
	virtual void OnMouseUp(WPARAM btnState, int x, int y);
	virtual void OnMouseMove(WPARAM btnState, int x, int y);


	float AspectRatio()const;

	UINT GetWidth() const { return g_width; }
	UINT GetHeight() const { return g_height; }
	const WCHAR* GetTitle() const { return g_title.c_str(); }
	std::wstring GetGameAssetPath();

	void ParseCommandLineArgs(_In_reads_(argc) WCHAR* argv[], int argc);

protected:
	std::wstring GetAssetFullPath(LPCWSTR assetName);
	IDXGIAdapter1* GetSupportedAdapter(ComPtr<IDXGIFactory6>& dxgiFactory, const D3D_FEATURE_LEVEL featureLevel);


	UINT g_width;
	UINT g_height;
	float g_aspectRatio;

	Camera g_camera;
	ZHEngineTimer g_timer;
	bool g_useWarpDevice;


private:
	std::wstring g_assetsPath;
	std::wstring g_title;

	
};


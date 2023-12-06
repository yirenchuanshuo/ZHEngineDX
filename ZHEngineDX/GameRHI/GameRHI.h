#pragma once
#include "../GameHelper/GameHelper.h"
#include "../Common/ZHEngineTimer.h"
#include "Camera.h"
#include "GameWindowApplication.h"




class GameRHI
{
public:
	GameRHI(UINT width, UINT height, std::wstring name);
	virtual ~GameRHI();

	//Windows


	//DXRHI
public:
	void SetWindow();
	void CreateDeviceResources();
	void CreateWindowResources();
	void WaitForGPU();
	void MoveToNextFrame();
	void CreateFrameResource();

protected:
	void SetFence();

protected:
	static const UINT FrameCount = 3;
	UINT g_frameIndex;

	//D3DOBJ
	ComPtr<ID3D12Device> g_device;
	ComPtr<ID3D12CommandQueue> g_commandQueue;
	ComPtr<ID3D12GraphicsCommandList> g_commandList;
	ComPtr<ID3D12CommandAllocator> g_commandAllocator[FrameCount];

	//SwapChain
	ComPtr<IDXGIFactory6> g_dxgiFactory;
	ComPtr<IDXGISwapChain3> g_swapChain;
	ComPtr<ID3D12Resource> g_renderTargets[FrameCount];
	ComPtr<ID3D12Resource> g_depthStencilBuffer;

	//Window Properties
	DXGI_FORMAT                  g_backBufferFormat;
	DXGI_FORMAT                  g_depthstencilBufferFormat;

	//RenderingOBJ
	ComPtr<ID3D12DescriptorHeap> g_rtvDescriptorHeap;
	ComPtr<ID3D12DescriptorHeap> g_dsvDescriptorHeap;
	UINT g_rtvDescriptorSize;
	CD3DX12_VIEWPORT g_viewport;
	CD3DX12_RECT g_scissorRect;

	
	

	// Î§À¸
	Microsoft::WRL::Wrappers::Event g_fenceEvent;
	ComPtr<ID3D12Fence> g_fence;
	UINT64 g_fenceValues[FrameCount];

private:
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
	int LoadImageDataFromFile(std::shared_ptr<BYTE>& imageData, D3D12_RESOURCE_DESC& resourceDescription, LPCWSTR filename, int& bytesPerRow);

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

	

	DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID);
	WICPixelFormatGUID GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID);
	int GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat);
};


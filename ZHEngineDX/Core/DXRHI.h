#pragma once
#include"../Common/CommonCore.h"
#include "../GameHelper/GameHelper.h"

using namespace Microsoft::WRL;
class DXRHI
{
public:
	static const unsigned int g_AllowTearing = 0x1;
	static const unsigned int g_EnableHDR = 0x2;

	DXRHI();
	~DXRHI();
	HWND hwnd;

	ID3D12Device* GetDevice() const { return g_Device.Get(); }
	IDXGIFactory6* GetDXGIFactory() const { return g_DxgiFactory.Get(); }
	ID3D12CommandQueue* GetCommandQueue() const { return g_CommandQueue.Get(); }
	ID3D12CommandAllocator* GetCommandAllocator() const { return g_CommandAllocators[g_frameIndex].Get(); }
	IDXGISwapChain3* GetSwapChain() const { return g_SwapChain.Get(); }
	ID3D12Resource* GetRenderTarget() const { return g_renderTargets[g_frameIndex].Get(); }
	UINT                        GetCurrentFrameIndex() const { return g_frameIndex; }
	

public:
	void CreateDeviceElement(UINT WindowWidth, UINT WindowHeight, HWND& hwnd);

private:
	IDXGIAdapter1* GetSupportedAdapter(ComPtr<IDXGIFactory6>& dxgiFactory, const D3D_FEATURE_LEVEL featureLevel);
	void CreateCommandQueue();
	void CreateRenderTargetViewDesCribeHeap();
	void CreateDepthStencilViewDesCribeHeap();
	void CreateFrameResource();
	
private:
	static const UINT FrameCount = 3;

	ComPtr<IDXGIFactory6> g_DxgiFactory;
	ComPtr<ID3D12Device> g_Device;
	ComPtr<ID3D12CommandQueue> g_CommandQueue;
	ComPtr<ID3D12CommandAllocator> g_CommandAllocators[FrameCount];


	ComPtr<IDXGISwapChain3> g_SwapChain;
	ComPtr<ID3D12Resource> g_renderTargets[FrameCount];

	ComPtr<ID3D12DescriptorHeap> g_rtvDescriptorHeap;
	ComPtr<ID3D12DescriptorHeap> g_dsvDescriptorHeap;
	UINT g_rtvDescriptorSize;

	// Î§À¸
	UINT g_frameIndex;
	HANDLE g_fenceEvent;
	ComPtr<ID3D12Fence> g_fence;
	UINT64 g_fenceValues[FrameCount];
};
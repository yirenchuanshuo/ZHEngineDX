#pragma once
#include"../Game/Game.h"


class HelloGame :public Game
{
public:
	HelloGame(UINT width, UINT height, std::wstring name);

    virtual void OnInit();
    virtual void OnUpdate();
    virtual void OnRender();
    virtual void OnDestroy();

private:
    static const UINT FrameCount = 2;

    //管线对象
    ComPtr<IDXGISwapChain3> g_swapChain;
    ComPtr<ID3D12Device> g_device;
    ComPtr<ID3D12Resource> g_renderTargets[FrameCount];
    ComPtr<ID3D12CommandAllocator> g_commandAllocator;
    ComPtr<ID3D12CommandQueue> g_commandQueue;
    ComPtr<ID3D12DescriptorHeap> g_rtvHeap;
    ComPtr<ID3D12PipelineState> g_pipelineState;
    ComPtr<ID3D12GraphicsCommandList> g_commandList;
    UINT g_rtvDescriptorSize;


    // 同步对象
    UINT g_frameIndex;
    HANDLE g_fenceEvent;
    ComPtr<ID3D12Fence> g_fence;
    UINT64 g_fenceValue;

private:
    void LoadPipeline();
    void LoadAsset();
    void PopulateCommandList();
    void WaitForPreviousFrame();
};
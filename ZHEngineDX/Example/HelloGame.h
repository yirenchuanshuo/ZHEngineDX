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


    virtual void UpdateBackGround();
    virtual void UpdateConstantBuffer();

protected:
    virtual void UpdateMVP();

public:
    //BackGround
    float clearColor[4] = { 0.0f, 0.2f, 0.4f, 1.0f };

    bool isRAdd = true;
    bool isGAdd = true;
    bool isBAdd = true;

    //Data
    struct Vertex
    {
        Float3 position;
        FLinearColor color;
    };

    //ConstantBuffer
    struct SceneConstantBuffer
    {
        //内存对齐256;
       Float4x4 MVP;
       //Float4 offset;
    };

private:
    static const UINT FrameCount = 2;

    //管线对象
    CD3DX12_VIEWPORT g_viewport;
    CD3DX12_RECT g_scissorRect;
    ComPtr<IDXGISwapChain3> g_swapChain;
    ComPtr<ID3D12Device> g_device;
    ComPtr<ID3D12Resource> g_renderTargets[FrameCount];
    ComPtr<ID3D12CommandAllocator> g_commandAllocator;
    ComPtr<ID3D12CommandQueue> g_commandQueue;
    ComPtr<ID3D12RootSignature> g_rootSignature;

    ComPtr<ID3D12DescriptorHeap> g_rtvHeap;
    ComPtr<ID3D12DescriptorHeap> g_dsvHeap;
    ComPtr<ID3D12DescriptorHeap> g_cbvHeap;


    ComPtr<ID3D12PipelineState> g_pipelineState;
    ComPtr<ID3D12GraphicsCommandList> g_commandList;
    UINT g_rtvDescriptorSize;

    //资源Buffer
    ComPtr<ID3D12Resource> g_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW g_vertexBufferView;

    ComPtr<ID3D12Resource> g_indexBuffer;
    D3D12_INDEX_BUFFER_VIEW g_indexBufferView;

    ComPtr<ID3D12Resource> g_constantBuffer;
    SceneConstantBuffer g_constantBufferData;
    UINT8* g_pCbvDataBegin;
    ComPtr<ID3D12Resource> g_depthStencilBuffer;
    

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
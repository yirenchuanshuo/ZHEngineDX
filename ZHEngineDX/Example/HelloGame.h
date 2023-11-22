#pragma once
#include "../Game/Game.h"
#include "../Mesh/Light.h"
#include "../Mesh/OBJ.h"
#include "../Mesh/StaticMesh.h"
#include "../Texture/Texture.h"

class HelloGame :public Game
{
public:
	HelloGame(UINT width, UINT height, std::wstring name);

    virtual void OnInit()override;
    virtual void OnUpdate()override;
    virtual void OnRender()override;
    virtual void OnDestroy()override;



    virtual void UpdateBackGround();
    virtual void UpdateLight();
    virtual void UpdateConstantBuffer();

protected:
    virtual void UpdateMVP();
    virtual void LoadTexture();

public:
    //BackGround
    float clearColor[4] = { 0.0f, 0.2f, 0.4f, 1.0f };

    bool isRAdd = true;
    bool isGAdd = true;
    bool isBAdd = true;

    //Data
    StaticMesh Mesh;
    DirectionLight light;
    float lightangle=0.0f;

    //ConstantBuffer
    struct SceneConstantBuffer
    {
        //内存对齐256;
        Float4x4 ObjectToWorld;
        Float4x4 MVP;
        FLinearColor lightColor;
        Float4 lightDirection;
        Float3 viewPosition;
        float pad2;
       //Float4 offset;
    };

private:
    static const UINT FrameCount = 2;
    

    //渲染预备资源
    CD3DX12_VIEWPORT g_viewport;
    CD3DX12_RECT g_scissorRect;
    ComPtr<IDXGISwapChain3> g_swapChain;
    ComPtr<ID3D12Device> g_device;
    ComPtr<ID3D12Resource> g_renderTargets[FrameCount];
    ComPtr<ID3D12CommandAllocator> g_commandAllocator[FrameCount];
    ComPtr<ID3D12CommandQueue> g_commandQueue;
    ComPtr<ID3D12RootSignature> g_rootSignature;

    ComPtr<ID3D12DescriptorHeap> g_rtvHeap;
    ComPtr<ID3D12DescriptorHeap> g_dsvHeap;
    ComPtr<ID3D12DescriptorHeap> g_cbvsrvHeap;


    ComPtr<ID3D12PipelineState> g_pipelineState;
    ComPtr<ID3D12GraphicsCommandList> g_commandList;
    UINT g_rtvDescriptorSize;
    UINT g_cbvsrvDescriptorSize;

    //资源Buffer
    ComPtr<ID3D12Resource> g_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW g_vertexBufferView;

    ComPtr<ID3D12Resource> g_indexBuffer;
    D3D12_INDEX_BUFFER_VIEW g_indexBufferView;

    ComPtr<ID3D12Resource> g_constantBuffer;
    SceneConstantBuffer g_constantBufferData;

    

    std::vector<UTexture> g_textures;

    UINT8* g_pCbvDataBegin;
    ComPtr<ID3D12Resource> g_depthStencilBuffer;

    
    

    // 围栏
    UINT g_frameIndex;
    HANDLE g_fenceEvent;
    ComPtr<ID3D12Fence> g_fence;
    UINT64 g_fenceValues[FrameCount];

private:
    void LoadPipeline();
    void LoadAsset();
    void PopulateCommandList();
    void MoveToNextFrame();
    void WaitForGPU();


    void CreateGPUElement(ComPtr<IDXGIFactory6>& gDxgiFactory);
    void CreateCommandQueue();
    void CreateSwapChain(ComPtr<IDXGISwapChain1>& swapChain, ComPtr<IDXGIFactory6>& gDxgiFactory);
    void CreateRenderTargetViewDesCribeHeap();
    void CreateDepthStencilViewDesCribeHeap();
    void CreateConstantBufferDesCribeHeap();
    void CreateFrameResource(ComPtr<IDXGISwapChain1>& swapChain,CD3DX12_CPU_DESCRIPTOR_HANDLE& rtvHandle);


    void CreateRootSignature();
    D3D12_STATIC_SAMPLER_DESC CreateSamplerDesCribe(UINT index);
    void CreatePSO(ComPtr<ID3DBlob>& vertexShader, ComPtr<ID3DBlob>& pixelShader);
    void UpLoadVertexAndIndexToHeap(CD3DX12_HEAP_PROPERTIES& heapProperties, CD3DX12_RANGE& readRange,const UINT vertexBufferSize, const UINT indexBufferSize);
    void UpLoadDepthStencialBuffer();
    void UpLoadConstantBuffer(CD3DX12_HEAP_PROPERTIES& heapProperties, CD3DX12_RANGE& readRange);
    void UpLoadShaderResource();
    void SetFence();
};
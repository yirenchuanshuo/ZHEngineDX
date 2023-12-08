#pragma once
#include "../GameRHI/GameRHI.h"
#include "../Mesh/Light.h"
#include "../Mesh/OBJ.h"
#include "../Rendering/RenderActor.h"
#include "../Texture/Texture.h"
#include "../Rendering/Shader.h"



class HelloGame :public GameRHI
{
public:
	HelloGame(UINT width, UINT height, std::wstring name);

    virtual void OnInit()override;
    virtual void OnUpdate(ZHEngineTimer const& timer)override;
    virtual void OnRender()override;
    virtual void OnDestroy()override;
    virtual void Tick()override;



    virtual void UpdateBackGround();
    virtual void UpdateLight();
    virtual void UpdateConstantBuffer();

protected:
    virtual void UpdateMVP();
    virtual void LoadTexture();

public:
    
    //{ 0.0f, 0.2f, 0.4f, 1.0f };

    bool isRAdd = true;
    bool isGAdd = true;
    bool isBAdd = true;

    //Data
    std::unique_ptr<RenderActor> ModeActor;
    StaticMesh Sky;
    DirectionLight light;
    float lightangle=0.0f;

    //ConstantBuffer
    struct SceneConstantBuffer
    {
        //内存对齐256;
        Float4x4 ObjectToWorld;
        Float4x4 VP;
        Float4x4 MVP;
        FLinearColor lightColor;
        Float4 lightDirection;
        Float4 cameraPosition;
    };

    

private:
    
    //渲染预备资源
    ComPtr<ID3D12RootSignature> g_rootSignature;
    ComPtr<ID3D12DescriptorHeap> g_cbvsrvHeap;
    ComPtr<ID3D12DescriptorHeap> g_skycbvsrvHeap;
    ComPtr<ID3D12PipelineState> g_pipelineState;
    ComPtr<ID3D12PipelineState> g_skyPipelineState;
    
    UINT g_cbvsrvDescriptorSize;


    //资源堆
    ComPtr<ID3D12Heap>					g_textureHeap;
    ComPtr<ID3D12Heap>					g_textureUpLoadHeap;
    //资源Buffer
    ComPtr<ID3D12Resource> g_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW g_vertexBufferView;

    ComPtr<ID3D12Resource> g_indexBuffer;
    D3D12_INDEX_BUFFER_VIEW g_indexBufferView;

    ComPtr<ID3D12Resource> g_constantBuffer;
    SceneConstantBuffer g_constantBufferData;

    //渲染资源
    std::vector<UShader> g_shaders;
    UShader g_skyShader;

    std::vector<UTexture> g_textures;
    UINT8* g_pCbvDataBegin;
    


private:
    //One Level
    void LoadPipeline();
    void LoadAsset();
    void PopulateCommandList();
    
    //Two Level
    void PreperShader();
    

    //Three Level
    void CreateConstantBufferDesCribeHeap();
    void CreateRootSignature();
    D3D12_STATIC_SAMPLER_DESC CreateSamplerDesCribe(UINT index);
    void CreateShader(ComPtr<ID3DBlob>& vertexShader, ComPtr<ID3DBlob>& pixelShader,std::wstring VSFileName, std::wstring PSFileName);
    void CreatePSO();
    void UpLoadVertexAndIndexToHeap(CD3DX12_HEAP_PROPERTIES& heapProperties, CD3DX12_RANGE& readRange, std::unique_ptr<RenderActor>& Actor);
    void UpLoadConstantBuffer(CD3DX12_HEAP_PROPERTIES& heapProperties, CD3DX12_RANGE& readRange);
    void UpLoadShaderResource();
    
    
};
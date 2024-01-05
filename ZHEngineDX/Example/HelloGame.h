#pragma once
#include "../GameRHI/GameRHI.h"
#include "../Mesh/Light.h"
#include "../Mesh/OBJ.h"
#include "../Rendering/RenderActor.h"
#include "../Texture/Texture.h"
#include "../Rendering/Material.h"




class HelloGame :public GameRHI
{
public:
	HelloGame(UINT width, UINT height, const std::wstring& name);

    virtual void OnInit()override;
    virtual void OnUpdate(ZHEngineTimer const& timer)override;
    virtual void OnResize()override;
    virtual void OnRender()override;
    virtual void OnDestroy()override;
    virtual void Tick()override;



    virtual void UpdateBackGround();
    virtual void UpdateLight();
    virtual void UpdateConstantBuffer();

    virtual void CreateFrameResource();

protected:
    virtual void UpdateMVP();
    virtual void LoadTexture();
    virtual void LoadSkyCubeMap();

public:
    
    //{ 0.0f, 0.2f, 0.4f, 1.0f };

    bool isRAdd = true;
    bool isGAdd = true;
    bool isBAdd = true;

    //RenderActor
    std::shared_ptr<RenderActor> ModeActor;
    std::shared_ptr<URenderActorInterface> ModeInterface;
    std::shared_ptr<RenderActor> SkyActor;
    std::shared_ptr<URenderActorInterface> SkyInterface;
    std::shared_ptr<RenderActor> GroundActor;
    std::shared_ptr<URenderActorInterface> GroundInterface;

    //Shader
    std::shared_ptr<UShader> ModeShader;
    std::shared_ptr<UShader> SkyShader;


    //Light
    DirectionLight light;
    float lightangle=0.0f;

    //ConstantBuffer
    struct SceneConstantBuffer
    {
        //内存对齐256;
        Float4x4 VP;
        FLinearColor lightColor;
        Float4 lightDirection;
        Float4 cameraPosition;
    };

    

private:
    
    //渲染预备资源
    
    ComPtr<ID3D12DescriptorHeap> g_samplerHeap;
    
    UINT g_cbvsrvDescriptorSize=0u;
    UINT g_currentFrameResourceIndex;
    UINT g_frameCounter;

    //资源堆
    D3D12_RESOURCE_DESC                 g_ResourceBufferDesc;

    //资源Buffer
    ComPtr<ID3D12Resource> g_UniformconstantBuffer;
    SceneConstantBuffer g_UniformconstantBufferData;

    //渲染资源

    std::vector<UTexture> g_Uniformtextures;
    UTexture g_SkyCubeMap;
    std::shared_ptr<UINT8> g_pCbvDataBegin;


private:
    //One Level
    void LoadPipeline();
    void LoadAsset();
    void PopulateCommandList();
    
    
    //Two Level
    void PreperRenderActor();

    //Three Level
    void CreateConstantBufferDesCribeHeap();
    void CreateSamplerDescribeHeap();
    void CreateRootSignature();
    D3D12_SAMPLER_DESC CreateSamplerDesCribe(UINT index);
    void CreateShader(ComPtr<ID3DBlob>& vertexShader, ComPtr<ID3DBlob>& pixelShader,std::wstring VSFileName, std::wstring PSFileName);
    void CreatePSO();
    void UpLoadVertexAndIndexToHeap(std::shared_ptr<RenderActor>& Actor) const;
    void UpLoadConstantBuffer(UINT FrameIndex);
    void UpLoadShaderResource();
    
};
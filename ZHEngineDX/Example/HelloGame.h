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

    //Data
    std::unique_ptr<RenderActor> ModeActor;
    std::unique_ptr<RenderActor> SkyActor;
    std::unique_ptr<RenderActor> GroundActor;
    DirectionLight light;
    float lightangle=0.0f;

    //ConstantBuffer
    struct SceneConstantBuffer
    {
        //�ڴ����256;
        Float4x4 VP;
        FLinearColor lightColor;
        Float4 lightDirection;
        Float4 cameraPosition;
    };

    

private:
    
    //��ȾԤ����Դ
    
    ComPtr<ID3D12DescriptorHeap> g_samplerHeap;
    
    UINT g_cbvsrvDescriptorSize=0u;


    //��Դ��
    D3D12_RESOURCE_DESC                 g_ResourceBufferDesc;

    //��ԴBuffer
    ComPtr<ID3D12Resource> g_UniformconstantBuffer;
    SceneConstantBuffer g_UniformconstantBufferData;

    //��Ⱦ��Դ
    std::vector<UMaterial> g_materials;
    UMaterial g_skyMaterial;

    std::vector<UTexture> g_textures;
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
    void UpLoadVertexAndIndexToHeap(const std::unique_ptr<RenderActor>& Actor) const;
    void UpLoadConstantBuffer(UINT FrameIndex);
    void UpLoadShaderResource();
    
};
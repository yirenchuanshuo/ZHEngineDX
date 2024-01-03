#pragma once
#include "../Mesh/StaticMesh.h"
#include "../GameHelper/GameHelper.h"
#include "../GameRHI/GameRHI.h"
#include "Material.h"


struct ObjectConstantBuffer
{
    Float4x4 ObjectToWorld  = ZMath::Float4x4Identity();
    Float4x4 ObjectToClip = ZMath::Float4x4Identity();
};

class RenderActor
{
public:
    RenderActor();

    //Function
    void SetPosition(FVector4& Pos) { Position = Pos; }
    FVector4 GetPosition() { return Position; }
    

    //RHI
    virtual void Init(ID3D12Device* pDevice, const wchar_t* shaderfile, const char* vsout, const char* psout, EBlendMode blend);
    void LoadMesh(std::string filepath);
    void SetTextures(UTexture& Texture);
    void UpdateMVP(FMatrix4x4 &VP);
    void AddHandleOffsetNum();



    void RecordCommands(ID3D12Device* pDevice,ID3D12DescriptorHeap* pSamplerDescriptorHeap, UINT cbvSrvDescriptorSize) const ;


    void SetPipleLineState(ID3D12Device* pDevice,D3D12_GRAPHICS_PIPELINE_STATE_DESC& PSODesc);
    void SetRootSignature(ID3D12Device* pDevice, CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& rootSignatureDesc);
    void UpLoadShaderResource(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList,D3D12_SHADER_RESOURCE_VIEW_DESC& SrvDesc);
    void CreateConstantBufferView(ID3D12Device* pDevice);
    void UpLoadConstantBuffer();


    ID3D12GraphicsCommandList* GetBundle()const { return g_bundle.Get(); }


    ID3D12DescriptorHeap* GetCbvSrvHeap() { return cbvsrvHeap.Get(); }
    ID3D12DescriptorHeap** GetCbvSrvHeapAddress() { return cbvsrvHeap.GetAddressOf(); }
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& GetCbvSrvHeapRef(){ return cbvsrvHeap;}
    D3D12_CPU_DESCRIPTOR_HANDLE GetCbvSrvHandle() { return cbvsrvHeap->GetCPUDescriptorHandleForHeapStart(); }

    size_t GetMeshVerticesByteSize() { return Mesh->GetVerticesByteSize(); }
    size_t GetMeshIndicesByteSize() { return Mesh->GetIndicesByteSize(); }
    Vertex* GetMeshVerticesData() { return Mesh->GetVerticesData(); }
    UINT* GetMeshIndicesData() { return Mesh->GetIndicesData(); }
    EBlendMode GetActorMaterialBlendMode() { return Material->GetMateriBlendMode(); }

    UINT GetCbvSrvHeapDescriptorsNum(UINT UniformDataTypeNums) { return Material->textures.size() + 1 + UniformDataTypeNums; }

    CD3DX12_CPU_DESCRIPTOR_HANDLE GetCbvSrvAvailableHandle();

public:
    ObjectConstantBuffer g_ObjectConstantBufferData;
   
    Microsoft::WRL::ComPtr<ID3D12Resource> g_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW g_vertexBufferView;

    Microsoft::WRL::ComPtr<ID3D12Resource> g_indexBuffer;
    D3D12_INDEX_BUFFER_VIEW g_indexBufferView;
    
    
private:
    std::unique_ptr<StaticMesh> Mesh;
    std::unique_ptr<UMaterial> Material;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> cbvsrvHeap;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> PipeLineState;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> g_bundleAllocator;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> g_bundle;

    Microsoft::WRL::ComPtr<ID3D12Resource> g_ObjectConstantBuffer;

    std::shared_ptr<UINT8> pObjectCbvDataBegin;
    UINT cbvsrvDescriptorSize;
    UINT HandleOffsetNum;

    FVector4 Position;
};
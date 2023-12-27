#pragma once
#include "../Mesh/StaticMesh.h"
#include "../GameHelper/GameHelper.h"
#include "../GameRHI/GameRHI.h"
#include "Material.h"

class RenderActor
{
public:
    RenderActor();

    void Init(ID3D12Device* pDevice);
    void RecordCommands(ID3D12Device* pDevice,ID3D12RootSignature* pRootSignature, ID3D12PipelineState* pPipleLineState, 
         ID3D12DescriptorHeap* pSamplerDescriptorHeap, UINT cbvSrvDescriptorSize) const ;

    ID3D12GraphicsCommandList* GetBundle()const { return g_bundle.Get(); }


    ID3D12DescriptorHeap* GetCbvSrvHeap() { return cbvsrvHeap.Get(); }
    ID3D12DescriptorHeap** GetCbvSrvHeapAddress() { return cbvsrvHeap.GetAddressOf(); }
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& GetCbvSrvHeapRef(){ return cbvsrvHeap;}
    D3D12_CPU_DESCRIPTOR_HANDLE GetCbvSrvHandle() { return cbvsrvHeap->GetCPUDescriptorHandleForHeapStart(); }


public:
	std::unique_ptr<StaticMesh> Mesh;
    std::unique_ptr<UMaterial> Material;

    Microsoft::WRL::ComPtr<ID3D12Resource> g_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW g_vertexBufferView;

    Microsoft::WRL::ComPtr<ID3D12Resource> g_indexBuffer;
    D3D12_INDEX_BUFFER_VIEW g_indexBufferView;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> cbvsrvHeap;

private:
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> g_bundleAllocator;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> g_bundle;
};
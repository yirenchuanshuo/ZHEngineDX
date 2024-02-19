#pragma once
#include "../Common/CommonCore.h"
#include "../GameHelper/GameHelper.h"
#include "Material.h"

class PostRenderActor 
{
public:
	PostRenderActor(ID3D12Device* pDevice,UINT width,UINT height,DXGI_FORMAT format);

	void OnResize(UINT newWidth,UINT newHeight);
	void ApplyPostProcess(ID3D12GraphicsCommandList* cmdList, ID3D12PipelineState* PSO01,
		ID3D12PipelineState* PSO02, ID3D12Resource* renderTarget,int blurCount);

public:
	ID3D12DescriptorHeap* GetPostCbvSrvUavHeap() { return g_PostCbvSrvUavHeap.Get(); }
	ID3D12DescriptorHeap** GetPostCbvSrvUavHeapAddress() { return g_PostCbvSrvUavHeap.GetAddressOf(); }

	void UpLoadShaderResource(UINT DescriptorSize);
	//void SetMaterial(ID3D12Device* pDevice, std::shared_ptr<UShader>& shader);


private:
	void BuildResources();
	void BuildDescriptors();
	std::vector<float> CalcGaussWeights(float sigma);
	


private:
	UINT g_width;
	UINT g_height;
	DXGI_FORMAT g_format;

	Microsoft::WRL::ComPtr<ID3D12Device> g_Device;
	
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> g_PostCbvSrvUavHeap;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> g_PipeLineState;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;

	std::shared_ptr<UMaterial> PostMaterial;


	CD3DX12_CPU_DESCRIPTOR_HANDLE g_Post0CpuSrvHandle;
	CD3DX12_CPU_DESCRIPTOR_HANDLE g_Post0CpuUavHandle;

	CD3DX12_CPU_DESCRIPTOR_HANDLE g_Post1CpuSrvHandle;
	CD3DX12_CPU_DESCRIPTOR_HANDLE g_Post1CpuUavHandle;

	CD3DX12_GPU_DESCRIPTOR_HANDLE g_Post0GpuSrvHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE g_Post0GpuUavHandle;

	CD3DX12_GPU_DESCRIPTOR_HANDLE g_Post1GpuSrvHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE g_Post1GpuUavHandle;

	Microsoft::WRL::ComPtr<ID3D12Resource> g_PostMap0 = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> g_PostMap1 = nullptr;
};
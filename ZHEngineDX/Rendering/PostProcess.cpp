#include "PostProcess.h"

PostRenderActor::PostRenderActor(ID3D12Device* pDevice, UINT width, UINT height, DXGI_FORMAT format):
	g_width(width),
	g_height(height),
	g_format(format)
{
	g_Device = pDevice;
	BuildResources();
}

void PostRenderActor::OnResize(UINT newWidth, UINT newHeight)
{
	if ((g_width != newWidth) || (g_height != newHeight))
	{
		g_width = newWidth;
		g_height = newHeight;

		BuildResources();
		BuildDescriptors();
	}
}


void PostRenderActor::UpLoadShaderResource(UINT DescriptorSize)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE CpuStartHandle(g_PostCbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart());
	g_Post0CpuSrvHandle = CpuStartHandle;
	g_Post0CpuUavHandle = CpuStartHandle.Offset(1, DescriptorSize);
	g_Post1CpuSrvHandle = CpuStartHandle.Offset(1, DescriptorSize);
	g_Post1CpuUavHandle = CpuStartHandle.Offset(1, DescriptorSize);


	CD3DX12_GPU_DESCRIPTOR_HANDLE GpuStartHandle(g_PostCbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart());
	g_Post0GpuSrvHandle = GpuStartHandle;
	g_Post0GpuUavHandle = GpuStartHandle.Offset(1, DescriptorSize);
	g_Post1GpuSrvHandle = GpuStartHandle.Offset(1, DescriptorSize);
	g_Post1GpuUavHandle = GpuStartHandle.Offset(1, DescriptorSize);

	BuildDescriptors();
}

void PostRenderActor::BuildResources()
{
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = g_width;
	texDesc.Height = g_height;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = g_format;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;


	CD3DX12_HEAP_PROPERTIES Property = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	ThrowIfFailed(g_Device->CreateCommittedResource(
		&Property,
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&g_PostMap0)));

	ThrowIfFailed(g_Device->CreateCommittedResource(
		&Property,
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&g_PostMap1)));

}

void PostRenderActor::BuildDescriptors()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = g_format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};

	uavDesc.Format = g_format;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	g_Device->CreateShaderResourceView(g_PostMap0.Get(), &srvDesc, g_Post0CpuSrvHandle);
	g_Device->CreateUnorderedAccessView(g_PostMap0.Get(), nullptr, &uavDesc, g_Post0CpuUavHandle);

	g_Device->CreateShaderResourceView(g_PostMap1.Get(), &srvDesc, g_Post1CpuSrvHandle);
	g_Device->CreateUnorderedAccessView(g_PostMap1.Get(), nullptr, &uavDesc, g_Post1CpuUavHandle);
}

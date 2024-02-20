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

void PostRenderActor::ApplyPostProcess(ID3D12GraphicsCommandList* cmdList, ID3D12PipelineState* PSO01, ID3D12PipelineState* PSO02, ID3D12Resource* renderTarget, int blurCount)
{
	auto weights = CalcGaussWeights(2.5f);
	int blurRadius = static_cast<int>(weights.size() / 2);

	cmdList->SetComputeRootSignature(rootSignature.Get());

	cmdList->SetComputeRoot32BitConstants(0, 1, &blurRadius, 0);
	cmdList->SetComputeRoot32BitConstants(0, static_cast<UINT>(weights.size()), weights.data(), 1);

	CD3DX12_RESOURCE_BARRIER TargetToCopy = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget,
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);

	CD3DX12_RESOURCE_BARRIER PostMap0CommonToCopy = CD3DX12_RESOURCE_BARRIER::Transition(g_PostMap0.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);

	CD3DX12_RESOURCE_BARRIER PostMap0CopyToRead = CD3DX12_RESOURCE_BARRIER::Transition(g_PostMap0.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);

	CD3DX12_RESOURCE_BARRIER PostMap1CommonToAccess = CD3DX12_RESOURCE_BARRIER::Transition(g_PostMap1.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	cmdList->ResourceBarrier(1, &TargetToCopy);

	cmdList->ResourceBarrier(1, &PostMap0CommonToCopy);

	cmdList->CopyResource(g_PostMap0.Get(), renderTarget);

	cmdList->ResourceBarrier(1, &PostMap0CopyToRead);

	cmdList->ResourceBarrier(1, &PostMap1CommonToAccess);

	CD3DX12_RESOURCE_BARRIER PostMap0ReadToAccess = CD3DX12_RESOURCE_BARRIER::Transition(g_PostMap0.Get(),
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	CD3DX12_RESOURCE_BARRIER PostMap1ReadToAccess = CD3DX12_RESOURCE_BARRIER::Transition(g_PostMap1.Get(),
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	CD3DX12_RESOURCE_BARRIER PostMap0AccessToRead = CD3DX12_RESOURCE_BARRIER::Transition(g_PostMap0.Get(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ);

	CD3DX12_RESOURCE_BARRIER PostMap1AccessToRead = CD3DX12_RESOURCE_BARRIER::Transition(g_PostMap1.Get(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ);


	for (int i = 0; i < blurCount; ++i)
	{

		cmdList->SetPipelineState(PSO01);

		cmdList->SetComputeRootDescriptorTable(1, g_Post0GpuSrvHandle);
		cmdList->SetComputeRootDescriptorTable(2, g_Post1GpuUavHandle);

		// How many groups do we need to dispatch to cover a row of pixels, where each
		// group covers 256 pixels (the 256 is defined in the ComputeShader).
		UINT numGroupsX = (UINT)ceilf(g_width / 256.0f);
		cmdList->Dispatch(numGroupsX, g_height, 1);

		cmdList->ResourceBarrier(1, &PostMap0ReadToAccess);

		cmdList->ResourceBarrier(1, &PostMap1AccessToRead);

		

		cmdList->SetPipelineState(PSO02);

		cmdList->SetComputeRootDescriptorTable(1, g_Post1GpuSrvHandle);
		cmdList->SetComputeRootDescriptorTable(2, g_Post0GpuUavHandle);

		// How many groups do we need to dispatch to cover a column of pixels, where each
		// group covers 256 pixels  (the 256 is defined in the ComputeShader).
		UINT numGroupsY = (UINT)ceilf(g_height / 256.0f);
		cmdList->Dispatch(g_width, numGroupsY, 1);

		cmdList->ResourceBarrier(1, &PostMap0AccessToRead);

		cmdList->ResourceBarrier(1, &PostMap1ReadToAccess);
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

std::vector<float> PostRenderActor::CalcGaussWeights(float sigma)
{
	float twoSigma2 = 2.0f * sigma * sigma;

	// Estimate the blur radius based on sigma since sigma controls the "width" of the bell curve.
	// For example, for sigma = 3, the width of the bell curve is 
	int blurRadius = static_cast<int>(ceil(2.0f * sigma));

	assert(blurRadius <= 5);

	std::vector<float> weights;
	weights.resize(2 * blurRadius + 1);
	float weightSum = 0.0f;


	for (int i = -blurRadius; i <= blurRadius; i++)
	{
		float x = static_cast<float>(i);

		weights[i + blurRadius] = expf(-x*x/twoSigma2);
		weightSum = weights[i + blurRadius];
	}

	const int n = weights.size();
	for (int i = 0; i < n; i++)
	{
		weights[i] /= weightSum;
	}

	return weights;
}

void PostRenderActor::SetMaterial(ID3D12Device* pDevice, std::shared_ptr<UShader>& shader)
{
	PostMaterial = std::make_shared<UComputeMaterial>(shader);
}

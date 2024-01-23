#include "ShadowMap.h"

ShadowMap::ShadowMap(ID3D12Device* pDevice, UINT width, UINT height)
{
	mapWidth = width;
	mapHeight = height;

	BuildResource(pDevice);
}

UINT ShadowMap::GetWidth() const
{
	return mapWidth;
}

UINT ShadowMap::GetHeight() const
{
	return mapHeight;
}

ID3D12Resource* ShadowMap::GetShadowMapResource()
{
	return g_ShadowMap.Get();
}

void ShadowMap::BuildResource(ID3D12Device* pDevice)
{
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = mapWidth;
	texDesc.Height = mapHeight;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = mapFormat;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	CD3DX12_HEAP_PROPERTIES Properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	ThrowIfFailed(pDevice->CreateCommittedResource(
		&Properties,
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		&optClear,
		IID_PPV_ARGS(&g_ShadowMap)
	));
}

#pragma once
#include"../Common/CommonCore.h"
#include "../GameHelper/GameHelper.h"

class ShadowMap
{
public:
	ShadowMap(ID3D12Device* pDevice,UINT width, UINT height);

	UINT GetWidth()const;
	UINT GetHeight()const;

	ID3D12Resource* GetShadowMapResource();

private:
	void BuildResource(ID3D12Device* pDevice);

private:
	UINT mapWidth = 0;
	UINT mapHeight = 0;

	DXGI_FORMAT mapFormat = DXGI_FORMAT_R24G8_TYPELESS;
	Microsoft::WRL::ComPtr<ID3D12Resource> g_ShadowMap = nullptr;
};
#pragma once
#include "../Common/CommonCore.h"

class UTexture
{
public:
	
	void GenerateTextureData();

	LPCWSTR Filename;

	std::shared_ptr<BYTE> Data;
	int texBytesPerRow;
	int texSize;

	Microsoft::WRL::ComPtr<ID3D12Resource> Resource ;
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadHeap ;

	D3D12_RESOURCE_DESC texDesc;
	D3D12_SUBRESOURCE_DATA texData;

};
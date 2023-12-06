#pragma once
#include "../Common/CommonCore.h"

namespace Texture
{
	DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID);
	WICPixelFormatGUID GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID);
	int GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat);
	int LoadImageDataFromFile(std::shared_ptr<BYTE>& imageData, D3D12_RESOURCE_DESC& resourceDescription, LPCWSTR filename, int& bytesPerRow);
}

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
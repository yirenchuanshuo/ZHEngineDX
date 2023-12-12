#pragma once
#include "../Common/CommonCore.h"
#include "../Common/DDSTextureLoader.h"

namespace Texture
{
	DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID);
	WICPixelFormatGUID GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID);
	UINT GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat);
	UINT LoadImageDataFromFile(std::shared_ptr<BYTE>& imageData, D3D12_RESOURCE_DESC& resourceDescription, LPCWSTR filename, UINT& bytesPerRow);
}

class UTexture
{
public:
	
	void GenerateTextureData();

	LPCWSTR Filename;

	std::shared_ptr<BYTE> Data;
	UINT texBytesPerRow;
	UINT texSize;

	Microsoft::WRL::ComPtr<ID3D12Resource> Resource ;
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadHeap ;

	D3D12_RESOURCE_DESC texDesc;
	D3D12_SUBRESOURCE_DATA texData;

};
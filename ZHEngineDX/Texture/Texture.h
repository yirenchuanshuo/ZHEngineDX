#pragma once
#include "../Common/CommonCore.h"

class UTexture
{
public:
	

	LPCWSTR Filename;

	std::shared_ptr<BYTE> Data;
	int texBytesPerRow;
	int texSize;

	Microsoft::WRL::ComPtr<ID3D12Resource> Resource ;
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadHeap ;
};
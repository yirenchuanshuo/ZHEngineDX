#pragma once
#include "../Common/CommonCore.h"

class UTexture
{
public:
	std::string Name;

	std::wstring Filename;

	std::shared_ptr<BYTE> Data;

	Microsoft::WRL::ComPtr<ID3D12Resource> Resource ;
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadHeap ;
};
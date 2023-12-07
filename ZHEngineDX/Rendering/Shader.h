#pragma once
#include"../Common/CommonCore.h"

enum class EBlendMode
{
	Opaque,
	SkyBox
};

class UShader 
{
public:
	UShader(std::wstring shaderfile, LPCSTR vsout, LPCSTR psout);
public:
	Microsoft::WRL::ComPtr<ID3DBlob> vertexShader;
	Microsoft::WRL::ComPtr<ID3DBlob> pixelShader;
	std::wstring ShaderFileName;
	LPCSTR VSOutName;
	LPCSTR PSOutName;

	EBlendMode blendMode;

public:
	void CreateShader();
};
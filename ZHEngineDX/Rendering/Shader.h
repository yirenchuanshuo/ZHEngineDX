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
	UShader();
	UShader(std::wstring shaderfile, LPCSTR vsout, LPCSTR psout, EBlendMode blend);


	[[nodiscard]]ID3DBlob* GetVertexShader() const;
	[[nodiscard]]ID3DBlob* GetPixelShader() const;
	[[nodiscard]]EBlendMode GetBlendMode() const;
	
	EBlendMode blendMode;
private:
	Microsoft::WRL::ComPtr<ID3DBlob> vertexShader;
	Microsoft::WRL::ComPtr<ID3DBlob> pixelShader;
	std::wstring ShaderFileName;
	LPCSTR VSOutName;
	LPCSTR PSOutName;

public:
	void CompileShader();
};
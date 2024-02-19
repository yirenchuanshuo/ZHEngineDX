#pragma once
#include"../Common/CommonCore.h"
#include "../GameHelper/GameHelper.h"

enum class EBlendMode
{
	Opaque,
	SkyBox
};

enum class EShaderType
{
	Vertex,
	Pixel,
	Compute
};




class UShader
{
public:
	UShader();
	UShader(std::wstring shaderfile, LPCSTR outname,EShaderType ShaderType);


	EShaderType ShaderType;
private:
	std::wstring ShaderFileName;
	LPCSTR OutName;
	
	Microsoft::WRL::ComPtr<ID3DBlob> Shader;

public:
	[[nodiscard]] ID3DBlob* GetShader() const;
	LPCSTR GetOutName()const;
	std::wstring GetFilePath()const;

	void CompileShader();
};






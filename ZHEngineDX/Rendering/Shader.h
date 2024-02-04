#pragma once
#include"../Common/CommonCore.h"
#include "../GameHelper/GameHelper.h"

enum class EBlendMode
{
	Opaque,
	SkyBox
};


class UShader
{
public:
	UShader();
	UShader(std::wstring shaderfile);


protected:
	std::wstring ShaderFileName;

public:
	virtual void CompileShader() = 0;
};


class UNormalShader :UShader
{
public:
	UNormalShader();
	UNormalShader(std::wstring shaderfile, LPCSTR vsout, LPCSTR psout, EBlendMode blend);

	[[nodiscard]] ID3DBlob* GetVertexShader() const;
	[[nodiscard]] ID3DBlob* GetPixelShader() const;
	[[nodiscard]] EBlendMode GetBlendMode() const;

public:
	void CompileShader()override;

	EBlendMode blendMode;

private:
	Microsoft::WRL::ComPtr<ID3DBlob> vertexShader;
	Microsoft::WRL::ComPtr<ID3DBlob> pixelShader;

	LPCSTR VSOutName;
	LPCSTR PSOutName;

};





#pragma once
#include "../Common/CommonCore.h"
#include "../Rendering/Shader.h"
#include "../Texture/Texture.h"

class UMaterial
{
public:
	void CompileShader(std::wstring shaderfile, LPCSTR vsout, LPCSTR psout, EBlendMode blend);
	EBlendMode GetMateriBlendMode()const;
public:
	std::shared_ptr<UShader> shader;
	std::vector<UTexture> textures;

private:

};
#pragma once
#include "../Common/CommonCore.h"
#include "../Rendering/Shader.h"
#include "../Texture/Texture.h"

class UMaterial
{
public:
	UMaterial(std::shared_ptr<UShader>& shader);
public:
	EBlendMode GetMateriBlendMode()const;
public:
	std::shared_ptr<UShader> pShader;
	std::vector<UTexture> textures;

private:

};
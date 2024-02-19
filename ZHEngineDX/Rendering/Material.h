#pragma once
#include "../Common/CommonCore.h"
#include "../Rendering/Shader.h"
#include "../Texture/Texture.h"

class UMaterial
{
public:
	UMaterial(std::shared_ptr<UShader>& vertexshader, std::shared_ptr<UShader>& pixelshader);

public:

	//EBlendMode GetMateriBlendMode()const;
	
public:

	std::shared_ptr<UShader> pVertexShader;
	std::shared_ptr<UShader> pPixelShader;
	std::vector<UTexture> textures;

private:

};
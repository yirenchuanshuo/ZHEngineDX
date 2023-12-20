#include "Material.h"

void UMaterial::CompileShader(std::wstring shaderfile, LPCSTR vsout, LPCSTR psout, EBlendMode blend)
{
	UShader Shader(shaderfile, vsout, psout, EBlendMode::Opaque);
	Shader.CreateShader();
	this->shader = std::make_shared<UShader>(Shader);
}

EBlendMode UMaterial::GetMateriBlendMode() const
{
	return shader->blendMode;
}

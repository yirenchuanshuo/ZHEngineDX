#include "Material.h"

void UMaterial::CompileShader(std::wstring shaderfile, LPCSTR vsout, LPCSTR psout, EBlendMode blend)
{
	this->shader = std::make_shared<UShader>(shaderfile, vsout, psout, EBlendMode::Opaque);
	shader->CreateShader();
}

EBlendMode UMaterial::GetMateriBlendMode() const
{
	return shader->blendMode;
}

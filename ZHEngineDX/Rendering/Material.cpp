#include "Material.h"

UMaterial::UMaterial(std::shared_ptr<UShader>& shader):pShader(shader)
{

}

EBlendMode UMaterial::GetMateriBlendMode() const
{
	return pShader->blendMode;
}

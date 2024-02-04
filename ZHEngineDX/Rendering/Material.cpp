#include "Material.h"

UMaterial::UMaterial(std::shared_ptr<UNormalShader>& shader)
	:pShader(shader)
{

}

EBlendMode UMaterial::GetMateriBlendMode() const
{
	return pShader->blendMode;
}

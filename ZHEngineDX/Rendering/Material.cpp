#include "Material.h"

UMaterial::UMaterial(std::shared_ptr<UShader>& vertexshader, std::shared_ptr<UShader>& pixelshader)
	:pVertexShader(vertexshader),pPixelShader(pixelshader)
{

}



/*EBlendMode UMaterial::GetMateriBlendMode() const
{
	return pShader->blendMode;
}*/

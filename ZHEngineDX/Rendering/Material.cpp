#include "Material.h"

UMaterial::UMaterial(std::shared_ptr<UShader>& vertexshader, std::shared_ptr<UShader>& pixelshader)
	:pVertexShader(vertexshader),pPixelShader(pixelshader)
{

}



/*EBlendMode UMaterial::GetMateriBlendMode() const
{
	return pShader->blendMode;
}*/

UComputeMaterial::UComputeMaterial(std::shared_ptr<UShader>& computeshader)
	:ComputeShader(computeshader)
{

}

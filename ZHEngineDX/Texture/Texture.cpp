#include "Texture.h"

void UTexture::GenerateTextureData()
{
	texData.pData = Data.get();
	texData.RowPitch = texBytesPerRow;
	texData.SlicePitch = texBytesPerRow * texDesc.Height;
}

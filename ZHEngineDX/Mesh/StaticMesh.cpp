#include "StaticMesh.h"
#include "AssetLoad.h"

void StaticMesh::Load(std::string filePath)
{
	MeshAssetLoad(*this, filePath);
}

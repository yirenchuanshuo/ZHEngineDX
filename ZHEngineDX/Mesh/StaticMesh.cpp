#include "StaticMesh.h"
#include "AssetLoad.h"

void StaticMesh::Load(std::string filePath)
{
	MeshAssetLoad(*this, filePath);
}

size_t StaticMesh::GetVerticesSize()
{
	return vertices.size();
}

size_t StaticMesh::GetIndicesSize()
{
	return indices.size();
}

size_t StaticMesh::GetVerticesByteSize()
{
	return vertices.size()*sizeof(Vertex);
}

size_t StaticMesh::GetIndicesByteSize()
{
	return indices.size()*sizeof(UINT);
}

Vertex* StaticMesh::GetVerticesData()
{
	return vertices.data();
}

UINT* StaticMesh::GetIndicesData()
{
	return indices.data();
}

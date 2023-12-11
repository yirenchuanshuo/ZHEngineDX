#pragma once
#include "../Common/CommonCore.h"
#include "MeshBaseData.h"

class StaticMesh 
{
public:
	  template<class T>
	  friend void MeshAssetLoad(T& MeshAsset, std::string filePath);
public:

	void Load(std::string filePath);

	size_t GetVerticesSize();
	size_t GetIndicesSize();

	size_t GetVerticesByteSize();
	size_t GetIndicesByteSize();

	Vertex* GetVerticesData();
	UINT* GetIndicesData();

private:
	std::vector<Vertex> vertices;
	std::vector<UINT>indices;
};
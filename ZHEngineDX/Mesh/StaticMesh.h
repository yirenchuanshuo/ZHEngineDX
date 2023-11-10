#pragma once
#include "../Common/CommonCore.h"
#include "MeshBaseData.h"

class StaticMesh 
{
public:
	std::vector<Vertex> vertices;
	std::vector<UINT>indices;

	void Load(std::string filePath);
};
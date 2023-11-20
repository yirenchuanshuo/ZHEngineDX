#include "OBJ.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "AssetLoad.h"

void OBJ::Load(std::string path)
{
	std::ifstream file(path);
	if (!file.is_open())
	{
		return;
	}
	std::string line;
	//srand(time(NULL));
	int indexcount = 0;
	while (std::getline(file, line)) {
		if (line.substr(0, 2) == "v ")
		{
			std::istringstream s(line.substr(2));
			Float3 Positon;
			s >> Positon.x >> Positon.y >> Positon.z;
			positionlut.push_back(Positon);
		}
		else if (line.substr(0, 3) == "vt ")
		{
			std::istringstream s(line.substr(3));
			Float2 UV;
			s >> UV.x >> UV.y;
			UV.y = 1 - UV.y;
			uvlut.push_back(UV);
		}
		else if (line.substr(0, 3) == "vn ")
		{
			std::istringstream s(line.substr(3));
			Float3 Normal;
			s >> Normal.x >> Normal.y>>Normal.z;
			normallut.push_back(Normal);
		}
		else if (line.substr(0, 2) == "f ")
		{
			std::istringstream s(line.substr(2));
			std::string splitted;
			std::vector<UINT>Triangleindex;
			std::vector<UINT>Trianglenormalindex;
			std::vector<UINT>Trianglevtindex;
			while (std::getline(s, splitted, ' '))
			{
				UINT index;
				UINT vtindex;
				UINT normalindex;
				char separator;
				std::istringstream ss(splitted);
				if (ss >> index >> separator >> vtindex >> separator>> normalindex)
				{
						Triangleindex.push_back(index - 1);
						Trianglenormalindex.push_back(normalindex-1);
						Trianglevtindex.push_back(vtindex - 1);
				}
			}
			UINT n = (UINT)Triangleindex.size();
			std::vector<int> vertexindex;
			for (UINT i = 0; i < n; i++)
			{
				vertexindex.push_back(indexcount+i);
				Vertex vertex;
				vertex.position = positionlut[Triangleindex[i]];
				vertex.normal = normallut[Trianglenormalindex[i]];
				vertex.tangent = Float3(0.0f, 0.0f, 0.0f);
				vertex.texcoord = uvlut[Trianglevtindex[i]];
				//vertex.color = randomColor();
				vertices.push_back(vertex);
			}
			indexcount += n;
			for (UINT i = 2; i < n; i++)
			{
				indices.push_back(vertexindex[0]);
				indices.push_back(vertexindex[i-1]);
				indices.push_back(vertexindex[i]);
			}
		}
	}
	int n = indices.size();
	for (int i = 0; i < n; i += 3)
	{
		ComputeTangent(indices[i],indices[i+1],indices[i+2]);
	}
	for (auto& x : vertices)
	{
		x.tangent = ZMath::Normalize(x.tangent);
	}
	DataDescribe[0] = (UINT)vertices.size();
	DataDescribe[1] = DataDescribe[0] *sizeof(Vertex);
	DataDescribe[2] = (UINT)indices.size();
	DataDescribe[3] = DataDescribe[2]*sizeof(UINT);

	MeshAssetCreate(*this, "Asset/Mode.uasset");
	file.close();
}

FLinearColor OBJ::randomColor()
{
	FLinearColor RandColor;
	float random1 = rand() % (99 + 1) / (float)(99 + 1);
	float random2 = rand() % (99 + 1) / (float)(99 + 1);
	float random3 = rand() % (99 + 1) / (float)(99 + 1);
	RandColor = { random1 ,random2 ,random3 ,1.0f};
	return RandColor;
}

void OBJ::ComputeTangent(UINT first, UINT second, UINT third)
{
	

	FVector3 pos1 = ZMath::Float3ToFVector3(&vertices[first].position);
	FVector3 pos2 = ZMath::Float3ToFVector3(&vertices[second].position);
	FVector3 pos3 = ZMath::Float3ToFVector3(&vertices[third].position);

	
	FVector2 uv1 = ZMath::Float2ToFVector2(&vertices[first].texcoord);
	FVector2 uv2 = ZMath::Float2ToFVector2(&vertices[second].texcoord);
	FVector2 uv3 = ZMath::Float2ToFVector2(&vertices[third].texcoord);
	
	FVector3 edge1 = pos2 - pos1;
	FVector3 edge2 = pos3 - pos1;
	FVector2 deltaUV1 = uv2 - uv1;
	FVector2 deltaUV2 = uv3 - uv1;

	float f = 1.0f / (ZMath::GetFVectorX(deltaUV1)* ZMath::GetFVectorY(deltaUV2) - ZMath::GetFVectorX(deltaUV2) * ZMath::GetFVectorY(deltaUV1));
	FVector3 tangent = (edge1 * ZMath::GetFVectorY(deltaUV2) - edge2 * ZMath::GetFVectorY(deltaUV1)) * f;
	FVector3 tangent1 = ZMath::Float3ToFVector3(&vertices[first].tangent);
	FVector3 tangent2 = ZMath::Float3ToFVector3(&vertices[second].tangent);
	FVector3 tangent3 = ZMath::Float3ToFVector3(&vertices[third].tangent);

	

	vertices[first].tangent = ZMath::FVector3ToFloat3(tangent1+tangent);
	vertices[second].tangent = ZMath::FVector3ToFloat3(tangent2 + tangent);
	vertices[third].tangent = ZMath::FVector3ToFloat3(tangent3 + tangent);
}



std::array<UINT, 4> OBJ::GetDataDescribe()
{
	return DataDescribe;
}

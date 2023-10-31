#include "OBJ.h"
#include <iostream>
#include <fstream>
#include <sstream>

void OBJ::Load(std::string path)
{
	std::ifstream file(path);
	if (!file.is_open())
	{
		return;
	}
	std::string line;
	FLinearColor Color = { 0.1,0.1,0.1,1.0 };
	srand(time(NULL));
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
			uvlut.push_back(UV);
		}
		else if (line.substr(0, 2) == "f ")
		{
			std::istringstream s(line.substr(2));
			std::string splitted;
			std::vector<DWORD>Triangleindex;
			std::vector<DWORD>Trianglevtindex;
			while (std::getline(s, splitted, ' '))
			{
				UINT index;
				UINT vtindex;
				std::istringstream(splitted) >> index;
				std::istringstream(splitted) >> vtindex;
				Triangleindex.push_back(index - 1);
				Trianglevtindex.push_back(vtindex-1);
			}
			std::vector<int> vertexindex;
			for (size_t i = 0; i < Triangleindex.size(); i++)
			{
				vertexindex.push_back(indexcount+i);
				Vertex vertex;
				vertex.position = positionlut[Triangleindex[i]];
				vertex.texcoord = uvlut[Trianglevtindex[i]];
				vertex.color = randomColor();
				vertices.push_back(vertex);
			}
			indexcount += Triangleindex.size();
			for (size_t i = 2; i < Triangleindex.size(); i++)
			{
				indices.push_back(vertexindex[0]);
				indices.push_back(vertexindex[i-1]);
				indices.push_back(vertexindex[i]);
			}
		}
	}
	file.close();
	//std::cout << "Loaded " << vertices.size() << " vertices, " << faces.size() << " faces.\n";
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

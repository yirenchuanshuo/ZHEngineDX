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
	while (std::getline(file, line)) {
		if (line.substr(0, 2) == "v ")
		{
			std::istringstream s(line.substr(2));
			Vertex vertex;
			s >> vertex.position.x >> vertex.position.y >> vertex.position.z;
			vertex.color = randomColor();
			vertices.push_back(vertex);
		}
		else if (line.substr(0, 2) == "f ")
		{
			std::istringstream s(line.substr(2));
			std::string splitted;
			std::vector<DWORD>Triangleindex;
			while (std::getline(s, splitted, ' '))
			{
				UINT index;
				std::istringstream(splitted) >> index;
				Triangleindex.push_back(index - 1);
			}
			for (size_t i = 2; i < Triangleindex.size(); i++)
			{
				indices.push_back(Triangleindex[0]);
				indices.push_back(Triangleindex[i - 1]);
				indices.push_back(Triangleindex[i]);
				//Face face = { indices[0],indices[i - 1],indices[i] };
				//faces.push_back(face);
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

#pragma once
#include <vector>
#include <string>
#include "MeshBaseData.h"


class OBJ {
public:
    std::vector<Vertex> vertices;
    std::vector<DWORD>indices;
    //std::vector<Face> faces;

    void Load(std::string path);
    FLinearColor randomColor();
    //void draw_obj(bool isFlat);
};


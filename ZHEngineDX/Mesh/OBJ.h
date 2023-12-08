#pragma once
#include <vector>
#include <string>
#include <array>
#include "MeshBaseData.h"
#include "../Common/ZHEngineMath.h"

std::string GenerateUAssetPath(std::string& file);

class OBJ {
public:
    std::vector<Vertex> vertices;
    std::vector<UINT>indices;
    //std::vector<Face> faces;

    void Load(std::string path);
    FLinearColor randomColor();
    void ComputeTangent(UINT first, UINT second, UINT third);
    //void draw_obj(bool isFlat);

    std::array<UINT, 4> GetDataDescribe();
    
    
private:
    std::vector<Float3> positionlut;
    std::vector<Float2> uvlut;
    std::vector<Float3> normallut;
    std::array<UINT, 4> DataDescribe;
};


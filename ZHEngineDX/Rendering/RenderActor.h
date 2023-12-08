#pragma once
#include "../Mesh/StaticMesh.h"

class RenderActor
{
public:
    RenderActor();

    void Init();
public:
	std::unique_ptr<StaticMesh> Mesh;

    Microsoft::WRL::ComPtr<ID3D12Resource> g_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW g_vertexBufferView;

    Microsoft::WRL::ComPtr<ID3D12Resource> g_indexBuffer;
    D3D12_INDEX_BUFFER_VIEW g_indexBufferView;
};
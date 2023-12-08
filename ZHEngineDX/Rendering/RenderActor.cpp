#include "RenderActor.h"

RenderActor::RenderActor()
	:g_vertexBufferView{}, g_indexBufferView{}
{

}

void RenderActor::Init()
{
	Mesh = std::make_unique<StaticMesh>();
}

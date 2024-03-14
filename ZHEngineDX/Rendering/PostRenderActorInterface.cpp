#include "PostRenderActorInterface.h"



UPostRenderActorInterface::UPostRenderActorInterface(std::shared_ptr<PostRenderActor>& PostRenderActorBase)
	:pPostRenderActor(PostRenderActorBase)
{

}

UPostRenderActorInterface::~UPostRenderActorInterface()
{
}

void UPostRenderActorInterface::ApplyPostProcess(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* renderTarget, int blurCount)
{
	pPostRenderActor->ApplyPostProcess(cmdList,renderTarget,blurCount);
}

ID3D12Resource* UPostRenderActorInterface::PostProcessOutPut()
{
	return pPostRenderActor->g_PostMap0.Get();
}

#pragma once
#include"PostProcess.h"

class PostRenderActor;
class UPostRenderActorInterface
{
public:
	UPostRenderActorInterface(std::shared_ptr<PostRenderActor>& PostRenderActorBase);
	~UPostRenderActorInterface();
	void ApplyPostProcess(ID3D12GraphicsCommandList* cmdList, ID3D12Resource* renderTarget, int blurCount);
	ID3D12Resource* PostProcessOutPut();
private:
	std::shared_ptr<PostRenderActor> pPostRenderActor;

};
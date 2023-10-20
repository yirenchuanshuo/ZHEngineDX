#include "Common/CommonCore.h"
#include "Example/HelloGame.h"


_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	HelloGame game(1280, 768, L"ZHEngine");
	return GameWindowApplication::Run(&game, hInstance, nCmdShow);
}
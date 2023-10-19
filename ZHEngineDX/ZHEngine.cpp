#include "Common/CommonCore.h"
#include "Example/HelloGame.h"


_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	HelloGame game(800, 800, L"ZHEngine");
	return GameWindowApplication::Run(&game, hInstance, nCmdShow);
}
#include "Common/CommonCore.h"
#include "Example/HelloGame.h"
#include <iostream>


_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	HelloGame game(1280, 768, L"ZHEngine");
	return GameWindowApplication::Run(&game, hInstance, nCmdShow);
}

int main()
{
	OBJ Mode;
	Mode.Load("Asset/Cube.obj");
	std::cout << std::endl;
	/*UINT n = Mode.indices.size();
	for (int i = 0; i < n -2; i+=3)
	{
		std::cout << Mode.indices[i]<<" " << Mode.indices[i + 1]<<" " << Mode.indices[i + 2];
		std::cout << std::endl;
	}*/
	return 0;
}
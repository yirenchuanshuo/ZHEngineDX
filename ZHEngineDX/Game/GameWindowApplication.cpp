#include "GameWindowApplication.h"
#define DEBUGMESSAGE 0
HWND GameWindowApplication::g_hwnd = nullptr;

int GameWindowApplication::Run(Game* game, HINSTANCE hInstance, int nCmdShow)
{
	WNDCLASSEX windowClass = { 0 };
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowProc;
	windowClass.hInstance = hInstance;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName = L"ZHEngine";
	RegisterClassEx(&windowClass);

	g_hwnd = CreateWindow(
		windowClass.lpszClassName,
		game->GetTitle(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		game->GetWidth(),
		game->GetHeight(),
		nullptr,
		nullptr,
		windowClass.hInstance,
		game);

	game->OnInit();

	ShowWindow(g_hwnd, nCmdShow);

	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	game->OnDestroy();

	return static_cast<char>(msg.wParam);
}

LRESULT GameWindowApplication::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	Game* game = reinterpret_cast<Game*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	switch (message)
	{
	case WM_CREATE:
		{	//����ָ�뵽���ڣ������Զ����û�����
			LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
		}
		return 0;
	case WM_PAINT:
	{
		if (DEBUGMESSAGE)
		{
			hdc = BeginPaint(hWnd, &ps);
			SetTextColor(hdc, RGB(255, 0, 0)); // �����ı���ɫΪ��ɫ
			SetBkMode(hdc, TRANSPARENT); // ���ñ���ģʽΪ͸��
			TextOut(hdc, 50, 50, DebugToDisplay, wcslen(DebugToDisplay)); // ����ı�
			EndPaint(hWnd, &ps);
		}
		if (game)
		{
			game->OnRender();
		}
		return 0;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

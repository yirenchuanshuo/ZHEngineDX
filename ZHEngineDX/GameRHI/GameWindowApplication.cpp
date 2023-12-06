#include "GameWindowApplication.h"

#define DEBUGMESSAGE 0
HWND GameWindowApplication::g_hwnd = nullptr;

int GameWindowApplication::Run(GameRHI* gameRHI, HINSTANCE hInstance, int nCmdShow)
{
	int argc;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	gameRHI->ParseCommandLineArgs(argv, argc);
	LocalFree(argv);

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
		gameRHI->GetTitle(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		gameRHI->GetWidth(),
		gameRHI->GetHeight(),
		nullptr,
		nullptr,
		windowClass.hInstance,
		gameRHI);

	if (!g_hwnd)
		return 1;

	gameRHI->OnInit();

	ShowWindow(g_hwnd, nCmdShow);

	

	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			gameRHI->Tick();
		}
	}

	gameRHI->OnDestroy();

	CoUninitialize();

	return static_cast<char>(msg.wParam);
}




LRESULT GameWindowApplication::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	GameRHI* gameRHI = reinterpret_cast<GameRHI*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
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
		else
		{
			if (gameRHI)
			{
				gameRHI->Tick();
			}
		}
		return 0;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_RBUTTONDOWN:
		gameRHI->OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_RBUTTONUP:
		gameRHI->OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		gameRHI->OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

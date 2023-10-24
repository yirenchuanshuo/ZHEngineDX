#include"Game.h"


Game::Game(UINT width, UINT height, std::wstring name):
    g_width(width),
    g_height(height),
    g_title(name)
{
	WCHAR assetsPath[512];
	GetAssetsPath(assetsPath, _countof(assetsPath));
	g_assetsPath = assetsPath;


	g_aspectRatio = static_cast<float>(width) / static_cast<float>(height);
}

Game::~Game()
{
}

std::wstring Game::GetGameAssetPath()
{
	return g_assetsPath;
}

std::wstring Game::GetAssetFullPath(LPCWSTR assetName)
{
	return g_assetsPath + assetName;
}

IDXGIAdapter1* Game::GetSupportedAdapter(ComPtr<IDXGIFactory6>& dxgiFactory, const D3D_FEATURE_LEVEL featureLevel)
{
	IDXGIAdapter1* adapter = nullptr;
	for (std::uint32_t adapterIndex = 0U; ; ++adapterIndex)
	{
		IDXGIAdapter1* currentAdapter = nullptr;
		if (DXGI_ERROR_NOT_FOUND == dxgiFactory->EnumAdapters1(adapterIndex, &currentAdapter))
		{
			break;
		}

		const HRESULT hres = D3D12CreateDevice(currentAdapter, featureLevel, _uuidof(ID3D12Device), nullptr);
		if (SUCCEEDED(hres))
		{
			adapter = currentAdapter;
			break;
		}

		currentAdapter->Release();
	}

	return adapter;
}

void Game::OnMouseDown(WPARAM btnState, int x, int y)
{
	g_camera.g_LastMousePos.x = x;
	g_camera.g_LastMousePos.y = y;

	SetCapture(hwnd);
}

void Game::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void Game::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - g_camera.g_LastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - g_camera.g_LastMousePos.y));

		// Update angles based on input to orbit camera around box.
		g_camera.g_Theta += dx;
		g_camera.g_Phi += dy;

		// Restrict the angle mPhi.
		g_camera.g_Phi = ZHEngineMath::Clamp(g_camera.g_Phi, 0.1f, PI - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to 0.005 unit in the scene.
		float dx = 0.005f * static_cast<float>(x - g_camera.g_LastMousePos.x);
		float dy = 0.005f * static_cast<float>(y - g_camera.g_LastMousePos.y);

		// Update the camera radius based on input.
		g_camera.g_Radius += dx - dy;

		// Restrict the radius.
		g_camera.g_Radius = ZHEngineMath::Clamp(g_camera.g_Radius, 3.0f, 15.0f);
	}

	g_camera.g_LastMousePos.x = x;
	g_camera.g_LastMousePos.y = y;
}

float Game::AspectRatio() const
{
	
	return static_cast<float>(g_width) / g_height;
	
}

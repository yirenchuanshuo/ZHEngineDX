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

#pragma once
#include"../GameHelper/GameHelper.h"
#include"GameWindowApplication.h"

class Game
{
public:
	Game(UINT width, UINT height, std::wstring name);
	virtual ~Game();
public:
	HWND hwnd;

	virtual void OnInit() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnRender() = 0;
	virtual void OnDestroy() = 0;


	UINT GetWidth() const { return g_width; }
	UINT GetHeight() const { return g_height; }
	const WCHAR* GetTitle() const { return g_title.c_str();}
	std::wstring GetGameAssetPath();

protected:
	std::wstring GetAssetFullPath(LPCWSTR assetName);
	IDXGIAdapter1* GetSupportedAdapter(ComPtr<IDXGIFactory6>& dxgiFactory, const D3D_FEATURE_LEVEL featureLevel);


	UINT g_width;
	UINT g_height;
	float g_aspectRatio;

private:
	std::wstring g_assetsPath;
	std::wstring g_title;
};


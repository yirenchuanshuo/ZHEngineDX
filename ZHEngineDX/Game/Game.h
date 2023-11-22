#pragma once
#include "../GameHelper/GameHelper.h"
#include "Camera.h"
#include "GameWindowApplication.h"

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

	virtual void OnMouseDown(WPARAM btnState, int x, int y);
	virtual void OnMouseUp(WPARAM btnState, int x, int y);
	virtual void OnMouseMove(WPARAM btnState, int x, int y);
	

	float AspectRatio()const;
	int LoadImageDataFromFile(std::shared_ptr<BYTE>& imageData, D3D12_RESOURCE_DESC& resourceDescription, LPCWSTR filename, int& bytesPerRow);
	
	UINT GetWidth() const { return g_width; }
	UINT GetHeight() const { return g_height; }
	const WCHAR* GetTitle() const { return g_title.c_str();}
	std::wstring GetGameAssetPath();

	void ParseCommandLineArgs(_In_reads_(argc) WCHAR* argv[], int argc);

protected:
	std::wstring GetAssetFullPath(LPCWSTR assetName);
	IDXGIAdapter1* GetSupportedAdapter(ComPtr<IDXGIFactory6>& dxgiFactory, const D3D_FEATURE_LEVEL featureLevel);


	UINT g_width;
	UINT g_height;
	float g_aspectRatio;
	Camera g_camera;

	bool g_useWarpDevice;

private:
	std::wstring g_assetsPath;
	std::wstring g_title;

	DXGI_FORMAT GetDXGIFormatFromWICFormat(WICPixelFormatGUID& wicFormatGUID);
	WICPixelFormatGUID GetConvertToWICFormat(WICPixelFormatGUID& wicFormatGUID);
	int GetDXGIFormatBitsPerPixel(DXGI_FORMAT& dxgiFormat);
};


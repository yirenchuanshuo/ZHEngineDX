#pragma once
#include"../Common/CommonCore.h"
using namespace Microsoft::WRL;
class DXRHI
{
public:
	static const unsigned int g_AllowTearing = 0x1;
	static const unsigned int g_EnableHDR = 0x2;

	DXRHI();
	~DXRHI();

private:
	IDXGIAdapter1* GetSupportedAdapter(ComPtr<IDXGIFactory6>& dxgiFactory, const D3D_FEATURE_LEVEL featureLevel);
};
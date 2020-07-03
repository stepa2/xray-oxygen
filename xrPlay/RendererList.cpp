////////////////////////////////////
#include <d3d9.h>
#include <d3d11.h>
////////////////////////////////////
#pragma comment(lib, "xrCore.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3d10.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#include "../xrCore/xrCore.h"
#include <stdio.h>
#include <iostream>
///////////////////////////////////////////////
#define DLL_API __declspec(dllimport)
#define _RELEASE(x) { if(x) { (x)->Release(); (x)=NULL; } }
DLL_API xr_vector<xr_token> vid_quality_token;

constexpr const char* r4_name = "xrRender_R4";
/////////////////////////////////////////////////////
bool SupportsDX11Rendering()
{
	// Register class
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(wcex));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.lpfnWndProc = DefWindowProc;
	wcex.hInstance = GetModuleHandle(NULL);		//#VERTVER: wat?
	wcex.lpszClassName = "TestDX11WindowClass";
	if (!RegisterClassExA(&wcex))
	{
		Log("* DX11: failed to register window class");
		return false;
	}

	// Create window
	HWND hWnd = CreateWindowExA(0L, "TestDX11WindowClass", "", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, NULL, NULL);

	if (!hWnd)
	{
		Msg("* DX11: failed to create window");
		return false;
	}

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = 800;
	sd.BufferDesc.Height = 600;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	D3D_FEATURE_LEVEL pFeatureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
	D3D_FEATURE_LEVEL FeatureLevel;

	ID3D11Device* pd3dDevice = NULL;
	ID3D11DeviceContext* pContext = NULL;
	IDXGISwapChain* pSwapChain = NULL;

	HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, pFeatureLevels, 1, D3D11_SDK_VERSION,
		&sd, &pSwapChain, &pd3dDevice, &FeatureLevel, &pContext);

	if (FAILED(hr))
		Msg("* D3D11: device creation failed with hr=0x%08x", hr);

	_RELEASE(pSwapChain);
	_RELEASE(pd3dDevice);
	_RELEASE(pContext);

	DestroyWindow(hWnd);
	UnregisterClassA("TestDX11WindowClass", GetModuleHandle(NULL));

	return SUCCEEDED(hr);
}

void CreateRendererList()
{
	if (!vid_quality_token.empty())
		return;

	xr_vector<xr_token> modes;

	// try to initialize R4
	SetErrorMode(0);
	if (SupportsDX11Rendering())
	{
		modes.push_back(xr_token("renderer_r4", 0));
	}

	modes.emplace_back(xr_token(nullptr, -1));

	Msg("Available render modes[%d]:", modes.size());

	for (xr_token& mode : modes)
	{
		if (mode.name)
		{
			Log(mode.name);
		}
	}

	vid_quality_token = std::move(modes);
}
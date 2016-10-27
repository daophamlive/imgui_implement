// direct2d_example.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "direct2d_example.h"
#include <memory>
#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);



extern std::shared_ptr<ID2D1Factory>						m_sp_d2d_factory;				//Direct2D factory
extern std::shared_ptr<ID2D1HwndRenderTarget>				m_sp_render_target;				//Render target
extern std::shared_ptr<IDWriteFactory>						m_sp_dwrite_factory;			//DirectWrite factory		
extern std::shared_ptr<IWICImagingFactory>					m_sp_wic_factory;				//wic imaging factory
extern std::shared_ptr<ID2D1SolidColorBrush>				m_sp_solid_brush;

void CreateBasicResource()
{
		ID2D1Factory*		p_d2d_factory;
		IDWriteFactory*		p_dwrite_Factory;

		//create a Direct2D factory

		#ifdef _DEBUG
			D2D1_FACTORY_OPTIONS d2d1_factory_options;
			d2d1_factory_options.debugLevel = D2D1_DEBUG_LEVEL_NONE;
			//d2d1_factory_options.debugLevel = D2D1_DEBUG_LEVEL_ERROR;
			//d2d1_factory_options.debugLevel = D2D1_DEBUG_LEVEL_WARNING;
			//d2d1_factory_options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;

			D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, d2d1_factory_options, &p_d2d_factory);
		#else
			D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &p_d2d_factory);
		#endif
		
		m_sp_d2d_factory = std::make_shared<ID2D1Factory>(p_d2d_factory);
		//m_sp_d2d_factory->AddRef();

		//create a DirectWrite factory
		DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&p_dwrite_Factory));
		m_sp_dwrite_factory = std::make_shared<IDWriteFactory>(p_dwrite_Factory);
		//m_sp_dwrite_factory->AddRef();

		//create a WIC imaging factory
		// Initialize COM
		CoInitialize(NULL);
		// The factory pointer
		IWICImagingFactory *p_wic_factory = NULL;

		// Create the COM imaging factory
		GUID factory_id = CLSID_WICImagingFactory;

		/*
		Windows 8					6.2
		Windows Server 2012			6.2
		Windows 7					6.1
		Windows Server 2008 R2		6.1
		Windows Server 2008			6.0
		Windows Vista				6.0
		Windows Server 2003 R2		5.2
		Windows Server 2003			5.2
		Windows XP 64-Bit Edition	5.2
		Windows XP					5.1
		Windows 2000				5.0
		*/
		OSVERSIONINFO osvi;

		ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

		GetVersionEx(&osvi);

		//if(_WIN32_WINNT < _WIN32_WINNT_WIN8)
		if((osvi.dwMajorVersion < 6) ||
		((osvi.dwMajorVersion == 6) && (osvi.dwMinorVersion < 2)))
		{
			//In Window7, always use CLSID_WICImagingFactory1
			//In Window8, CLSID_WICImagingFactory is defined as CLSID_WICImagingFactory2
			factory_id = CLSID_WICImagingFactory1;
		}

		CoCreateInstance(factory_id, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&p_wic_factory));

		m_sp_wic_factory = std::make_shared<IWICImagingFactory>(p_wic_factory);
}

HRESULT CheckDeviceResources(HWND hWnd)
{
	HRESULT hr = S_OK;

	if (m_sp_render_target.get() == NULL)
	{
		LPRECT windows_rect;
		GetClientRect(hWnd, windows_rect);

		D2D1_SIZE_U size = D2D1::SizeU(static_cast<UINT32>((windows_rect->right - windows_rect->left)/*/m_sp_share->m_dpi_scale_x*/), static_cast<UINT32>((windows_rect->bottom - windows_rect->top)/*/m_sp_share->m_dpi_scale_y*/));

		// Create a Direct2D render target
		ID2D1HwndRenderTarget* pRenderTarget;

		m_sp_d2d_factory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(hWnd, size),
			&pRenderTarget);

		m_sp_render_target = std::make_shared<ID2D1HwndRenderTarget>(pRenderTarget);

		// Create a Direct2D solid brush
		ID2D1SolidColorBrush* pSolidBrush;
		m_sp_render_target->CreateSolidColorBrush(
			D2D1::ColorF( D2D1::ColorF::Black ),
			&pSolidBrush);

		m_sp_solid_brush =  std::make_shared<ID2D1SolidColorBrush>(pSolidBrush);

		
	}

	if (m_sp_render_target.get() == NULL)
		return E_FAIL;

	return hr;
}

void DiscardDeviceResources()
{
	//boost::mutex::scoped_lock	lock(m_mutex);

	m_sp_solid_brush.reset();

	//Bin note: always remove render target at the end, because brush and other members were created based on render target
	m_sp_render_target.reset();
}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_DIRECT2D_EXAMPLE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DIRECT2D_EXAMPLE));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DIRECT2D_EXAMPLE));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_DIRECT2D_EXAMPLE);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

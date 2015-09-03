#include "RedShell.h"

// forward declarations
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow);
ATOM MyRegisterClass(HINSTANCE hInstance);

// constants
#define MAX_LOADSTRING 100
#define	WM_ICON_NOTIFY WM_APP+10

// globals
HINSTANCE hInst;
TCHAR szTitle[MAX_LOADSTRING];
TCHAR szWindowClass[MAX_LOADSTRING];
CSystemTray TrayIcon;
CommandConfig command_config;
NetworkConfig network_config;
bool listening = true;
void* z_context;

void ZmqSocketListener()
{
	z_context = zmq_ctx_new();
	void *responder = zmq_socket(z_context, ZMQ_REP);

	// const char* addr = "tcp://*:5555";
	const char* addr = network_config.GetSocketBinding();
	int rc = zmq_bind(responder, addr);

	const int BUFF_SIZE = 1024;
	while (listening) {
		char buffer[BUFF_SIZE];
		rc = zmq_recv(responder, &buffer, BUFF_SIZE, 0);

		if (rc > 0 && rc < BUFF_SIZE) {
			try {
				std::string s(buffer, rc);

				// split the string by vertical tab
				// first entry is the command (registry key)
				// second entry is the command parameters
				auto pivot = s.find('\v');

				if (-1 != pivot && pivot < s.length()) {
					auto cmd = s.substr(0, pivot);
					auto params = s.substr(pivot + 1, s.length());
					auto exe = command_config.GetRegistryEntry(cmd);
					ShellExecute(NULL, "open", exe.c_str(), params.c_str(), NULL, SW_SHOWNORMAL);
				}
				else {
					ShellExecute(NULL, "open", s.c_str(), NULL, NULL, SW_SHOWNORMAL);
				}

				zmq_send(responder, "OK", 3, 0);
			}
			catch (std::exception& ex) {
				std::cerr << "exception caught: " << ex.what() << '\n';
			}
		}
	}

	zmq_close(responder);
}

void StopNetworkThread(std::thread& network_thread)
{
	try
	{
		listening = false;
		zmq_ctx_destroy(z_context);
		network_thread.join();
	}
	catch (const void* ex)
	{
		std::cout << ex << std::endl;
	}
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_TASKBAR, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_TASKBAR);

	// Background networking thread
	std::thread network_thread(ZmqSocketListener);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	// Clean shutdown
	StopNetworkThread(network_thread);

	return msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_APPICON);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCSTR)IDC_TASKBAR;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm        = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_APPICON);

	return RegisterClassEx(&wcex);
}

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

   // Create the tray icon
   TCHAR szHello[MAX_LOADSTRING];
   LoadString(hInst, IDS_HELLO, szHello, MAX_LOADSTRING);

	if (!TrayIcon.Create(hInstance,
						hWnd,                            // Parent window
						WM_ICON_NOTIFY,                  // Icon notify message to use
						szHello,                         // tooltip
						::LoadIcon(hInstance, (LPCTSTR)IDI_APPICON),
						IDR_POPUP_MENU))
		return FALSE;

   //ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	TCHAR szHello[MAX_LOADSTRING];
	LoadString(hInst, IDS_HELLO, szHello, MAX_LOADSTRING);

	switch (message)
	{
        case WM_ICON_NOTIFY:
            return TrayIcon.OnTrayNotification(wParam, lParam);

		case WM_COMMAND:
			wmId    = LOWORD(wParam);
			wmEvent = HIWORD(wParam);
			// Parse the menu selections:
			switch (wmId)
			{
				case IDM_ABOUT:
				   DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
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
			RECT rt;
			GetClientRect(hWnd, &rt);
			DrawText(hdc, szHello, strlen(szHello), &rt, DT_VCENTER | DT_CENTER | DT_SINGLELINE |DT_WORDBREAK);
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

LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
				return TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;
	}
    return FALSE;
}

/* ========================== */
/*     © SlonoBoyko 2018      */
/* ========================== */

#include "main.h"

CHAR		szCommand[MAX_PATH];
HMENU		hMenu, hFileMenu, hSetMenu;
HWND		hEdit, hStatic, hCombo;
HWND		hOpenBtn, hCryptBtn;
HFONT		hFont;
HBITMAP		hBitmap;
HINSTANCE	hInst;

enum t_menu_idx
{
	IDX_NONE = 0,
	IDX_BUTTON_OPEN,
	IDX_BUTTON_CRYPT,
	IDX_COMBOBOX,
	IDX_EDITBOX
};

LRESULT CALLBACK WndProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	RECT rect; HDC hdc;
	GetClientRect(hWnd, &rect);
	size_t cmd_len = strlen(szCommand);

	switch (messg)
	{
		case WM_CREATE:
		{
			DragAcceptFiles(hWnd, TRUE);

			size_t flags = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_READONLY | ES_AUTOHSCROLL;
			hEdit = CreateWindow(WC_EDIT, NULL, flags, 5, 95, 300, 22, hWnd, HMENU(IDX_EDITBOX), hInst, 0L);
			SendMessage(hEdit, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), 0L);
			if (cmd_len != NULL) SetWindowText(hEdit, szCommand);

			flags = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON;
			hOpenBtn = CreateWindow(WC_BUTTON, "Open", flags, 309, 94, 86, 24, hWnd, HMENU(IDX_BUTTON_OPEN), hInst, 0L);
			SendMessage(hOpenBtn, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), 0L);

			flags = WS_CHILD | WS_VISIBLE | WS_TABSTOP | (cmd_len ? BS_DEFPUSHBUTTON : WS_DISABLED);
			hCryptBtn = CreateWindow(WC_BUTTON, "Encrypt", flags, 5, 121, 395, 26, hWnd, HMENU(IDX_BUTTON_CRYPT), hInst, 0L);
			SendMessage(hCryptBtn, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), 0L);
			
			break;
		}
		
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDX_BUTTON_OPEN:
				{
					char buf[MAX_PATH], dir[MAX_PATH];
					memset(buf, 0, sizeof(buf));
					GetCurrentDirectory(MAX_PATH, dir);
					const char *filter = "Plugins (*.asi, *.sf)\0*.asi;*.sf\0" "Libraries (*.dll)\0*.dll\0" "All files (*.*)\0*.*\0\0";		
					OPENFILENAME ofn = { 88, hWnd, hInst, filter, 0, 0, 1, buf, MAX_PATH, 0, 0, dir, "Open file" };
					if (GetOpenFileName(&ofn)) {
						SetWindowText(hEdit, buf);
						EnableWindow(hCryptBtn, true);
					}
					break;
				}
				case IDX_BUTTON_CRYPT:
				{
					char buf[MAX_PATH];
					GetWindowText(hEdit, buf, MAX_PATH);
					std::string fname = buf;

					if (fname.size() == 0) {
						MessageBox(hWnd, "No files selected!", "Error!", MB_OK | MB_ICONERROR);
						break;
					}
					if (access(fname.data(), 0) == -1) {
						MessageBox(hWnd, "File not found!", "Error!", MB_OK | MB_ICONERROR);
						break;
					}
					
					encrypt_file(fname.data());

					MessageBox(hWnd, "File successfully encrypted.", "Cool!", MB_OK | MB_ICONINFORMATION);
					break;
				}
			}
			break;
		}
		
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORSTATIC:
		case WM_CTLCOLORLISTBOX:
		{
			SetBkColor((HDC)wParam, 0xF0F0F0);
			SetTextColor((HDC)wParam, 0x101010);
			return (LRESULT)GetSysColorBrush(COLOR_MENU);
		}

		case WM_PAINT:
		{
			PAINTSTRUCT		ps;
			hdc = BeginPaint(hWnd, &ps);
			HDC hMemDC = CreateCompatibleDC(hdc);
			SelectObject(hMemDC, hBitmap);
			BitBlt(hdc, 0, 0, 400, 105, hMemDC, 0, 0, SRCCOPY);
			DeleteObject(hBitmap);
			ReleaseDC(hWnd, hMemDC);
			EndPaint(hWnd, &ps);

			break;
		}
		
		case WM_DROPFILES:
		{
			char filename[MAX_PATH];
			HDROP hDrop = (HDROP)wParam;
			DragQueryFile(hDrop, 0, filename, MAX_PATH);
			SetWindowText(hEdit, filename);
			/*for (size_t i = 0; i < dwCount; i++) {
				DragQueryFile(hDrop, i, szFileName, MAX_PATH);
				SetWindowText(hEdit, szFileName);
			}*/
			DragFinish(hDrop);
			EnableWindow(hCryptBtn, true);
			SendMessage(hOpenBtn, BM_SETSTYLE, BS_PUSHBUTTON, 0L);
			break;
		}

		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;
		}

		default:
			return (DefWindowProc(hWnd, messg, wParam, lParam));
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	hInst		= hInstance;
	WNDCLASS wc = { CS_HREDRAW | CS_VREDRAW, WndProc, 0, 0, hInstance, LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_ICON1)),
					LoadCursor(NULL, IDC_ARROW), CreateSolidBrush(0xf0f0f0), NULL, "dll_cryptor" };
	if (!RegisterClass(&wc)) return 0;

	strcpy_s(szCommand, lpCmdLine);

	HDC hdc = GetDC(HWND_DESKTOP);
	hFont = CreateFont(-MulDiv(11, GetDeviceCaps(hdc, LOGPIXELSY), 81),
		0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, RUSSIAN_CHARSET, OUT_TT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, "Arial");
	ReleaseDC(HWND_DESKTOP, hdc);

	hBitmap = LoadBitmap(hInst, MAKEINTRESOURCEA(IDB_BITMAP1));

	uint32_t flags = WS_DLGFRAME | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZE | WS_EX_LAYERED;
	uint32_t scr_x = (GetSystemMetrics(SM_CXSCREEN) / 2 - 300);
	uint32_t scr_y = (GetSystemMetrics(SM_CYSCREEN) / 2 - 200);
	HWND hWnd = CreateWindowEx(WS_EX_LAYERED, "dll_cryptor", "Dll Cryptor by Slono Boyko", flags, 
		scr_x, scr_y, 405, 180, NULL, NULL, hInstance, NULL);
	if (hWnd == nullptr) return 0;

	SetLayeredWindowAttributes(hWnd, 0, 240, LWA_ALPHA);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	MSG lpMsg;
	while (GetMessage(&lpMsg, NULL, 0, 0))
	{
		TranslateMessage(&lpMsg);
		DispatchMessage(&lpMsg);
	}
	return (lpMsg.wParam);
}

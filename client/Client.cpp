#include <process.h>
#include "ClientGame.h"
#include "Drawing.h"
#include "DirectionEnum.h"

char *ip = "172.27.160.1";
char *port = "27015";
ClientGame *client;
Tile board[MAX_X * MAX_Y];

// ��������� �������, ������� ��������� ������ ������� � ����������� �����
void __cdecl connectionThread(void *args) {
	while (true)
		if (client != nullptr)
			client->update(board);
}

/* ������������� ����, ���������� ��� ������ ������ ���� "New Game". ��� �������������� ������� ����
 * � ������� ������ �����. ����� ��� ������� ����� ������ ������� � ����������� ��� ��������� ����������
 * ���������� client. */
void restartGame(HWND hWnd) {
	Drawing::redrawMenu(hWnd, board);
	memset(board, 0, sizeof(board[0]) * MAX_X * MAX_Y);
	client = new ClientGame(ip, port, hWnd);
}

/* ������������ ����� ������� ����. ���� ������ "New Game", ��� �������� ������� restart_game(),
 * � ���� "Exit", �� ���������� ��������� WM_QUIT � ������� ���������. */
void handleMenu(HWND hWnd, WPARAM param) {
	switch (LOWORD(param)) {
		case ID_FILE_NEWGAME:
			restartGame(hWnd);
			break;
	}
}


/* ������������ ���� � ����������. ���� ������ �� ��������� (�.�. client == nullptr), �� �������
 * �� ������ ������. � ��������� ������, ������� ���������� ����� � ��������� ������ (�������,
 * ������, �����, ���� ��� ����� � ������ ����) �� ������. */
void handleKeyboard(HWND hWnd, WPARAM input) {
	if (client == nullptr)
		return;

	switch (input) {
		case VK_RIGHT:
			client->sendAction(RIGHT);
			break;
		case VK_LEFT:
			client->sendAction(LEFT);
			break;
		case VK_UP:
			client->sendAction(UP);
			break;
		case VK_DOWN:
			client->sendAction(DOWN);
			break;
		case VK_SPACE:
			client->sendReady();
			break;
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
	}
}

// ��� ������� ���������, ������� ������������ ���������, ������������ � ������� ����. ��� �������� ��������������� ����������� ��� ������� ���� ���������.
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
		case WM_KEYDOWN:
			handleKeyboard(hWnd, wParam);
			break;
		case WM_PAINT:
			Drawing::redrawMenu(hWnd, board);
			break;
		case WM_COMMAND:
			handleMenu(hWnd, wParam);
			break;
		case WM_CREATE:
			Drawing::init(hWnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

/* ��� ����� ����� � ����������. ��� ������������ ����� ����, ������� ������� ����, ������� ����� ���
 * ���������� ������ ������� � ��������� ������� ���� ����������, ������� ������� ������� � ����������
 * �� � ������� ���������. ����� ���� ����������� (��������, ����� ������������ ��������� ����), ����������
 * �������� ����������� ������ ���� � �����������. */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nShowCmd) {
	if (__argc > 1)
		ip = __argv[1];
	if (__argc > 2)
		port = __argv[2];

	MSG msg;
	client = nullptr;

	WNDCLASSEX wc = { sizeof(WNDCLASSEX), 0, WndProc, 0, 0, hInstance, nullptr,
		nullptr, HBRUSH(COLOR_WINDOW + 1), nullptr, "wndClass", nullptr };

	if (!RegisterClassEx(&wc)) {
		MessageBox(nullptr, "Window Creation Failed", "Error", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	HWND hWnd = CreateWindowEx(WS_EX_CLIENTEDGE, "wndClass", "Snake Game", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, WINDOWSIZE_X, WINDOWSIZE_Y, nullptr, nullptr, hInstance, nullptr);
	if (hWnd == nullptr) {
		MessageBox(nullptr, "Window Creation Failed", "Error", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	ShowWindow(hWnd, nShowCmd);
	UpdateWindow(hWnd);

	auto hThread = HANDLE(_beginthread(connectionThread, 0, nullptr));
	while (GetMessage(&msg, nullptr, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UnregisterClass("wndClass", wc.hInstance);
	return 0;
}

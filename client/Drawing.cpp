#include "Drawing.h"

RECT Drawing::status_text = { 10, 5, 200, 30 }; // ���������� ���������� ������������� ��������� ��� ������ ������� ����
RECT Drawing::game = { 10, 37, 971, 726 }; // ���������� ���������� ������������� ��������� ��� ������ �������� ����

const HBRUSH Drawing::windowColor = CreateSolidBrush(RGB(0, 0, 0)); // ���� ���� ����
const HBRUSH Drawing::backgroundColor = CreateSolidBrush(RGB(83, 104, 114)); // ���� ���� �������� ����
const HBRUSH Drawing::mainPlayerColor = CreateSolidBrush(RGB(240, 234, 214)); // ���� ������
const HBRUSH Drawing::otherPlayerColor = CreateSolidBrush(RGB(235, 76, 66)); // ���� ������ �������

const HFONT Drawing::font = CreateFont(20, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET,
	OUT_TT_PRECIS, 0, CLEARTYPE_QUALITY, FF_MODERN, "Courier New"); // ���������� ����������� ������

// ������� ��� �������� ���� ����
void Drawing::init(HWND hWnd) {
	HMENU hMenu;
	hMenu = CreateMenu();
	AppendMenu(hMenu, MF_STRING, ID_FILE_NEWGAME, "&New Game");
	SetMenu(hWnd, hMenu);
}

// ������� ��� ������ ������� ����
void Drawing::drawStatus(HWND hWnd, std::string status) {
	PAINTSTRUCT PaintStruct;
	HDC hdc = BeginPaint(hWnd, &PaintStruct);
	FillRect(hdc, &status_text, backgroundColor);
	SelectObject(hdc, font);
	SetBkColor(hdc, RGB(0, 0, 0));
	SetTextColor(hdc, RGB(255, 255, 255));
	DrawText(hdc, status.c_str(), -1, &status_text, DT_LEFT);
	EndPaint(hWnd, &PaintStruct);
}

// ������� ��� ����������� �������� ���� � ����� �������
void Drawing::redrawMenu(HWND hWnd, Tile board[MAX_X * MAX_Y]) {
	PAINTSTRUCT PaintStruct;
	HDC hdc = BeginPaint(hWnd, &PaintStruct);

	auto hdcBuffer = CreateCompatibleDC(hdc);
	auto hBitmapBuffer = CreateCompatibleBitmap(hdc, WINDOWSIZE_X, WINDOWSIZE_Y);
	SelectObject(hdcBuffer, hBitmapBuffer);

	static RECT clientArea;
	GetClientRect(hWnd, &clientArea);
	FillRect(hdcBuffer, &clientArea, windowColor);
	FillRect(hdcBuffer, &game, backgroundColor);

	static RECT drawPos;
	for (auto index = 0; index < MAX_X * MAX_Y; index++) {
		if (board[index].type == EMPTY) break;

		auto x = board[index].x;
		auto y = board[index].y;

		drawPos.top = (y + 2)*TILESIZE + 22;
		drawPos.bottom = drawPos.top + TILESIZE - 1;
		drawPos.left = x*TILESIZE + 11;
		drawPos.right = drawPos.left + TILESIZE - 1;

		FillRect(hdcBuffer, &drawPos, (board[index].type == CURRENT_PLAYER) ? mainPlayerColor : otherPlayerColor);
	}

	BitBlt(hdc, 0, 0, WINDOWSIZE_X, WINDOWSIZE_Y, hdcBuffer, 0, 0, SRCCOPY);
	DeleteDC(hdcBuffer);
	DeleteObject(hBitmapBuffer);
	EndPaint(hWnd, &PaintStruct);
}

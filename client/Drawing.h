#pragma once
#include <windows.h>
#include <string>
#include "Tile.h"

#define WINDOWSIZE_X 1000
#define WINDOWSIZE_Y 800
#define MAX_X 120
#define MAX_Y 87
#define TILESIZE 8
#define ID_FILE_NEWGAME 9001
#define ID_FILE_EXIT 9002


class Drawing {
	static RECT status_text;
	static RECT game;

	static const HBRUSH windowColor;
	static const HBRUSH backgroundColor;
	static const HBRUSH mainPlayerColor;
	static const HBRUSH otherPlayerColor;

	static const HFONT font;
public:
	static void init(HWND hWnd);
	static void drawStatus(HWND hWnd, std::string status);
	static void redrawMenu(HWND hWnd, Tile board[MAX_X * MAX_Y]);
};

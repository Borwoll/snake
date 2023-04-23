#include <process.h>
#include "ServerGame.h"

ServerGame *server;

/* ��� ������� �������� �������, ������� ����������� ��� ��������� ����� ��������.
 * ������ ������������ ����� ���������� ����� getNewClients() ������� ServerGame,
 * ������� �������� �� ��������� ����� ���������� �����������. */
void __cdecl getNewClientsThread(void *args) {
	while (true)
		server->getNewClients();
}

/* ��� ������� ����� �������� �������, ������� ����������� ��� ������ ������ �� ��������.
 * ������ ������������ ����� ���������� ����� receiveFromClients() ������� ServerGame,
 * ������� �������� �� ��������� � ��������� ������ �� ��������. */
void __cdecl receiveFromClientsThread(void *args) {
	while (true)
		server->receiveFromClients();
}

/* ��� ������� ����� �������� ������� � �������� �� ������� ���� �������. �� ����������
 * ��������� server->gameTick(), ������� ������������ ������� ������� � ��������� ������ ����. */
void __cdecl gameLoop(void *args) {
	float tickLength = 80;
	while (true) {
		Sleep(tickLength);
		tickLength *= 0.999;
		server->gameTick();
	}
}

/* ��� ������� �������� ������ ����� � ���������. ��� ������� ������ ServerGame � ��������� ������
 * (���� �� ������ � ���������� ��������� ������) ��� � ������ �� ��������� ("27015"). ����� ��� �������
 * ��� ������ � ������� ������� _beginthread: getNewClientsThread � receiveFromClientsThread. ����� ���
 * ��������� ������� ���� gameLoop. */
int main(int argc, char *argv[]) {
	srand(time(nullptr));

	if (argc < 2)
		server = new ServerGame("27015");
	else
		server = new ServerGame(argv[1]);

	auto hThreadNC = HANDLE(_beginthread(getNewClientsThread, 0, nullptr));
	auto hThreadRC = HANDLE(_beginthread(receiveFromClientsThread, 0, nullptr));
	gameLoop(nullptr);
}

#include <cstdio>
#include "ClientGame.h"
#include "Networking.h"


/* ����������� ������, ������� ��������� ip-����� � ���� �������, � ����� ���������� ����.
 * ������� ������ ClientNetwork ��� ������������ ���������� � �������� � ��������� ����������
 * ���� � ���� ������ hWnd. */
ClientGame::ClientGame(char *ip, char *port, HWND hWnd) {
	this->hWnd = hWnd;
	tiles = 0;
	network = new ClientNetwork(ip, port);
}


ClientGame::~ClientGame()
{
}

/* �������, ������� ���������� ����� ���������� ������ �� ������. ������� ����� ���� READY_PACKET,
 * ����������� ��� � ���������� ����� ����� network->ConnectSocket. */
void ClientGame::sendReady() const {
	const unsigned int packet_size = sizeof(Packet);
	char packet_buffer[packet_size];

	Packet packet;
	packet.packet_type = READY_PACKET;

	packet.serialize(packet_buffer);
	NetworkServices::sendMessage(network->ConnectSocket, packet_buffer, packet_size);
}

/* �������, ������� ���������� ����� � ��������� ������ �� ������. ������� ����� ���� ACTION_EVENT,
 * ������������� ����������� �������� � ���� ������ ������ � ���������� ����� ����� network->ConnectSocket. */
void ClientGame::sendAction(DirectionEnum direction) const {
	const unsigned int packet_size = sizeof(Packet);
	char packet_buffer[packet_size];

	Packet packet;
	packet.packet_type = ACTION_EVENT;
	packet.data[0] = direction;

	packet.serialize(packet_buffer);
	NetworkServices::sendMessage(network->ConnectSocket, packet_buffer, packet_size);
}

/* �������, ������� ������������ ���������� ������ �� ������� � ��������� ������� ����. ��������� ������ board,
 * ������� ����� �������������� ��� �������� ���������� �� ������� �������. ������� �������� ��� ��������� ������
 * �� ������ �������� ���������� � �������� ������ �� ������� ������. ������������� ������ ����� � ��������
 * ��������������� ���������� � ����������� �� ���� ������. ����� ���������� �������� ���� �������� �������
 * InvalidateRect � UpdateWindow ��� ���������� ���� ����. */
void ClientGame::update(Tile board[MAX_X * MAX_Y]) {
	Packet packet;
	int data_length = network->receivePackets(network_buffer);

	if (data_length <= 0)
		return;

	int i = 0;
	while (i < data_length) {
		packet.deserialize(&network_buffer[i]);
		i += sizeof(Packet);

		switch (packet.packet_type) {
			case INIT_PACKET:
				handleInit(packet.data, board);
				break;
			case TICK_PACKET:
				handleTick(packet.data, board);
				break;
			case RESTART_PACKET:
				handleRestart(packet.data, board);
				break;
			case NEW_PLAYER_CONNECTED:
				handleNewPlayer(packet.data, board);
				break;
			default:
				break;
		}
	}

	InvalidateRect(hWnd, nullptr, FALSE);
	UpdateWindow(hWnd);
}

/* ���������� ������ �������������. ��������� ������ ������ data, ������� ��������
 * ���������� � ������������ ������� ������. ��������� ID ������ � ���������� � �������
 * � ��������� �� � ������ board. */
void ClientGame::handleInit(char data[], Tile board[MAX_X * MAX_Y]) {
	int index = 0;
	clientID = data[index++];

	while (data[index] != -1) {
		char id = data[index++];
		char x = data[index++];
		char y = data[index++];
		auto type = (id == clientID) ? CURRENT_PLAYER : ANOTHER_PLAYER;
		board[tiles++] = Tile(x, y, type);
	}
}

/* ���������� ������ ���� ����. ��������� ������ ������ data, ������� �������� ���������� � �����
 * ���������� ������� ������. ��������� ���������� � ������� � ��������� �� � ������ board. */
void ClientGame::handleTick(char data[], Tile board[MAX_X * MAX_Y]) {
	int index = 1;

	while (data[index] != -1) {
		char id = data[index++];
		char x = data[index++];
		char y = data[index++];
		auto type = (id == clientID) ? CURRENT_PLAYER : ANOTHER_PLAYER;
		board[tiles++] = Tile(x, y, type);
	}
}

// ��� ������� ���������� ��� ��������� ������ ���� RESTART_PACKET. ��� ������������ ������ ������, ����� �������� ��������� �������� ���� �� �������.
void ClientGame::handleRestart(char data[], Tile board[MAX_X * MAX_Y]) {
	int index = tiles = 0;
	memset(board, 0, sizeof(board[0]) * MAX_X * MAX_Y); 

	while (data[index] != -1) {
		char id = data[index++];
		char x = data[index++];
		char y = data[index++];
		auto type = (id == clientID) ? CURRENT_PLAYER : ANOTHER_PLAYER;

		board[tiles++] = Tile(x, y, type);
	}
}

// ��� ������� ���������� ��� ��������� ������ ���� NEW_PLAYER_CONNECTED. ��� ������������ ������ ������, ����� �������� ������ ������ �� ������� ���� �� �������.
void ClientGame::handleNewPlayer(char data[], Tile board[MAX_X * MAX_Y]) {
	char x = data[0];
	char y = data[1];
	board[tiles++] = Tile(x, y, ANOTHER_PLAYER);
}

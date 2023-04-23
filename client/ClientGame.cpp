#include <cstdio>
#include "ClientGame.h"
#include "Networking.h"


/* конструктор класса, который принимает ip-адрес и порт сервера, а также дескриптор окна.
 * Создает объект ClientNetwork для установления соединения с сервером и сохраняет дескриптор
 * окна в член класса hWnd. */
ClientGame::ClientGame(char *ip, char *port, HWND hWnd) {
	this->hWnd = hWnd;
	tiles = 0;
	network = new ClientNetwork(ip, port);
}


ClientGame::~ClientGame()
{
}

/* функция, которая отправляет пакет готовности игрока на сервер. Создает пакет типа READY_PACKET,
 * сериализует его и отправляет через сокет network->ConnectSocket. */
void ClientGame::sendReady() const {
	const unsigned int packet_size = sizeof(Packet);
	char packet_buffer[packet_size];

	Packet packet;
	packet.packet_type = READY_PACKET;

	packet.serialize(packet_buffer);
	NetworkServices::sendMessage(network->ConnectSocket, packet_buffer, packet_size);
}

/* функция, которая отправляет пакет с действием игрока на сервер. Создает пакет типа ACTION_EVENT,
 * устанавливает направление движения в поле данных пакета и отправляет через сокет network->ConnectSocket. */
void ClientGame::sendAction(DirectionEnum direction) const {
	const unsigned int packet_size = sizeof(Packet);
	char packet_buffer[packet_size];

	Packet packet;
	packet.packet_type = ACTION_EVENT;
	packet.data[0] = direction;

	packet.serialize(packet_buffer);
	NetworkServices::sendMessage(network->ConnectSocket, packet_buffer, packet_size);
}

/* функция, которая обрабатывает полученные пакеты от сервера и обновляет игровое поле. Принимает массив board,
 * который будет использоваться для хранения информации об игровых плитках. Сначала получает все доступные пакеты
 * из буфера сетевого соединения и проходит циклом по каждому пакету. Десериализует каждый пакет и вызывает
 * соответствующий обработчик в зависимости от типа пакета. После обновления игрового поля вызывает функции
 * InvalidateRect и UpdateWindow для обновления окна игры. */
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

/* обработчик пакета инициализации. Принимает массив байтов data, который содержит
 * информацию о расположении игровых плиток. Извлекает ID игрока и информацию о плитках
 * и добавляет их в массив board. */
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

/* обработчик пакета тика игры. Принимает массив байтов data, который содержит информацию о новых
 * положениях игровых плиток. Извлекает информацию о плитках и добавляет их в массив board. */
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

// Эта функция вызывается при получении пакета типа RESTART_PACKET. Она обрабатывает данные пакета, чтобы сбросить состояние игрового поля на клиенте.
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

// Эта функция вызывается при получении пакета типа NEW_PLAYER_CONNECTED. Она обрабатывает данные пакета, чтобы добавить нового игрока на игровое поле на клиенте.
void ClientGame::handleNewPlayer(char data[], Tile board[MAX_X * MAX_Y]) {
	char x = data[0];
	char y = data[1];
	board[tiles++] = Tile(x, y, ANOTHER_PLAYER);
}

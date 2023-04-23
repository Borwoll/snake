#include "ServerGame.h"


ServerGame::ServerGame(char *port) {
	gameStarted = false;
	playerCount = 0;
	readyPlayersCount = 0;
	alivePlayersCount = 0;
	network = new ServerNetwork(port);
}


ServerGame::~ServerGame()
{
}

/* ���������, �������� �� ���� � ������� �������� ����� �������. ���� �������� ����� ���� �������,
 * ���� ���������������. ���� ���� �� ��������, �� ������� ������ �� ������. ���� �� ���� ��������,
 * �� ������� �������� ������� movePlayers() � sendTick(). */
void ServerGame::gameTick() {
	if (gameStarted && alivePlayersCount < 2)
		restartGame();

	if (!gameStarted)
		return;

	movePlayers();
	sendTick();
}

/* ���������, �� �������� �� ��� ����, � ���� �� ��������, �� �������� ������� ������ �������.
 * ���� ���������� �������� ������ �������������, � ������ ������� ������, �� ������� ��������
 * ������� initialize(), sendInitial() � sendNewPlayer(). */
void ServerGame::getNewClients() {
	if (gameStarted)
		return;

	if (playerCount < MAX_PLAYERS && network->acceptNewClient(playerCount)) {
		initialize(playerCount++);
		sendInitial(playerCount);
		sendNewPlayer(playerCount);
		Log("player %d has connected to the server\n", playerCount);
	}
}

/* �������� ������ �� �������� � ������������� �� � ������� ���� Packet. ����� ������� ������������
 * ������ � ������������ � �� �����, ��������� ������� handleAction() � handleReady(). ���� ��� ������
 * ����������, ������� ������� ��������� �� ������. */
void ServerGame::receiveFromClients() {
	Packet packet;
	std::map<unsigned int, SOCKET>::iterator iter;
	auto localSessionsCopy = network->sessions;

	for (iter = localSessionsCopy.begin(); iter != localSessionsCopy.end(); ++iter) {
		int data_length = network->receiveData(iter->first, network_buffer);
		if (data_length <= 0)
			continue;

		int i = 0;
		while (i < data_length) {
			packet.deserialize(&network_buffer[i]);
			i += sizeof(Packet);
			switch (packet.packet_type) {
				case ACTION_EVENT:
					handleAction(iter->first - 1, network_buffer[4]);
					break;
				case READY_PACKET:
					handleReady(iter->first - 1);
					break;
				default:
					Log("error in packet types\n");
					LogInDebugOnly("%s\n", packet.data);
					break;
			}
		}
	}
}

/* ������������� ����, �������� �������� ����� � ������� �������, ������� �������
 * ���� � �������� ������� initialize() � sendRestart(). */
void ServerGame::restartGame() {
	gameStarted = false;
	readyPlayersCount = alivePlayersCount = 0;
	memset(board, 0, sizeof(board[0][0]) * MAX_X * MAX_Y); // clear the board

	for (int id = 0; id < playerCount; id++)
		initialize(id);

	sendRestart();
	Log("game has been restarted\n");
}

/* ���������� ����� ������� �� ������� ����. ���� ����� �������, ������� ��������� �������
 * ����� �������, ������������� ���� alive ������ � false � ������� ��������� �� ����. ���� ��
 * ����� �������� ���, ������� �������� ��� ����� ��������� �� ������� ����. */
void ServerGame::movePlayers() {
	for (int id = 0; id < playerCount; id++) {
		if (!players[id].alive)
			continue;

		auto pos = players[id].position;
		switch (players[id].direction) {
			case UP:
				if (pos.y != 0 && !board[pos.x][pos.y - 1]) {
					players[id].move(0, -1);
					board[pos.x][pos.y - 1] = true;
					continue;
				}
				break;
			case RIGHT:
				if (pos.x != MAX_X && !board[pos.x + 1][pos.y]) {
					players[id].move(1, 0);
					board[pos.x + 1][pos.y] = true;
					continue;
				}
				break;
			case DOWN:
				if (pos.y != MAX_Y - 1 && !board[pos.x][pos.y + 1]) {
					players[id].move(0, 1);
					board[pos.x][pos.y + 1] = true;
					continue;
				}
				break;
			case LEFT:
				if (pos.x != 0 && !board[pos.x - 1][pos.y]) {
					players[id].move(-1, 0);
					board[pos.x - 1][pos.y] = true;
					continue;
				}
				break;
			default:
				continue;
		}

		alivePlayersCount--;
		players[id].alive = false;
		Log("player %d was killed at %d, %d\n", id + 1, players[id].position.x, players[id].position.y);
	}
}

/* ������� ��������� ���������� ������ � ����. ���� ���� ��� ��������, ������� ������ ���������� ����������.
 * �����, ������� ����������� ���� ���������� ������. ���� ����� ���� �������, ������������� ������� �������
 * �������. ���� ��� ������ ������, ���� ����������. */
void ServerGame::handleReady(unsigned char id) {
	if (gameStarted)
		return;

	// toggle the flag
	if (players[id].ready) {
		readyPlayersCount--;
		players[id].ready = false;
	} else {
		readyPlayersCount++;
		players[id].ready = true;
	}

	Log("player %d changed state to %s\n", id + 1, players[id].ready ? "ready" : "not ready");

	if (playerCount > 1 && readyPlayersCount == playerCount) {
		gameStarted = true;
		Log("All clients are ready, starting the game\n");
	}
}

/* ������� ��������� �������� ������. ������� �������� ������������� ������ � ����������� ��� ��������.
 * ������� ���������, ���������� �� ����������� �������� ������, � ���� ���, �� ������ ���������� ����������.
 * �����, ������� ���������, ��� ����� ����������� �������� �� �������������� �����������. ���� �����������
 * �� ��������������, �� ������� ��������� ����� ����������� ��������. */
void ServerGame::handleAction(unsigned char id, int direction) {
	auto prevDirection = players[id].direction;
	auto curDirection = DirectionEnum(direction);

	if (prevDirection == curDirection)
		return;

	switch (curDirection) {
		case UP:
			if (prevDirection == DOWN)
				return;
			break;
		case RIGHT:
			if (prevDirection == LEFT)
				return;
			break;
		case DOWN:
			if (prevDirection == UP)
				return;
			break;
		case LEFT:
			if (prevDirection == RIGHT)
				return;
			break;
		default:
			break;
	}

	players[id].direction = curDirection;
	LogInDebugOnly("player %d changed direction to %d\n", id + 1, curDirection);
}

/* ������� ������������� ����. ������� �������� ������������� ������ � �������������� ��� �� �����.
 * ������� �������� ��������� ����������� �������� � ��������� ������� ������ �� �����. */
void ServerGame::initialize(unsigned char id) {
	auto dir = DirectionEnum(rand() % LEFT);
	unsigned char x = MAX_X / 5 + rand() % MAX_X / 5 + 2 * (id % 2) * MAX_X / 5;
	unsigned char y = MAX_Y / 5 + rand() % MAX_Y / 5;
	if (id >= 2) y += 2 * MAX_Y / 5;

	Player p = Player(id + 1, dir, Position(x, y));

	LogInDebugOnly("Created player %d with direction %d and position (%d, %d)\n", p.id, p.direction, p.position.x, p.position.y);

	players[id] = p;
	board[x][y] = true;
	alivePlayersCount++;
}

// ������� �������� ��������� ������� ��������, ����������� ���������� � ������� ��������� ����.
void ServerGame::sendTick() const {
	const unsigned int packet_size = sizeof(Packet);
	char packet_data[packet_size];

	Packet packet;
	packet.packet_type = TICK_PACKET;
	createPositions(0, packet.data);

	packet.serialize(packet_data);
	network->sendToAll(packet_data, packet_size);
}

// ������� �������� ��������� ������� ��������, ����������� ���������� � ����������� ����.
void ServerGame::sendRestart() const {
	const unsigned int packet_size = sizeof(Packet);
	char packet_data[packet_size];

	Packet packet;
	packet.packet_type = RESTART_PACKET;
	createRestart(packet.data);

	packet.serialize(packet_data);
	network->sendToAll(packet_data, packet_size);
}

// ������� �������� ��������� ������� �������, ����������� ���������� � ��������� ��������� ����.
void ServerGame::sendInitial(unsigned char id) const {
	const unsigned int packet_size = sizeof(Packet);
	char packet_data[packet_size];

	Packet packet;
	packet.packet_type = INIT_PACKET;
	createPositions(id, packet.data);

	packet.serialize(packet_data);
	network->sendToOne(id, packet_data, packet_size);
}

// ������� �������� ��������� ������� ��������, ����������� ���������� � ����� ������������ ������.
void ServerGame::sendNewPlayer(unsigned char id) const {
	const unsigned int packet_size = sizeof(Packet);
	char packet_data[packet_size];

	Packet packet;
	packet.packet_type = NEW_PLAYER_CONNECTED;
	createNewPlayer(id, packet.data);

	packet.serialize(packet_data);
	network->sendToAllButOne(id, packet_data, packet_size);
}

// ������� �������� ������ ������, ����������� ���������� � ����������� ����.
void ServerGame::createRestart(char packet_data[]) const {
	int index = 0;

	for (unsigned int i = 0; i < playerCount; i++) {
		packet_data[index++] = players[i].id;
		packet_data[index++] = players[i].position.x;
		packet_data[index++] = players[i].position.y;
	}

	packet_data[index + 1] = packet_data[index] = -1;
	LogInDebugOnly("Created restart packet: %s\n", packet_data);
}

// ������� �������� ������ ������, ����������� ���������� � ������� �������� �������.
void ServerGame::createPositions(unsigned char id, char packet_data[]) const {
	int index = 0;
	packet_data[index++] = id;
	for (unsigned int i = 0; i < playerCount; i++) {
		packet_data[index++] = players[i].id;
		packet_data[index++] = players[i].position.x;
		packet_data[index++] = players[i].position.y;
	}

	packet_data[index + 1] = packet_data[index] = -1;
	LogInDebugOnly("Created packet with positions: %s\n", packet_data);
}

// ������� �������� ������ ������, ����������� ���������� � ����� ������������ ������.
void ServerGame::createNewPlayer(unsigned char id, char packet_data[]) const {
	packet_data[0] = players[id - 1].position.x;
	packet_data[1] = players[id - 1].position.y;
	LogInDebugOnly("Created new player packet: %d, %d\n", packet_data[0], packet_data[1]);
}

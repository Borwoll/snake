#pragma once
#include <ctime>
#include "ServerNetwork.h"
#include "Networking.h"
#include "Player.h"


#define MAX_PLAYERS 4
#define MAX_X 119
#define MAX_Y 86


class ServerGame {
	ServerNetwork* network;
	bool gameStarted;
	unsigned int playerCount;
	unsigned int readyPlayersCount;
	unsigned int alivePlayersCount;
	char network_buffer[MAX_PACKET_SIZE];

	Player players[MAX_PLAYERS];
	bool board[MAX_X][MAX_Y];

public:
	explicit ServerGame(char *port);
	~ServerGame();
	void gameTick();
	void getNewClients();
	void receiveFromClients();
private:
	void restartGame();
	void movePlayers();
	void handleReady(unsigned char id);
	void handleAction(unsigned char id, int direction);
	void initialize(unsigned char id);
	void sendTick() const;
	void sendRestart() const;
	void sendInitial(unsigned char id) const;
	void sendNewPlayer(unsigned char id) const;
	void createRestart(char packet_data[]) const;
	void createPositions(unsigned char id, char packet_data[]) const;
	void createNewPlayer(unsigned char id, char packet_data[]) const;
};

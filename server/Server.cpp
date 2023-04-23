#include <process.h>
#include "ServerGame.h"

ServerGame *server;

/* Эта функция является потоком, который запускается для получения новых клиентов.
 * Внутри бесконечного цикла вызывается метод getNewClients() объекта ServerGame,
 * который отвечает за обработку новых клиентских подключений. */
void __cdecl getNewClientsThread(void *args) {
	while (true)
		server->getNewClients();
}

/* Эта функция также является потоком, который запускается для приема данных от клиентов.
 * Внутри бесконечного цикла вызывается метод receiveFromClients() объекта ServerGame,
 * который отвечает за получение и обработку данных от клиентов. */
void __cdecl receiveFromClientsThread(void *args) {
	while (true)
		server->receiveFromClients();
}

/* Эта функция также является потоком и отвечает за игровой цикл сервера. Он бесконечно
 * запускает server->gameTick(), который обрабатывает игровые события и выполняет логику игры. */
void __cdecl gameLoop(void *args) {
	float tickLength = 80;
	while (true) {
		Sleep(tickLength);
		tickLength *= 0.999;
		server->gameTick();
	}
}

/* Эта функция является точкой входа в программу. Она создает объект ServerGame с указанным портом
 * (если он указан в аргументах командной строки) или с портом по умолчанию ("27015"). Затем она создает
 * два потока с помощью функций _beginthread: getNewClientsThread и receiveFromClientsThread. Затем она
 * запускает игровой цикл gameLoop. */
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

#include "Networking.h"

/* Принимает сокет curSocket, указатель на массив символов message и размер сообщения messageSize в байтах.
 * Она использует функцию send() для отправки сообщения через сокет curSocket. Функция send() передает массив
 * символов message размером messageSize в байтах через сокет curSocket и возвращает количество байтов,
 * которые были успешно отправлены. */
int NetworkServices::sendMessage(SOCKET curSocket, char * message, int messageSize) {
	return send(curSocket, message, messageSize, 0);
}

/* Принимает сокет curSocket, указатель на буфер buffer и его размер bufSize в байтах. Она использует функцию
 * recv() для приема сообщения через сокет curSocket. Функция recv() получает данные из сокета curSocket и
 * сохраняет их в буфере buffer размером bufSize в байтах. Функция также возвращает количество байтов, которые
 * были успешно получены. */
int NetworkServices::receiveMessage(SOCKET curSocket, char * buffer, int bufSize) {
	return recv(curSocket, buffer, bufSize, 0);
}

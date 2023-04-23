#include "Networking.h"

/* ��������� ����� curSocket, ��������� �� ������ �������� message � ������ ��������� messageSize � ������.
 * ��� ���������� ������� send() ��� �������� ��������� ����� ����� curSocket. ������� send() �������� ������
 * �������� message �������� messageSize � ������ ����� ����� curSocket � ���������� ���������� ������,
 * ������� ���� ������� ����������. */
int NetworkServices::sendMessage(SOCKET curSocket, char * message, int messageSize) {
	return send(curSocket, message, messageSize, 0);
}

/* ��������� ����� curSocket, ��������� �� ����� buffer � ��� ������ bufSize � ������. ��� ���������� �������
 * recv() ��� ������ ��������� ����� ����� curSocket. ������� recv() �������� ������ �� ������ curSocket �
 * ��������� �� � ������ buffer �������� bufSize � ������. ������� ����� ���������� ���������� ������, �������
 * ���� ������� ��������. */
int NetworkServices::receiveMessage(SOCKET curSocket, char * buffer, int bufSize) {
	return recv(curSocket, buffer, bufSize, 0);
}

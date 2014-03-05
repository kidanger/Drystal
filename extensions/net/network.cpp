#include <cassert>
#include <cstdio>
#include <cstring>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifdef EMSCRIPTEN
#include "emscripten.h"
#endif

#include "network.hpp"

const int UNDEFINED_SOCKET = -1;

ErrorCode Network::connect(const char* hostname, int port)
{
	struct sockaddr_in serv_addr;
	struct hostent *server;

	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockfd == -1) {
		sockfd = UNDEFINED_SOCKET;
		return UNABLE_TO_OPEN_SOCKET;
	}

	server = gethostbyname(hostname);
	if (server == NULL) {
		sockfd = UNDEFINED_SOCKET;
		return UNABLE_TO_GET_HOST;
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	memcpy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
#ifdef EMSCRIPTEN
	port += 1;
#endif
	serv_addr.sin_port = htons(port);

	//if (_connect(sockfd, (const struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
	//{
		//sockfd = UNDEFINED_SOCKET;
		//return UNABLE_TO_CONNECT;
	//}
	return NO_ERROR;
}

Network::Network() : sockfd(UNDEFINED_SOCKET)
{
}

Network::~Network()
{
	disconnect();
}

long Network::send(const char* data, unsigned long len)
{
	if (sockfd == UNDEFINED_SOCKET) {
		return NOT_CONNECTED;
	}

	(void) data;
	(void) len;
	int n = 0;
	//int n = _send(sockfd, data, len, 0);
	//if (n < 0) {
		//sockfd = UNDEFINED_SOCKET;
		//return CONNECTION_LOST;
	//}

#ifndef EMSCRIPTEN
	// XXX: emscripten add a buffer, so it will never blocks
	// that not true in native. Add something to handle this case
#endif

	return n;
}

long Network::receive(char* buffer, unsigned long len)
{
	if (sockfd == UNDEFINED_SOCKET) {
		return NOT_CONNECTED;
	}
	fd_set sett;
	FD_ZERO(&sett);
	FD_SET(sockfd, &sett);

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	int ret = select(64, &sett, NULL, NULL, &tv);
	if (ret < 0) {
		sockfd = UNDEFINED_SOCKET;
		return CONNECTION_LOST;
	} else if (ret == 0) {
		buffer[0] = 0;
		return 0;
	}

	int n = recv(sockfd, buffer, len - 1, 0);
	if (n == 0) {
		sockfd = UNDEFINED_SOCKET;
		return CONNECTION_LOST;
	}
	assert(n > 0);
	buffer[n] = 0;
	return n;
}

ErrorCode Network::disconnect()
{
	if (sockfd == UNDEFINED_SOCKET) {
		return ALREADY_DISCONNECTED;
	}

	ErrorCode error = NO_ERROR;
	if (close(sockfd) < 0) {
		error = CANNOT_CLOSE_SOCKET;
	}
	sockfd = UNDEFINED_SOCKET;
	return error;
}


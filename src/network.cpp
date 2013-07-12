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

int (*_connect)(int, const struct sockaddr*, socklen_t) = connect;
ssize_t (*_send)(int, const void*, size_t, int) = send;

#ifdef EMSCRIPTEN
#include "emscripten.h"
#endif

#include "engine.hpp"
#include "network.hpp"

static char buff[1024*11];

void Network::poll()
{
	if (sockfd < 0)
		return;
	fd_set sett;
	FD_ZERO(&sett);
	FD_SET(sockfd, &sett);

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	int ret = select(64, &sett, NULL, NULL, &tv);
	if (ret < 0)
	{
		disconnect();
		engine.net_disconnected();
		perror("Error message");
		return;
	}
	else if (ret == 0)
		return;

	int n = recv(sockfd, buff, sizeof(buff)-1, 0);
	if (n == 0)
	{
		perror("No message to read");
		disconnect();
		engine.net_disconnected();
		return;
	}
	buff[n] = 0;
	//printf("received n=%d %s\n", n, buff);
	const char *last = buff;
	char *str = NULL;
	do
	{
		str = strstr((char*) last, "\n");
		if(str)
		{
			*str = 0;
			engine.net_recv(last);
			last = str+1;
		}
	} while(str);
	poll();
}

bool Network::connect(const char* hostname, int port)
{
	struct sockaddr_in serv_addr;
	struct hostent *server;

	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockfd < 0)
	{
		perror("ERROR opening socket");
		disconnect();
		return false;
	}
	printf("connecting to %s\n", hostname);

	server = gethostbyname(hostname);
	if (server == NULL)
	{
		fprintf(stderr,"ERROR, no such host\n");
		disconnect();
		return false;
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	memcpy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
#ifdef EMSCRIPTEN
	port += 1;
#endif
	serv_addr.sin_port = htons(port);

	if (_connect(sockfd, (const struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("ERROR connecting");
		disconnect();
		return false;
	}
	engine.net_connected();
	return true;
}

Network::Network(Engine& e)
	: engine(e),
	  sockfd(-1)
{
}

Network::~Network()
{
	disconnect();
}

void Network::disconnect()
{
	if (sockfd < 0)
		return;

	int ret = close(sockfd);
	if (ret < 0)
	{
		perror("cannot close connection");
	}
	sockfd = -1;
	printf("connection closed");
}

void Network::send(const void* data, size_t len)
{
	char *buff = new char[len+1];
	memcpy(buff, data, len);
	buff[len] = '\n';
	ssize_t n = _send(sockfd, buff, len+1, 0);
	delete buff;
	if (n < 0)
	{
		printf("unable to send: %s\n", (char*)data);
		disconnect();
		engine.net_disconnected();
	}
}

void Network::flush()
{
	//TODO implement
}


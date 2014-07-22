/**
 * This file is part of Drystal.
 *
 * Drystal is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Drystal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Drystal.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <cassert>
#include <sys/time.h>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netdb.h>

#include "server.hpp"
#include "socket.hpp"
#include "macro.hpp"

void Server::listen(int port) {
	assert(port >= 0 && port < 65536);

	fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	int yes = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) != 0) {
		// ignore
	}

	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);
	if (bind(fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
		perror("bind");
	}

	::listen(fd, 1024);
}

Socket* Server::accept(float timeout, float* time) {
	assert(time);

	int sockfd;
	struct sockaddr_in cliaddr;
	socklen_t clilen;

	if (!readyToRead(timeout, time)) {
		return NULL;
	}

	clilen = sizeof(cliaddr);
	sockfd = ::accept(fd, (struct sockaddr *)&cliaddr, &clilen);

	if (sockfd) {
		char address[512];
		getnameinfo((struct sockaddr*)&cliaddr, clilen, address, sizeof(address),
				NULL, 0, NI_NUMERICHOST);
		ws_ctx_t* wsctx = do_handshake(sockfd);
		return new Socket(sockfd, address, wsctx);
	}
	return NULL;
}

bool Server::readyToRead(float timeout, float* time) {
	assert(time);

	fd_set sett;
	FD_ZERO(&sett);
	FD_SET(fd, &sett);

	struct timeval tv;
	tv.tv_sec = (int) timeout;
	tv.tv_usec = (timeout - tv.tv_sec) * USEC_PER_SEC;

	int ret = select(64, &sett, NULL, NULL, &tv);

	*time = tv.tv_sec;
	*time += tv.tv_usec / (float) USEC_PER_SEC;
	return ret > 0;
}


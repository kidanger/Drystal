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
#include <cstdlib>
#include <unistd.h>
#include <cerrno>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netdb.h>
#include <lua.hpp> // LUA_REFNIL

#include "macro.hpp"
#include "socket.hpp"
#include "log.hpp"
#include "util.h"

log_category("net");

Socket::Socket(int fd, const char* address, bool is_websocket):
	fd(fd),
	tableref(LUA_REFNIL),
#ifndef EMSCRIPTEN
	wsctx(NULL),
#endif
	address(address),
	output(NULL),
	ref(0)
{
	if (is_websocket) {
#ifndef EMSCRIPTEN
		wsctx = do_handshake(fd);
#endif
	}
}

Socket* Socket::connect(const char* hostname, int port)
{
	struct sockaddr_in serv_addr;
	struct hostent *server;

	assert(hostname);
	assert(port >= 0 && port < 65536);

	int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (fd == -1) {
		return NULL;
	}
	//fcntl(fd, F_SETFL, O_NONBLOCK);

	server = gethostbyname(hostname);
	if (server == NULL) {
		close(fd);
		return NULL;
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	memcpy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(port);
	if (::connect(fd, (const struct sockaddr*) &serv_addr, sizeof(serv_addr)) != -1
	        && errno != 0
	        && errno != EINPROGRESS) {
		log_error("connect: %s", strerror(errno));
		close(fd);
		return NULL;
	}

	return new Socket(fd, hostname, false);
}

void Socket::send(const char* msg, _unused_ int len, bool* error)
{
	char *t;

	assert(error);
	assert(msg);

	t = strnappend(output, msg, strlen(msg));
	if (!t) {
		return;
	}

	free(output);
	output = t;

	flush(error);
}

void Socket::flush(bool* error)
{
	assert(error);

	int totallen = strlen(output);
	if (totallen == 0 || !readyToSend()) {
		return;
	}

	int n;
#ifndef EMSCRIPTEN
	if (wsctx) {
		n = ws_send(wsctx, output, totallen);
	} else {
		n = ::send(fd, output, totallen, 0);
	}
#else
	n = ::send(fd, output, totallen, 0);
#endif

	if (n < 0) {
#ifdef EMSCRIPTEN
		if (errno == EAGAIN) {
			// connecting, try again later
#else
		if (0) {
#endif
		} else {
			*error = true;
			fd = -1;
		}
	} else {
		memmove(output, &output[n], strlen(output) - n);
	}
}

int Socket::receive(char* buffer, int capacity, bool* error)
{
	assert(buffer);
	assert(error);

	if (!readyToRead()) {
		return 0;
	}

	int n;
#ifndef EMSCRIPTEN
	if (wsctx) {
		n = ws_recv(wsctx, buffer, capacity);
	} else {
		n = recv(fd, buffer, capacity, 0);
	}
#else
	n = recv(fd, buffer, capacity, 0);
#endif
	if (n) {
		log_debug("received %s %d", buffer, n);
	}

	if (n == -1) {
#ifdef EMSCRIPTEN
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
#else
		if (0) {
#endif
		} else {
			fd = -1;
			*error = true;
		}
		return 0;
	}
	return n;
}

void Socket::disconnect()
{
	shutdown(fd, SHUT_RDWR);
	close(fd);
}

bool Socket::readyToRead()
{
	fd_set sett;
	FD_ZERO(&sett);
	FD_SET(fd, &sett);

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	int ret = select(64, &sett, NULL, NULL, &tv);
	return ret > 0;
}

bool Socket::readyToSend()
{
	fd_set sett;
	FD_ZERO(&sett);
	FD_SET(fd, &sett);

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	int ret = select(64, NULL, &sett, NULL, &tv);
	return ret > 0;
}


const char* Socket::getAddress() const
{
	return address;
}

void Socket::setTable(int ref)
{
	this->tableref = ref;
}

int Socket::getTable() const
{
	return tableref;
}


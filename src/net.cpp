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
#include <cstring>
#include <sstream>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <netdb.h>
#include <lua.hpp>

extern "C" {
#include "websocket.h"
}
#include "engine.hpp"

int (*_socket)(int, int, int) = socket;
int (*_connect)(int, const struct sockaddr*, socklen_t) = connect;
ssize_t (*_send)(int, const void*, size_t, int) = send;
int (*_listen)(int, int) = listen;
int (*_accept)(int, struct sockaddr*, socklen_t*) = accept;

class Socket {
private:
	Socket(const Socket&);
	Socket& operator=(const Socket&);

	int fd;
	int tableref;
	ws_ctx_t* wsctx;
	std::string address;
	std::string output;
public:
	Socket(int fd, const char* address, ws_ctx_t* ctx):
		fd(fd),
		tableref(LUA_REFNIL),
		wsctx(ctx),
		address(address),
		output()
	{
	}

	static Socket* connect(const char* hostname, int port) {
		struct sockaddr_in serv_addr;
		struct hostent *server;

		int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (fd == -1) {
			return NULL;
		}
		//fcntl(fd, F_SETFL, O_NONBLOCK);

		server = gethostbyname(hostname);
		if (server == NULL) {
			return NULL;
		}

		memset(&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		memcpy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
		serv_addr.sin_port = htons(port);
		if (_connect(fd, (const struct sockaddr*) &serv_addr, sizeof(serv_addr)) != -1
				&& errno != EINPROGRESS) {
			perror("connect");
			return NULL;
		}

		return new Socket(fd, hostname, NULL);
	}

	void send(const char* msg, int len, bool* error) {
		(void) len;
		output += msg;

		flush(error);
	}

	void flush(bool* error) {
		int totallen = output.length();
		if (totallen == 0 || !readyToSend()) {
			return;
		}

		int n;
		if (wsctx) {
			n = ws_send(wsctx, output.c_str(), totallen);
		} else {
			n = _send(fd, output.c_str(), totallen, 0);
		}

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
			output.erase(0, n);
		}
	}

	int receive(char* buffer, int capacity, bool* error) {
		if (!readyToRead()) {
			return 0;
		}

		int n;
		if (wsctx) {
			n = ws_recv(wsctx, buffer, capacity);
		} else {
			n = recv(fd, buffer, capacity, 0);
		}
		if (n)
			printf("received %s %d\n\n", buffer, n);

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

	void disconnect() {
		shutdown(fd, SHUT_RDWR);
		close(fd);
	}

	bool readyToRead() {
		fd_set sett;
		FD_ZERO(&sett);
		FD_SET(fd, &sett);

		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 0;

		int ret = select(64, &sett, NULL, NULL, &tv);
		return ret > 0;
	}
	bool readyToSend() {
		fd_set sett;
		FD_ZERO(&sett);
		FD_SET(fd, &sett);

		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 0;

		int ret = select(64, NULL, &sett, NULL, &tv);
		return ret > 0;
	}


	const char* getAddress() const {
		return address.c_str();
	}

	void setTable(int ref) {
		this->tableref = ref;
	}
	int getTable() const {
		return tableref;
	}
};

DECLARE_PUSHPOP(Socket, socket)


class Server {
private:
	int fd;

public:

	void listen(int port) {
		fd = _socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		int yes = 1;
		if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) != 0) {
			// ignore
		}

		struct sockaddr_in servaddr;
		bzero(&servaddr, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		servaddr.sin_port = htons(port);
		if (bind(fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
			perror("bind");
		}

		_listen(fd, 1024);
	}

	Socket* accept(float timeout, float* time) {
		int sockfd;
		struct sockaddr_in cliaddr;
		socklen_t clilen;

		if (!readyToRead(timeout, time)) {
			return NULL;
		}

		clilen = sizeof(cliaddr);
		sockfd = _accept(fd, (struct sockaddr *)&cliaddr, &clilen);

		if (sockfd) {
			char address[512];
			getnameinfo((struct sockaddr*)&cliaddr, clilen, address, sizeof(address),
					NULL, 0, NI_NUMERICHOST);
			ws_ctx_t* wsctx = do_handshake(sockfd);
			return new Socket(sockfd, address, wsctx);
		}
		return NULL;
	}

	bool readyToRead(float timeout, float* time) {
		fd_set sett;
		FD_ZERO(&sett);
		FD_SET(fd, &sett);

		struct timeval tv;
		tv.tv_sec = (int) timeout;
		tv.tv_usec = (timeout - tv.tv_sec) * 1000000;

		int ret = select(64, &sett, NULL, NULL, &tv);

		*time = tv.tv_sec;
		*time += tv.tv_usec / 1000000.;
		return ret > 0;
	}

};

Server server;

static int mlua_connect(lua_State* L)
{
	const char* host = luaL_checkstring(L, 1);
	int port = luaL_checkint(L, 2);
	Socket* socket = Socket::connect(host, port);
	if (socket) {
		lua_newtable(L);
		int ref = luaL_ref(L, LUA_REGISTRYINDEX);
		socket->setTable(ref);
		push_socket(L, socket);
		return 1;
	}
	return 0;
}

static int mlua_listen(lua_State* L)
{
	int port = luaL_checkint(L, 1);
	server.listen(port);
	return 0;
}

static int mlua_accept(lua_State* L)
{
	lua_Number timeout = luaL_optnumber(L, 2, 0);
	lua_Number timepassed = 0;
	int max = luaL_optint(L, 3, -1);
	int i = 0;
	while ((max == -1 || i < max) && timepassed <= timeout) {
		lua_Number time;
		Socket* sock = server.accept(timeout - timepassed, &time);
		timepassed += time;

		if (!sock)
			break;

		lua_pushvalue(L, 1); // push the callback

		lua_newtable(L);
		int ref = luaL_ref(L, LUA_REGISTRYINDEX);
		sock->setTable(ref);
		push_socket(L, sock);

		CALL(1, 0);

		i += 1;
	}
	lua_pushnumber(L, timepassed);
	return 1;
}

static int __socket_class_index(lua_State* L)
{
	Socket* socket = pop_socket(L, 1);
	const char* index = luaL_checkstring(L, 2);
	if (strcmp(index, "address") == 0) {
		lua_pushstring(L, socket->getAddress());
	} else {
		lua_rawgeti(L, LUA_REGISTRYINDEX, socket->getTable());
		lua_getfield(L, -1, index);
		if (lua_isnoneornil(L, -1)) {
			lua_pop(L, 2);
			lua_getmetatable(L, 1);
			lua_getfield(L, -1, index);
		}
	}
	return 1;
}

static int __socket_class_newindex(lua_State* L)
{
	Socket* socket = pop_socket(L, 1);
	lua_rawgeti(L, LUA_REGISTRYINDEX, socket->getTable());
	lua_replace(L, 1);
	lua_rawset(L, 1);
	return 0;
}

static int mlua_send_socket(lua_State* L)
{
	Socket* socket = pop_socket(L, 1);
	size_t size;
	const char* message = luaL_checklstring(L, 2, &size);
	bool error = false;
	socket->send(message, size, &error);
	if (error) {
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}
	return 0;
}

static char buffer[1024];
static int mlua_recv_socket(lua_State* L)
{
	Socket* socket = pop_socket(L, 1);
	bool error = false;
	int len = socket->receive(buffer, sizeof(buffer), &error);
	if (error) {
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}
	if (len) {
		lua_pushlstring(L, buffer, len);
		return 1;
	}
	return 0;
}

static int mlua_flush_socket(lua_State* L)
{
	Socket* socket = pop_socket(L, 1);
	bool error = false;
	socket->flush(&error);
	if (error) {
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}
	return 0;
}


static int mlua_disconnect_socket(lua_State* L)
{
	Socket* socket = pop_socket(L, 1);
	socket->disconnect();
	return 0;
}

static int mlua_free_socket(lua_State* L)
{
	Socket* socket = pop_socket(L, 1);
	luaL_unref(L, LUA_REGISTRYINDEX, socket->getTable());
	delete socket;
	return 0;
}

BEGIN_MODULE(net)
	/* CLIENT */
	DECLARE_FUNCTION(connect)

	/* SERVER */
	DECLARE_FUNCTION(listen)
	DECLARE_FUNCTION(accept)

	BEGIN_CLASS(socket)
		ADD_METHOD(socket, send)
		ADD_METHOD(socket, recv)
		ADD_METHOD(socket, flush)
		ADD_METHOD(socket, disconnect)
		ADD_GC(free_socket)
		END_CLASS();
	REGISTER_CLASS_WITH_INDEX_AND_NEWINDEX(socket, "__Socket");
END_MODULE()


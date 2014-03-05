#include <cstring>
#include <list>
#include <sstream>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <lua.hpp>

#include "engine.hpp"

#include "network.hpp"

int (*_socket)(int, int, int) = socket;
int (*_connect)(int, const struct sockaddr*, socklen_t) = connect;
ssize_t (*_send)(int, const void*, size_t, int) = send;
int (*_listen)(int, int) = listen;
int (*_accept)(int, struct sockaddr*, socklen_t*) = accept;

class Socket {
private:
	int fd;
	int tableref;
	int userdataref;
	bool failed;
	std::string address;
	std::string output;
public:
	Socket(int fd, const char* address):
		fd(fd),
		tableref(LUA_REFNIL),
		failed(false),
		address(address)
	{
	}

	static Socket* connect(const char* hostname, int port) {
		struct sockaddr_in serv_addr;
		struct hostent *server;

		int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (fd == -1) {
			return NULL;
		}

		server = gethostbyname(hostname);
		if (server == NULL) {
			return NULL;
		}

		memset(&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		memcpy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
		serv_addr.sin_port = htons(port);
		if (_connect(fd, (const struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
			perror("connect");
			return NULL;
		}

		return new Socket(fd, hostname);
	}

	void send(const char* msg, int len) {
		if (failed) {
			return;
		}
		(void) len;
		output += msg;
		output += '\n';
	}

	int receive(char* buffer, int capacity) {
		if (failed) {
			return 0;
		}
		fd_set sett;
		FD_ZERO(&sett);
		FD_SET(fd, &sett);

		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 0;

		int ret = select(64, &sett, NULL, NULL, &tv);
		if (ret < 0) {
			failed = true;
			perror("select");
			return 0;
		} else if (ret == 0) {
		} else {
			int n = recv(fd, buffer, capacity - 1, 0);
			if (n <= 0) {
				failed = true;
				return 0;
			} else {
				//printf("%d recv\n", n);
				buffer[n - 1] = 0; // remove \n
				return n - 1;
			}
		}
		return 0;
	}

	void flush() {
		if (failed)
			return;

		int len = output.length();
		if (len) {
			int n = _send(fd, output.c_str(), len, 0);
			//printf("sending %s\n", output.c_str());
			//if (n != 0)
				//printf("%d sent\n", n);
			if (n < 0) {
				failed = true;
				perror("send");
				return;
			}
			output.erase(0, n);
		}
	}

	void disconnect() {
		shutdown(fd, SHUT_RDWR);
		close(fd);
	}

	const char* getAddress() const {
		return address.c_str();
	}

	bool hasErrors() const {
		return failed;
	}

	void setTable(int ref) {
		this->tableref = ref;
	}
	int getTable() const {
		return tableref;
	}
	void setUserdata(int ref) {
		this->userdataref = ref;
	}
	int getUserdata() const {
		return userdataref;
	}
};

DECLARE_PUSHPOP(Socket, socket)

class ReceptionCallback {
	lua_State* L;
public:
	ReceptionCallback(lua_State* L) :
		L(L)
	{
	}

	void on_recv(Socket* socket, const char* msg, size_t len) {
		lua_rawgeti(L, LUA_REGISTRYINDEX, socket->getUserdata());
		lua_pushlstring(L, msg, len);
		CALL(2);
	}
};

class DropCallback {
	lua_State* L;
public:
	DropCallback(lua_State* L) :
		L(L)
	{
	}

	void on_drop(Socket* socket) {
		lua_rawgeti(L, LUA_REGISTRYINDEX, socket->getUserdata());
		CALL(1);
	}
};

class Server {
private:
	int fd;
	std::list<Socket*> sockets;
public:

	void listen(int port) {
		fd = _socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		int yes = 1;
		if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) != 0) {
			perror("opt");
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
			return new Socket(sockfd, address);
		}
		return NULL;
	}

	void addSocket(Socket* socket) {
		sockets.push_back(socket);
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

	void recvAll(ReceptionCallback callback) {
		char buffer[512];
		for (std::list<Socket*>::iterator it = sockets.begin(); it != sockets.end(); it++) {
			Socket* socket = *it;
			int len = socket->receive(buffer, sizeof(buffer));
			if (len) {
				callback.on_recv(socket, buffer, len);
			}
		}
	}

	void sendAll(const char* msg, size_t len) {
		for (std::list<Socket*>::iterator it = sockets.begin(); it != sockets.end(); it++) {
			Socket* socket = *it;
			socket->send(msg, len);
		}
	}

	void sendAllExcept(const char* msg, size_t len, Socket* except) {
		for (std::list<Socket*>::iterator it = sockets.begin(); it != sockets.end(); it++) {
			Socket* socket = *it;
			if (socket != except) {
				socket->send(msg, len);
			}
		}
	}

	void flushAll() {
		for (std::list<Socket*>::iterator it = sockets.begin(); it != sockets.end(); it++) {
			Socket* socket = *it;
			socket->flush();
		}
	}

	void dropClients(DropCallback callback) {
		for (std::list<Socket*>::iterator it = sockets.begin(); it != sockets.end(); it++) {
			Socket* socket = *it;
			if (socket->hasErrors()) {
				socket->disconnect();
				printf("drop %s\n", socket->getAddress());
				sockets.erase(it++);
				callback.on_drop(socket);
			}
		}
	}
};
Server server;

static int net_connect(lua_State* L)
{
	const char* host = luaL_checkstring(L, 1);
	int port = luaL_checkint(L, 2);
	Socket* socket = Socket::connect(host, port);
	if (socket) {
		push_socket(L, socket);
		return 1;
	}
	return 0;
}

/*
int mlua_send(lua_State* L)
{
	size_t size;
	const char* message = luaL_checklstring(L, 1, &size);
	int sent_or_error = net.send(message, size);
	lua_pushnumber(L, sent_or_error);
	return 1;
}
int mlua_receive(lua_State* L)
{
	reception_buffer[0] = 0; // assure that even if Network doesn't touch the buffer, it's still valid
	long received_or_error = net.receive(reception_buffer, sizeof(reception_buffer));
	lua_pushnumber(L, received_or_error);
	if (received_or_error > 0) {
		reception_buffer[received_or_error] = 0;
		lua_pushstring(L, reception_buffer);
		return 2;
	}
	return 1;
}
*/

static int net_listen(lua_State* L)
{
	int port = luaL_checknumber(L, 1);
	server.listen(port);
	return 0;
}

static int net_accept(lua_State* L)
{
	lua_Number timeout = luaL_optnumber(L, 2, 0);
	lua_Number timepassed = 0;
	int max = luaL_optint(L, 3, -1);
	int i = 0;
	while ((max == -1 || i < max) && timepassed < timeout) {
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
		ref = luaL_ref(L, LUA_REGISTRYINDEX);
		sock->setUserdata(ref);
		lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

		CALL(1);
		if (!lua_isboolean(L, -1) || lua_toboolean(L, -1) == 1) {
			server.addSocket(sock);
		}

		i += 1;
	}
	lua_pushnumber(L, timepassed);
	return 1;
}

static int net_send_all(lua_State* L)
{
	size_t size;
	const char* message = luaL_checklstring(L, 1, &size);
	server.sendAll(message, size);
	return 0;
}
static int net_send_all_except(lua_State* L)
{
	size_t size;
	const char* message = luaL_checklstring(L, 1, &size);
	Socket* sock = pop_socket(L, 2);
	server.sendAllExcept(message, size, sock);
	return 0;
}

static int net_recv_all(lua_State* L)
{
	ReceptionCallback cb(L);
	server.recvAll(cb);
	return 0;
}

static int net_drop_clients(lua_State* L)
{
	DropCallback cb(L);
	server.dropClients(cb);
	return 0;
}

static int net_flush_all(lua_State* L)
{
	(void) L;
	server.flushAll();
	return 0;
}

static int __socket_class_index(lua_State* L)
{
	Socket* socket = pop_socket(L, 1);
	const char* index = luaL_checkstring(L, 2);
	if (strcmp(index, "address") == 0) {
		lua_pushstring(L, socket->getAddress());
	} else {
		lua_getmetatable(L, 1);
		lua_getfield(L, -1, index);
		if (lua_isnoneornil(L, -1)) {
			lua_pop(L, 2);
			lua_rawgeti(L, LUA_REGISTRYINDEX, socket->getTable());
			lua_replace(L, 1);
			lua_rawget(L, 1);
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
	socket->send(message, size);
	socket->flush();
	return 0;
}

static int mlua_recv_socket(lua_State* L)
{
	Socket* socket = pop_socket(L, 1);
	char buffer[64];
	int len = socket->receive(buffer, sizeof(buffer));
	if (len) {
		lua_pushlstring(L, buffer, len);
		return 1;
	}
	return 0;
}

static int mlua_free_socket(lua_State* L)
{
	Socket* socket = pop_socket(L, 1);
	luaL_unref(L, LUA_REGISTRYINDEX, socket->getUserdata());
	luaL_unref(L, LUA_REGISTRYINDEX, socket->getTable());
	delete socket;
	return 0;
}


#define DECLARE_FUNCTION(x) {#x, net_##x}
static const luaL_Reg lib[] =
{
	/* CLIENT */
	DECLARE_FUNCTION(connect),

	/* SERVER */
	DECLARE_FUNCTION(listen),
	DECLARE_FUNCTION(accept),
	DECLARE_FUNCTION(send_all),
	DECLARE_FUNCTION(send_all_except),
	DECLARE_FUNCTION(recv_all),
	DECLARE_FUNCTION(drop_clients),
	DECLARE_FUNCTION(flush_all),

	{NULL, NULL}
};

DEFINE_EXTENSION(net)
{
	luaL_newlib(L, lib);

	BEGIN_CLASS(socket)
		ADD_METHOD(socket, send)
		ADD_METHOD(socket, recv)
		ADD_GC(free_socket)
		END_CLASS();
	REGISTER_CLASS_WITH_INDEX_AND_NEWINDEX(socket, "__Socket");

	lua_pushnumber(L, NO_ERROR);
	lua_setfield(L, -2, "NO_ERROR");
	lua_pushnumber(L, UNABLE_TO_OPEN_SOCKET);
	lua_setfield(L, -2, "UNABLE_TO_OPEN_SOCKET");
	lua_pushnumber(L, UNABLE_TO_GET_HOST);
	lua_setfield(L, -2, "UNABLE_TO_GET_HOST");
	lua_pushnumber(L, UNABLE_TO_CONNECT);
	lua_setfield(L, -2, "UNABLE_TO_CONNECT");
	lua_pushnumber(L, NOT_CONNECTED);
	lua_setfield(L, -2, "NOT_CONNECTED");
	lua_pushnumber(L, CONNECTION_LOST);
	lua_setfield(L, -2, "CONNECTION_LOST");
	lua_pushnumber(L, ALREADY_DISCONNECTED);
	lua_setfield(L, -2, "ALREADY_DISCONNECTED");
	lua_pushnumber(L, CANNOT_CLOSE_SOCKET);
	lua_setfield(L, -2, "CANNOT_CLOSE_SOCKET");

	return 1;
}


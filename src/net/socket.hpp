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
#pragma once

#include <string>

extern "C" {
#include "websocket.h"
}

class Socket
{
private:
	Socket(const Socket&);
	Socket& operator=(const Socket&);

	int fd;
	int tableref;
	ws_ctx_t* wsctx;
	std::string address;
	std::string output;
public:
	Socket(int fd, const char* address, ws_ctx_t* ctx);

	static Socket* connect(const char* hostname, int port);

	void send(const char* msg, int len, bool* error);
	void flush(bool* error);
	int receive(char* buffer, int capacity, bool* error);
	void disconnect();
	bool readyToRead();
	bool readyToSend();
	const char* getAddress() const;
	void setTable(int ref);
	int getTable() const;
};


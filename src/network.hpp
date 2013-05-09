#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <enet/enet.h>

class Engine;

class Network
{
	private:
		Engine& engine;
		ENetHost* host;
		ENetPeer* peer;

	public:
		Network(Engine&);

		void poll();
		bool connect(const char* hostname, int port);
		void disconnect();
		void send(const void* data, int len);
		void flush();
};

#endif

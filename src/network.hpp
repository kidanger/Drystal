#ifndef NETWORK_HPP
#define NETWORK_HPP

class Engine;

class Network
{
	private:
		Engine& engine;
		int sockfd;

	public:
		Network(Engine&);

		void poll();
		bool connect(const char* hostname, int port);
		void disconnect();
		void send(const void* data, int len);
		void flush();
};

#endif

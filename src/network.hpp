#ifndef NETWORK_HPP
#define NETWORK_HPP

class Engine;

class Network
{
	private:
		Engine& engine;
		int sockfd;

	public:
		Network(Engine &engine);
		~Network();

		void poll();
		bool connect(const char* hostname, int port);
		void disconnect();
		void send(const void* data, size_t len);
		void flush();
};

#endif

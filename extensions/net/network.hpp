#ifndef NETWORK_HPP
#define NETWORK_HPP

class Network
{
	private:
		int sockfd;

	public:
		Network();
		~Network();

		void poll();
		bool connect(const char* hostname, int port);
		void disconnect();
		void send(const void* data, unsigned long len);
		void flush();
};

#endif

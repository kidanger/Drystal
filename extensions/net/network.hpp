#ifndef NETWORK_HPP
#define NETWORK_HPP

enum ErrorCode {
	NO_ERROR=0,

	// connection errors
	UNABLE_TO_OPEN_SOCKET=1,
	UNABLE_TO_GET_HOST=2,
	UNABLE_TO_CONNECT=3,

	// send, receive errors
	NOT_CONNECTED=-1,
	CONNECTION_LOST=-2,

	// disconnect
	ALREADY_DISCONNECTED=-1,
	CANNOT_CLOSE_SOCKET=-2, // very unlikely to happen
};

class Network
{
	private:
		int sockfd;

	public:
		Network();
		~Network();

		ErrorCode connect(const char* hostname, int port);
		long send(const char* data, unsigned long len);
		long receive(char* buffer, unsigned long len);
		ErrorCode disconnect();
};

#endif

#include "module.hpp"
#include "socket_bind.hpp"
#include "server_bind.hpp"

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
	REGISTER_CLASS_WITH_INDEX_AND_NEWINDEX(socket, "__Socket")
END_MODULE()


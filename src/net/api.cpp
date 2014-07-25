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
#include "module.hpp"
#include "socket_bind.hpp"
#include "server_bind.hpp"
#include "api.hpp"

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


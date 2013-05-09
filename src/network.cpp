#include <cstdio>
#include <enet/enet.h>

#include "emscripten.h"

#include "engine.hpp"
#include "network.hpp"

ENetHost * host;

void main_loop() {
	static int counter = 0;
#if EMSCRIPTEN
	counter++;
#endif
	if (counter == 100) {
		printf("stop!\n");
		emscripten_cancel_main_loop();
		return;
	}
	printf("stop!\n");

}

void Network::poll()
{
	ENetEvent event;
	if (enet_host_service (host, & event, 0) == 0) return;
	switch (event.type)
	{
		case ENET_EVENT_TYPE_CONNECT:
			engine.net_connected();
			break;
		case ENET_EVENT_TYPE_RECEIVE:
			//printf("A packet of length %lu containing %s was received from %s on channel %u.\n",
			//        event.packet->dataLength,
			//        event.packet->data,
			//        event.peer->data,
			//        event.channelID);

			/* Clean up the packet now that we're done using it. */
			engine.net_recv(event.packet->data);
			enet_packet_destroy(event.packet);
			break;
		case ENET_EVENT_TYPE_DISCONNECT:
			printf("%s disconnected.\n", event.peer->data);
			/* Reset the peer's client information. */
			event.peer->data = NULL;
			//enet_host_destroy(host);
			engine.net_disconnected();
			break;
		default:
			printf("whaaa? %d\n", event.type);
	}
}

Network::Network(Engine& e) : engine(e)
{
	if (enet_initialize () != 0)
	{
		fprintf (stderr, "An error occurred while initializing ENet.\n");
	}
	atexit (enet_deinitialize);
}

bool Network::connect(const char* hostname, int port)
{
	host = enet_host_create (NULL /* create a client host */,
			1 /* only allow 1 outgoing connection */,
			2 /* allow up 2 channels to be used, 0 and 1 */,
			57600 / 8 /* 56K modem with 56 Kbps downstream bandwidth */,
			14400 / 8 /* 56K modem with 14 Kbps upstream bandwidth */);
	if (host == NULL)
	{
		fprintf (stderr,
				"An error occurred while trying to create an ENet client host.\n");
		return false;
	}

	ENetAddress address;
	enet_address_set_host(&address, hostname);
	address.port = port;

	printf("connecting to server...\n");

	peer = enet_host_connect(host, &address, 2, 0);
	if (peer == NULL)
	{
		fprintf (stderr,
				"No available peers for initiating an ENet connection.\n");
		return false;
	}
	return true;
}

void Network::disconnect()
{
	enet_peer_disconnect_now(peer, 0);
	enet_peer_reset(peer);
}

void Network::send(const void* data, int len)
{
	ENetPacket* packet = enet_packet_create(data, len, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, packet);
}

void Network::flush()
{
	enet_host_flush (host);
}


#!/usr/bin/env python2
# -*- coding:utf-8 -*-

import time
import socket
import random
from Client import Client

MAX_CLIENTS = 5
TICK_SPEED  = 40

class Server:
    def __init__(self):
        Client.server = self
        self.is_running = False

        # The client list:
        self.clients = [None] * MAX_CLIENTS

        # The main socket, to receive connections.
        self.main_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.main_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.main_socket.bind(("localhost", 1234))
        self.main_socket.setblocking(False)
        self.main_socket.listen(len(self.clients))

    def send(self, message, only=[], to=None, dont=None):
        """
        Send a message to everyone (if only is [] and to is None)
            or just few clients.
        Can avoid sending on one client, passing its instance as dont=instance.
        """
        if only or to:
            to and to.send(message)
            for client in only:
                client.send(message)
            return

        for client in [c for c in self.clients if c and c is not dont]:
            client.send(message)


    def handle_connection(self, client_socket, address):
        """Handle a new client connection."""
        # Search for a valid slot.
        slot_index = [index for index, client in enumerate(self.clients)
                        if not client][0]

        print('new client')
        # Set the client socket to non blocking.
        client_socket.setblocking(False)

        # Instanciate the new client.
        self.clients[slot_index] = Client(client_socket, slot_index)

        new_client = self.clients[slot_index]

        self.send("coucou")
        print slot_index, "connected. :", address

    def run(self):
        """The loop: check new connections, check new messages, and send new info to clients."""
        self.is_running = True
        tick = 0
        while self.is_running:
            tick += 1
            # Handle sleep
            time_before = time.time()

            # Check incomming
            try:
                client_socket, address = self.main_socket.accept()
            except:
                pass # Nobody want to connect :(
            else:
                self.handle_connection(client_socket, address)

            count = len([c for c in self.clients if c])
            if not count:
                time.sleep(1)
                continue

            # Check clients messages
            for client in [c for c in self.clients if c]:
                client.recv()
                if client.errors >= 3:
                    print "Too many errors! from " + client.nick
                    client.close()

            self.send("lol")

            # flush clients
            [c.flush() for c in self.clients if c]

            # Keep the CPU usage low
            time_used = time.time() - time_before
            time.sleep(time_used < 1./TICK_SPEED and 1./TICK_SPEED - time_used)


    def stop(self):
        """Stop the server and close sockets."""
        self.is_running = False
        for client in self.clients:
            client and client.close()
        self.main_socket.close()

if __name__ == '__main__':
    server = Server()
    try:
        server.run()

    except KeyboardInterrupt:
        pass

    except:
        # Print traceback
        import sys
        import traceback
        print
        traceback.print_tb(sys.exc_info()[2])
        print sys.exc_info()[1]
        print

    # Stop the server
    server.stop()
    print "Server shutdowned."

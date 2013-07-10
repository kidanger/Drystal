#!/usr/bin/env python
# -*- coding:utf-8 -*-

import socket

class Client:
    server = None
    def __init__(self, socket, index):
        self.socket = socket

        self.index = index
        self.nick = "Player" + str(self.index)
        self.buffer = ''

        # Network
        self.errors = 0 # If too many strange message, the player get disconnected.


    def send(self, message):
        self.buffer += message + '\n'

    def dosend(self, message):
        """Send to message to the client."""
        try:
            fail = self.socket.sendall(message + '\n')
        except socket.error as e:
            print e
            self.close()
        else:
            if fail:
                print 'failed to sendall'
                self.close()

    def flush(self):
        if self.buffer:
            self.dosend(self.buffer)
            self.buffer = ''

    def recv(self):
        """Handle what the client send to the server."""
        try:
            recv = self.socket.recv(1024)
        except socket.error:
            # Nothing to receive..
            return

        if not recv or recv[-1] != '\n':
            print recv
            self.close()
            return

        recv = recv[:-1]
        for message in recv.split('\n'):
            if not message:
                continue
            print(message)

    def close(self):
        """Close the connection, delete this class and notify it to players."""
        print self.nick, "disconnected."
        self.socket.close()
        if self in self.server.clients:
            self.server.clients[self.server.clients.index(self)] = None


#pragma once

#include <stdlib.h>

#define BUFSIZE 65536
#define DBUFSIZE (BUFSIZE * 3) / 4 - 20

#define SERVER_HANDSHAKE_HYBI "HTTP/1.1 101 Switching Protocols\r\n\
Upgrade: websocket\r\n\
Connection: Upgrade\r\n\
Sec-WebSocket-Accept: %s\r\n\
Sec-WebSocket-Protocol: %s\r\n\
\r\n"

#define HYBI_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

#define HYBI10_ACCEPTHDRLEN 29

typedef struct {
    char path[1024+1];
    char host[1024+1];
    char origin[1024+1];
    char version[1024+1];
    char connection[1024+1];
    char protocols[1024+1];
    char key1[1024+1];
    char key2[1024+1];
    char key3[8+1];
} headers_t;

typedef struct {
    int        sockfd;
    int        hybi;
    headers_t *headers;
    char* cin_buf;
    char* cout_buf;
    char* tin_buf;
    char* tout_buf;
    int tin_start;
    int tout_start;
    int cin_start;
    int cout_start;
    int tin_end;
    int tout_end;
    int cin_end;
    int cout_end;
} ws_ctx_t;

typedef struct {
    int verbose;
    char listen_host[256];
    int listen_port;
    void (*handler)(ws_ctx_t*);
    int handler_id;
    char *cert;
    char *key;
    int daemon;
    int run_once;
} settings_t;


ws_ctx_t *do_handshake(int sock);
ssize_t ws_recv(ws_ctx_t *ctx, void *buf, size_t len);
ssize_t ws_send(ws_ctx_t *ctx, const void *buf, size_t len);
void free_ws_ctx(ws_ctx_t *ctx);

#define gen_handler_msg(stream, ...) \
    fprintf(stream, "  %d: ", settings.handler_id); \
    fprintf(stream, __VA_ARGS__); \

#define handler_msg(...) fprintf(stdout, __VA_ARGS__);
#define handler_emsg(...) fprintf(stderr, __VA_ARGS__);


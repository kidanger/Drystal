/*
 * WebSocket lib
 * Copyright 2010 Joel Martin
 * Licensed under LGPL version 3 (see docs/LICENSE.LGPL-3)
 * Modified in 2014 by Jérémy Anger
 *
 * You can make a cert/key with openssl using:
 * openssl req -new -x509 -days 365 -nodes -out self.pem -keyout self.pem
 * as taken from http://docs.python.org/dev/library/ssl.html#certificates
 */
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <resolv.h>      // base64 encode/decode
#include <openssl/md5.h> // md5 hash
#include <openssl/sha.h> // sha1 hash

#include "websocket.h"



/* ------------------------------------------------------- */


static int encode_hybi(const char *src, size_t srclength,
        char *target, size_t targsize, unsigned int opcode)
{
    (void) targsize;
    unsigned long payload_offset = 2;

    if ((int)srclength <= 0) {
        return 0;
    }

    target[0] = (char)((opcode & 0x0F) | 0x80);

    if (srclength <= 125) {
        target[1] = (char) srclength;
        payload_offset = 2;
    } else if ((srclength > 125) && (srclength < 65536)) {
        target[1] = (char) 126;
        *(u_short*)&(target[2]) = htons(srclength);
        payload_offset = 4;
    } else {
        handler_emsg("Sending frames larger than 65535 bytes not supported\n");
        return -1;
    }
    memcpy(target + payload_offset, src, srclength);

    return payload_offset + srclength;
}

static int decode_hybi(unsigned char *src, size_t srclength,
        unsigned char *target, size_t targsize,
        unsigned int *opcode, unsigned int *left)
{
    unsigned char *frame;
    unsigned char *mask;
    unsigned char *payload;
    unsigned char save_char;
    /*char cntstr[4];*/
    int masked = 0;
    unsigned int i = 0;
    int len, framecount = 0;
    size_t remaining;
    unsigned int target_offset = 0, hdr_length = 0, payload_length = 0;

    *left = srclength;
    frame = src;

    //printf("Deocde new frame\n");
    while (1) {
        // Need at least two bytes of the header
        // Find beginning of next frame. First time hdr_length, masked and
        // payload_length are zero
        frame += hdr_length + 4*masked + payload_length;
        //printf("frame[0..3]: 0x%x 0x%x 0x%x 0x%x (tot: %d)\n",
        //       (unsigned char) frame[0],
        //       (unsigned char) frame[1],
        //       (unsigned char) frame[2],
        //       (unsigned char) frame[3], srclength);

        if (frame > src + srclength) {
            //printf("Truncated frame from client, need %d more bytes\n", frame - (src + srclength) );
            break;
        }
        remaining = (src + srclength) - frame;
        if (remaining < 2) {
            //printf("Truncated frame header from client\n");
            break;
        }
        framecount ++;

        *opcode = frame[0] & 0x0f;
        masked = (frame[1] & 0x80) >> 7;

        if (*opcode == 0x8) {
            // client sent orderly close frame
            break;
        }

        payload_length = frame[1] & 0x7f;
        if (payload_length < 126) {
            hdr_length = 2;
            //frame += 2 * sizeof(char);
        } else if (payload_length == 126) {
            payload_length = (frame[2] << 8) + frame[3];
            hdr_length = 4;
        } else {
            handler_emsg("Receiving frames larger than 65535 bytes not supported\n");
            return -1;
        }
        if ((hdr_length + 4*masked + payload_length) > remaining) {
            continue;
        }
        //printf("    payload_length: %u, raw remaining: %u\n", payload_length, remaining);
        payload = frame + hdr_length + 4*masked;

        if (*opcode != 1 && *opcode != 2) {
            handler_msg("Ignoring non-data frame, opcode 0x%x\n", *opcode);
            continue;
        }

        if (payload_length == 0) {
            handler_msg("Ignoring empty frame\n");
            continue;
        }

        if ((payload_length > 0) && (!masked)) {
            handler_emsg("Received unmasked payload from client\n");
            return -1;
        }

        // Terminate with a null for base64 decode
        save_char = payload[payload_length];
        payload[payload_length] = '\0';

        // unmask the data
        mask = payload - 4;
        for (i = 0; i < payload_length; i++) {
            payload[i] ^= mask[i%4];
        }

        if (0) {
            // base64 decode the data
            len = b64_pton((const char*)payload, target+target_offset, targsize);
        } else {
            len = payload_length;
            memcpy(target+target_offset, payload, len);
        }

        // Restore the first character of the next frame
        payload[payload_length] = save_char;
        if (len < 0) {
            handler_emsg("Base64 decode error code %d", len);
            return len;
        }
        target_offset += len;

        //printf("    len %d, raw %s\n", len, frame);
    }

    if (framecount > 1) {
        /*snprintf(cntstr, 3, "%d", framecount);*/
        /*traffic(cntstr);*/
    }

    *left = remaining;
    return target_offset;
}




/* ------------------------------------------------------- */

ssize_t ws_recv(ws_ctx_t *ctx, void *buf, size_t buflen) {
    int bytes;
    int left;
    int opcode;

    bytes = recv(ctx->sockfd, ctx->tin_buf + ctx->tin_end, BUFSIZE-1, 0);
    if (bytes <= 0) {
        if (bytes < 0)
            handler_emsg("client closed connection\n");
        return bytes;
    }
    ctx->tin_end += bytes;

    int len;
    assert(ctx->hybi);
    len = decode_hybi(ctx->tin_buf + ctx->tin_start,
            ctx->tin_end - ctx->tin_start,
            ctx->tout_buf, BUFSIZE-1,
            &opcode, &left);

    if (opcode == 8) {
        handler_emsg("client sent orderly close frame\n");
        shutdown(ctx->sockfd, SHUT_RDWR);
        close(ctx->sockfd);
        return -1;
    }
    if (len < 0) {
        handler_emsg("decoding error\n");
        return -1;
    }
    if (left) {
        ctx->tin_start = ctx->tin_end - left;
        printf("partial frame from client\n");
    } else {
        ctx->tin_start = 0;
        ctx->tin_end = 0;
        memcpy(buf, ctx->tout_buf, len);
        printf("complete frame from client\n");
    }
    return len;
}

ssize_t ws_send(ws_ctx_t *ctx, const void *buf, size_t buflen) {
    assert(ctx->hybi);
    int len = encode_hybi(buf, buflen, ctx->cout_buf, BUFSIZE, 2);

    if (len < 0) {
        handler_emsg("encoding error\n");
        return -1;
    }

    int bytes = send(ctx->sockfd, ctx->cout_buf, len, 0);
    assert(bytes == len);
    if (0) {
        handler_emsg("len: %d, bytes: %d: %d\n",
                len, bytes,
                (int) *(ctx->cout_buf + ctx->cout_start));
    }

    return len;
}

ws_ctx_t *alloc_ws_ctx() {
    ws_ctx_t *ctx;
    ctx = (ws_ctx_t*) malloc(sizeof(ws_ctx_t));

    ctx->cin_buf = (char*) malloc(BUFSIZE);
    ctx->cout_buf = (char*) malloc(BUFSIZE);
    ctx->tin_buf = (char*) malloc(BUFSIZE);
    ctx->tout_buf = (char*) malloc(BUFSIZE);
    ctx->tin_start = ctx->tin_end = 0;
    ctx->tout_start = ctx->tout_end = 0;
    ctx->cin_start = ctx->cin_end = 0;
    ctx->cout_start = ctx->cout_end = 0;

    ctx->headers = (headers_t*) malloc(sizeof(headers_t));
    return ctx;
}

void free_ws_ctx(ws_ctx_t *ctx) {
    free(ctx->cin_buf);
    free(ctx->cout_buf);
    free(ctx->tin_buf);
    free(ctx->tout_buf);
    free(ctx);
}

void ws_socket(ws_ctx_t *ctx, int socket) {
    ctx->sockfd = socket;
}

void ws_socket_free(ws_ctx_t *ctx) {
    if (ctx->sockfd) {
        shutdown(ctx->sockfd, SHUT_RDWR);
        close(ctx->sockfd);
        ctx->sockfd = 0;
    }
}


int parse_handshake(ws_ctx_t *ws_ctx, char *handshake) {
    char *start, *end;
    headers_t *headers = ws_ctx->headers;

    headers->key1[0] = '\0';
    headers->key2[0] = '\0';
    headers->key3[0] = '\0';

    if ((strlen(handshake) < 92) || (bcmp(handshake, "GET ", 4) != 0)) {
        return 0;
    }
    start = handshake+4;
    end = strstr(start, " HTTP/1.1");
    if (!end) {
        return 0;
    }
    strncpy(headers->path, start, end-start);
    headers->path[end-start] = '\0';

    start = strstr(handshake, "\r\nHost: ");
    if (!start) {
        return 0;
    }
    start += 8;
    end = strstr(start, "\r\n");
    strncpy(headers->host, start, end-start);
    headers->host[end-start] = '\0';

    headers->origin[0] = '\0';
    start = strstr(handshake, "\r\nOrigin: ");
    if (start) {
        start += 10;
    } else {
        start = strstr(handshake, "\r\nSec-WebSocket-Origin: ");
        if (!start) {
            return 0;
        }
        start += 24;
    }
    end = strstr(start, "\r\n");
    strncpy(headers->origin, start, end-start);
    headers->origin[end-start] = '\0';

    start = strstr(handshake, "\r\nSec-WebSocket-Version: ");
    if (start) {
        // HyBi/RFC 6455
        start += 25;
        end = strstr(start, "\r\n");
        strncpy(headers->version, start, end-start);
        headers->version[end-start] = '\0';
        ws_ctx->hybi = strtol(headers->version, NULL, 10);

        start = strstr(handshake, "\r\nSec-WebSocket-Key: ");
        if (!start) {
            return 0;
        }
        start += 21;
        end = strstr(start, "\r\n");
        strncpy(headers->key1, start, end-start);
        headers->key1[end-start] = '\0';

        start = strstr(handshake, "\r\nConnection: ");
        if (!start) {
            return 0;
        }
        start += 14;
        end = strstr(start, "\r\n");
        strncpy(headers->connection, start, end-start);
        headers->connection[end-start] = '\0';

        start = strstr(handshake, "\r\nSec-WebSocket-Protocol: ");
        if (!start) {
            return 0;
        }
        start += 26;
        end = strstr(start, "\r\n");
        strncpy(headers->protocols, start, end-start);
        headers->protocols[end-start] = '\0';
    } else {
        return 0;
    }

    return 1;
}

static void gen_sha1(headers_t *headers, char *target) {
    SHA_CTX c;
    unsigned char hash[SHA_DIGEST_LENGTH];
    int r;

    SHA1_Init(&c);
    SHA1_Update(&c, headers->key1, strlen(headers->key1));
    SHA1_Update(&c, HYBI_GUID, 36);
    SHA1_Final(hash, &c);

    r = b64_ntop(hash, sizeof hash, target, HYBI10_ACCEPTHDRLEN);
    //assert(r == HYBI10_ACCEPTHDRLEN - 1);
}


ws_ctx_t *do_handshake(int sock) {
    char handshake[4096], response[4096], sha1[29];
    const char *scheme;
    headers_t *headers;
    int len, i, offset;
    ws_ctx_t * ws_ctx;

    // Peek, but don't read the data
    len = recv(sock, handshake, 1024, MSG_PEEK);
    handshake[len] = 0;
    if (len == 0) {
        handler_msg("ignoring empty handshake\n");
        return NULL;
    } else if (bcmp(handshake, "<policy-file-request/>", 22) == 0) {
        handler_msg("ignoring policy request\n");
        return NULL;
    } else if ((bcmp(handshake, "\x16", 1) == 0) ||
            (bcmp(handshake, "\x80", 1) == 0)) {
        handler_msg("SSL connection but no cert specified\n");
        return NULL;
    } else if (strncmp(handshake, "GET ", 4) == 0) {
        ws_ctx = alloc_ws_ctx();
        ws_socket(ws_ctx, sock);
        if (! ws_ctx) { return NULL; }
        scheme = "ws";
        handler_msg("using plain socket\n");
    } else {
        handler_msg("native socket: %s\n", handshake);
        return NULL;
    }
    offset = 0;
    for (i = 0; i < 10; i++) {
        len = recv(ws_ctx->sockfd, handshake+offset, 4096, 0);
        if (len == 0) {
            handler_emsg("Client closed during handshake\n");
            return NULL;
        }
        offset += len;
        handshake[offset] = 0;
        if (strstr(handshake, "\r\n\r\n")) {
            break;
        }
        usleep(10);
    }

    handler_msg("handshake: %s\n", handshake);
    if (!parse_handshake(ws_ctx, handshake)) {
        handler_emsg("Invalid WS request\n");
        return NULL;
    }

    headers = ws_ctx->headers;
    if (ws_ctx->hybi > 0) {
        handler_msg("using protocol HyBi/IETF 6455 %d\n", ws_ctx->hybi);
        gen_sha1(headers, sha1);
        sprintf(response, SERVER_HANDSHAKE_HYBI, sha1, "binary");
    } else {
        handler_msg("pixie protocol unsupported\n");
        return NULL;
    }

    //handler_msg("response: %s\n", response);
    send(ws_ctx->sockfd, response, strlen(response), 0);

    return ws_ctx;
}


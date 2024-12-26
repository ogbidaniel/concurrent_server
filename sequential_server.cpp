#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#pragma comment(lib, "ws2_32.lib")

typedef enum { WAIT_FOR_MSG, IN_MSG } ProcessingState;

void perror_die(const char* message) {
    fprintf(stderr, "%s: %d\n", message, WSAGetLastError());
    exit(1);
}

void serve_connection(SOCKET sockfd) {
    if (send(sockfd, "*", 1, 0) < 1) {
        perror_die("send");
    }

    ProcessingState state = WAIT_FOR_MSG;

    while (1) {
        uint8_t buf[1024];
        int len = recv(sockfd, (char*)buf, sizeof buf, 0);
        if (len < 0) {
            perror_die("recv");
        } else if (len == 0) {
            break;
        }

        for (int i = 0; i < len; ++i) {
            switch (state) {
            case WAIT_FOR_MSG:
                if (buf[i] == '^') {
                    state = IN_MSG;
                }
                break;
            case IN_MSG:
                if (buf[i] == '$') {
                    state = WAIT_FOR_MSG;
                } else {
                    buf[i] += 1;
                    if (send(sockfd, (char*)&buf[i], 1, 0) < 1) {
                        perror_die("send");
                    }
                }
                break;
            }
        }
    }

    closesocket(sockfd);
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        perror_die("WSAStartup failed");
    }

    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
        perror_die("socket");
    }

    struct sockaddr_in server_addr = { 0 };
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(9090);

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        perror_die("bind");
    }

    if (listen(sockfd, SOMAXCONN) == SOCKET_ERROR) {
        perror_die("listen");
    }

    printf("Server listening on port 9090...\n");

    while (1) {
        struct sockaddr_in client_addr;
        int addr_len = sizeof(client_addr);
        SOCKET client_sock = accept(sockfd, (struct sockaddr*)&client_addr, &addr_len);
        if (client_sock == INVALID_SOCKET) {
            perror_die("accept");
        }

        printf("Client connected\n");
        serve_connection(client_sock);
        printf("Client disconnected\n");
    }

    closesocket(sockfd);
    WSACleanup();
    return 0;
}

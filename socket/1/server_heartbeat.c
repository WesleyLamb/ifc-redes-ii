// Fonte: https://www.inf.pucrs.br/~cnunes/redes_si/sockets/teste_server.cpp
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include "src/helpers.h"

int main() {
    srand(time(NULL));

    int serverSocket, status, socketPort = 9001, clientAddressLen, randomNumber, bufferStrLen;
    struct sockaddr_in serverAddress, clientAddress;
    char buffer[BUFSIZ], responseBuffer[BUFSIZ], *helper, *helper2;
    int recvSequence = 0, currSequence = 0;
    time_t currTime;
    struct tm *t;
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    helper = malloc(sizeof(char) * 32);
    int firstPacket = 1;

    serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket < 0) {
        printf("Nao foi possivel criar o socket (%d).\n", errno);
        return 1;
    }

    // setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(socketPort);
    status = bind(serverSocket, (struct sockaddr *)&serverAddress,
                  sizeof(serverAddress));
    if (status < 0) {
        printf("Nao foi possivel vincular o socket a porta %d. Certifique-se de que a porta nao esta em uso (%d).\n", socketPort, errno);
        return 1;
    }

    while (1) {
        memset(&buffer, 0, BUFSIZ);

        clientAddressLen = sizeof(clientAddress);
        status = recvfrom(serverSocket, buffer, BUFSIZ, 0, (struct sockaddr *) &clientAddress, &clientAddressLen);
        currTime = time(NULL);
        t = localtime(&currTime);
        if (firstPacket) {
            setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
            firstPacket = 0;
        }

        if (status < 0) {
            memset(helper, 0, 32);
            datetimetostr(t, helper);
            printf("No heartbeat at %s.\n", helper);
            continue;
        }
        printf("Online: %s\n", buffer);
    }

    return 0;
}
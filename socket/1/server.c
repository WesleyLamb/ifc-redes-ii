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
#include <arpa/inet.h>

int main() {
    srand(time(NULL));

    int serverSocket, status, socketPort = 9001, clientAddressLen, randomNumber, bufferStrLen;
    struct sockaddr_in serverAddress, clientAddress;
    char buffer[BUFSIZ], responseBuffer[BUFSIZ];

    serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket < 0) {
        printf("Nao foi possivel criar o socket (%d).\n", errno);
        return 1;
    }

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

        printf("Aguardando mensagem.\n");
        clientAddressLen = sizeof(clientAddress);
        status = recvfrom(serverSocket, buffer, BUFSIZ, 0, (struct sockaddr *) &clientAddress, &clientAddressLen);

        if (status < 0) {
            printf("Nao foi possivel receber os dados do cliente (%d).\n", errno);
            continue;
        }
        printf("Nova mensagem: %s - cliente %s porta %d\n", buffer, inet_ntoa(clientAddress.sin_addr), clientAddress.sin_port);

        randomNumber = rand() % 11;
        if (randomNumber < 4) {
            printf("Vou trollar o cliente e nao responderei a mensagem.\n");
        } else {
            printf("Vou responder a mensagem.\n");
            memset(&responseBuffer, 0, BUFSIZ);
            bufferStrLen = strlen(buffer);
            for (int i = 0; i < bufferStrLen; i++) {
                responseBuffer[i] = toupper(buffer[i]);
            }
            sendto(serverSocket, responseBuffer, BUFSIZ, 0, (struct sockaddr *) &clientAddress, clientAddressLen);
        }

        // printf("%s de %s:%d\n", buffer, clientAddress.sin_addr.s_addr, ntohs(clientAddress.sin_port));
    }

    return 0;
}
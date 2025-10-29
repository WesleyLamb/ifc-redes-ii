// Fonte: https://www.linuxhowtos.org/C_C++/socket.htm
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

#define BUFFER_LEN 1024
#define MAX_WAITING_CONNECTIONS 5

int main(int argc, char *argv[])
{
    srand(time(NULL));
    int serverSocket, clientSocket, socketPort = 9001;
    socklen_t clientSocketLen;
    char buffer[BUFFER_LEN];
    struct sockaddr_in serverAddress, clientAddress;
    int status;

    serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket < 0)
    {
        printf("Nao foi possivel criar o socket (%d).\n", errno);
        return 1;
    }

    memset(&serverAddress, 0, sizeof(serverAddress));

    //  socketPort = atoi(argv[1]);
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(socketPort);
    status = bind(serverSocket, (struct sockaddr *)&serverAddress,
                  sizeof(serverAddress));
    if (status < 0) {
        printf("Nao foi possivel vincular o socket a porta %d. Certifique-se de que a porta nao esta em uso (%d).\n", socketPort, errno);
        return 1;
    }

    status = listen(serverSocket, 5);
    if (status < 0)
    {
        printf("Nao foi possivel ouvir a porta %d (%d).\n", socketPort, errno);
        return 1;
    }

    clientSocketLen = sizeof(clientAddress);

    while (1) {
        clientSocket = accept(serverSocket,
                              (struct sockaddr *)&clientAddress,
                              &clientSocketLen);

        if (clientSocket < 0) {
            printf("Nao foi possivel criar o arquivo do retorno (%d).\n", errno);
            return 1;
        }

        memset(&buffer, 0, BUFFER_LEN);
        status = read(clientSocket, buffer, BUFFER_LEN - 1);
        if (status < 0) {
                printf("Nao foi possivel abrir o arquivo para realizar a leitura (%d).\n", errno);
            return 1;
        }

        printf("Mensagem recebida do cliente: %s\n", buffer);

        int n = (random() % 11);
        if (n < 4) {
            printf("Vou trollar o cliente e nao responder ele\n");
        } else {
            printf("Vou responder ele\n");
            char response[BUFFER_LEN];
            int responseLen = strlen(response);
            for (int i = 0; i < responseLen; i++) {
                response[i] = toupper(response[i]);
            }

            status = write(clientSocket, &response, responseLen);
            if (status < 0) {
                printf("Erro ao enviar a resposta (%d)", errno);
                return 1;
            }
        }
    }

    close(clientSocket);
    close(serverSocket);
    return 0;
}

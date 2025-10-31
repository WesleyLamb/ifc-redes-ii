#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include "src/helpers.h"

#define ITERATIONS 10

int main(int argc, char *argv[])
{
    int clientSocket, status, serverAddressLen;
    struct sockaddr_in clientAddress, serverAddress;
    struct hostent *host;
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    char buffer[BUFSIZ];
    char* helper;
    helper = malloc(sizeof(char) * 32);

    // Retorna o IP do host especificado
    host = gethostbyname("localhost");
    if (host == NULL) {
        printf("Nao foi possivel estabelecer a conexao com o servidor, certifique-se de que o mesmo esteja rodando (%d). \n", errno);
        return 1;
    }

    serverAddress.sin_family = host->h_addrtype;
    memcpy((char*) &serverAddress.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
    serverAddress.sin_port = htons(9001);
    int losses = 0;
    int timeElapsed[ITERATIONS];
    int max = 0;
    int min = __INT_MAX__;
    int avg = 0;
    int i = 1;

    while (1) {
        // Cria o socket
        clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
        if (clientSocket < 0) {
            printf("Nao criar o socket (%d).\n", errno);
            return 1;
        }
        // Configura o timeout no socket
        setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

        clientAddress.sin_family = AF_INET;
        clientAddress.sin_addr.s_addr = INADDR_ANY;
        clientAddress.sin_port = 0;

        // Vincula o socket a uma porta na máquina de origem (aleatória e alta)
        status = bind(clientSocket, (struct sockaddr *) &clientAddress, sizeof(clientAddress));
        if (status < 0) {
            printf("Nao foi possivel se vincular a porta (%d).\n", errno);
            return 1;
        }

        time_t sendTime = time(NULL);
        struct tm *t = localtime(&sendTime);

        // Envia os dados ao destinatário
        memset(helper, 0, 32);
        itoa(i, helper);
        memset(&buffer, 0, BUFSIZ);
        strcpy((char*)&buffer, helper);
        int lastPosOfBuffer = strlen((char*)&buffer);
        buffer[lastPosOfBuffer] = ' ';
        lastPosOfBuffer++;
        memset(helper, 0, 32);
        datetimetostr(t, helper);

        copy(&buffer[lastPosOfBuffer], helper, strlen(helper));

        printf("Enviando %s\n", buffer);
        status = sendto(clientSocket, &buffer, 32, 0, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
        if (status < 0) {
            printf("Nao foi possivel enviar o ping ao servidor ($%d).\n", errno);
            return 1;
        }
        i++;
        usleep(250000);
        close(clientSocket);
    }
}

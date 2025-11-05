#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

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

    // Solicita ao SO o IP do host especificado
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

    for (int i = 0; i < ITERATIONS; i++) {
        // Cria o socket
        clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
        if (clientSocket < 0) {
            printf("Nao criar o socket (%d).\n", errno);
            return 1;
        }
        // Configura o timeout no socket
        setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

        // AF_INET = IP 4 ou 6
        // INADDR_ANY = Aceita requisição de qualquer origem (localhost, rede)
        // SIN_PORT 0 = porta a ser utilizada para a requisição será definida pelo SO
        clientAddress.sin_family = AF_INET;
        clientAddress.sin_addr.s_addr = INADDR_ANY;
        clientAddress.sin_port = 0;

        // Vincula o socket a uma porta na máquina de origem (aleatória e alta)
        status = bind(clientSocket, (struct sockaddr *) &clientAddress, sizeof(clientAddress));
        if (status < 0) {
            printf("Nao foi possivel se vincular a porta (%d).\n", errno);
            return 1;
        }

        clock_t sendTime = clock();
        clock_t recvTime = 0;
        // Envia os dados ao destinatário
        status = sendto(clientSocket, "ping\0", 5, 0, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
        if (status < 0) {
            printf("Nao foi possivel enviar o ping ao servidor ($%d).\n", errno);
            return 1;
        }
        memset(&buffer, 0, BUFSIZ);
        // Recebe do destinatário (com timeout de 1s)
        status = recvfrom(clientSocket, &buffer, BUFSIZ, 0, (struct sockaddr *) &serverAddress, &serverAddressLen);
        // Se status < 0, é erro
        if (status < 0) {
            printf("Ping %d %s (%d)\n", i + 1, "Request timed out", errno);
            losses++;
        } else {
            printf("%s\n",buffer);
            recvTime = clock();
            timeElapsed[i] = (recvTime - sendTime);
            if (timeElapsed[i] < min) {
                min = timeElapsed[i];
            }

            if (timeElapsed[i] > max) {
                max = timeElapsed[i];
            }

            avg += timeElapsed[i];

            printf("Ping %d %fs\n", i + 1, (double)timeElapsed[i] / (double)CLOCKS_PER_SEC);
            sleep(1);
        }
        // Encerra o socket
        close(clientSocket);
    }
    double lossesPerc = ((double)losses / (double)ITERATIONS) * 100;
    printf("Total: \n");
    printf("Packages sent %d\n", ITERATIONS - losses);
    printf("Packages lost: %d (%.2f%%)\n", losses, lossesPerc);
    printf("Avg: %fs, Min: %fs, Max: %fs\n", ((double) avg / (double) ITERATIONS) / (double)CLOCKS_PER_SEC, (double) min/ (double)CLOCKS_PER_SEC, (double) max/ (double)CLOCKS_PER_SEC);

}

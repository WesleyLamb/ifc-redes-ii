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

/* reverse:  reverse string s in place */
void reverse(char s[])
{
    int i, j;
    char c;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

/* itoa:  convert n to characters in s */
void itoa(int n, char s[])
{
    int i, sign;

    if ((sign = n) < 0)  /* record sign */
        n = -n;          /* make n positive */
    i = 0;
    do {       /* generate digits in reverse order */
        s[i++] = n % 10 + '0';   /* get next digit */
    } while ((n /= 10) > 0);     /* delete it */
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}

/* ltoa:  convert n to characters in s */
void ltoa(long n, char s[])
{
    long i, sign;

    if ((sign = n) < 0)  /* record sign */
        n = -n;          /* make n positive */
    i = 0;
    do {       /* generate digits in reverse order */
        s[i++] = n % 10 + '0';   /* get next digit */
    } while ((n /= 10) > 0);     /* delete it */
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}

void copy(char* dest, char* src, int len) {
    for (int i = 0; i < len; i++) {
        if (src[i] == '\0') {
            break;
        }
        dest[i] = src[i];
    }
}

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

        clock_t sendTime = clock();
        clock_t recvTime = 0;
        // Envia os dados ao destinatário
        memset(helper, 0, 32);
        itoa(i, helper);
        memset(&buffer, 0, BUFSIZ);
        strcpy((char*)&buffer, helper);
        int lastPosOfBuffer = strlen((char*)&buffer);
        buffer[lastPosOfBuffer] = ' ';
        lastPosOfBuffer++;
        memset(helper, 0, 32);
        ltoa(sendTime, helper);
        copy(&buffer[lastPosOfBuffer], helper, sizeof(long));

        printf("Enviando %s\n", buffer);
        status = sendto(clientSocket, &buffer, 32, 0, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
        if (status < 0) {
            printf("Nao foi possivel enviar o ping ao servidor ($%d).\n", errno);
            return 1;
        }
        i++;
        usleep(250000);        // // Recebe do destinatário (com timeout de 1s)
        // status = recvfrom(clientSocket, &buffer, BUFSIZ, 0, (struct sockaddr *) &serverAddress, &serverAddressLen);
        // // Se status < 0, é erro
        // if (status < 0) {
        //     printf("Ping %d %s (%d)\n", i + 1, "Request timed out", errno);
        //     losses++;
        // } else {
        //     recvTime = clock();
        //     timeElapsed[i] = (recvTime - sendTime);
        //     if (timeElapsed[i] < min) {
        //         min = timeElapsed[i];
        //     }

        //     if (timeElapsed[i] > max) {
        //         max = timeElapsed[i];
        //     }

        //     avg += timeElapsed[i];

        //     printf("Ping %d %dµs\n", i + 1, timeElapsed[i]);
        //     sleep(1);
        // }
        // Encerra o socket
        close(clientSocket);
    }
    // double lossesPerc = ((double)losses / (double)ITERATIONS) * 100;
    // printf("Total: \n");
    // printf("Packages sent %d\n", ITERATIONS - losses);
    // printf("Packages lost: %d (%.2f%%)\n", losses, lossesPerc);
    // printf("Avg: %.2fµs, Min: %dµs, Max: %dµs\n", (double) avg / (double) ITERATIONS, min, max);

}

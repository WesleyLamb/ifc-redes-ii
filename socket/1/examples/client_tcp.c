#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    int clientSocket, status;

    struct addrinfo hints;
    struct addrinfo* result;
    struct addrinfo* rp;
    memset(&hints, 0, sizeof(hints));
    char buffer [BUFSIZ];

    hints.ai_family = AF_INET; //IPv4
    hints.ai_socktype = SOCK_DGRAM; // TCP
    hints.ai_flags = 0;
    hints.ai_protocol = 0; // Qualquer protocolo

    status = getaddrinfo("localhost", "9001", &hints, &result);
    if (status != 0) {
        fprintf(stderr, "Nao foi possivel localizar o servidor: %s\n", gai_strerror(status));
        return EXIT_FAILURE;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        clientSocket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

        if (clientSocket == -1) {
            continue;
        }

        status = connect(clientSocket, rp->ai_addr, rp->ai_addrlen);
        if (status != -1) {
            break;
        }

        close(clientSocket);
    }

    freeaddrinfo(result);

    if (rp == NULL) {
        fprintf(stderr, "Nao foi possivel estabelecer a conexao: %d.\n", errno);
        return EXIT_FAILURE;
    }

    write(clientSocket, "ping\0", sizeof(char) * 5);
    read(clientSocket, &buffer, BUFSIZ);
    printf("%s\n", buffer);

    close(clientSocket);
    return 0;
}

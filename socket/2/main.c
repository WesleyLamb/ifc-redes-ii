// Fonte: https://github.com/somnisoft/smtp-client

#include <stdio.h>
#include <stdlib.h>
#include "const.h"
#include "src/helpers.h"
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

#define MAIL_SERVER "smtp.hostinger.com"
#define MAIL_PORT "587"
#define MAIL_CONNECTION_SECURITY 0
#define MAIL_USER "wesley.lamb@castorsoft.com.br"
#define MAIL_FROM "wesley.lamb@castorsoft.com.br"
#define MAIL_FROM_NAME "Wesley R. Lamb"

int main() {
    int status;
    struct addrinfo hints, *result, *iterator;
    int clientSocket;
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    char *requestBuffer, *responseBuffer, *b64, *concat, fileReader[1024], *fileReaderHelper;
    FILE *fptr;
    char image[PACKET_SIZE], imageb64[PACKET_SIZE];
    time_t data;


    requestBuffer = malloc(PACKET_SIZE * sizeof(char));
    responseBuffer = malloc(PACKET_SIZE * sizeof(char));
    b64 = malloc(PACKET_SIZE * sizeof(char));

    hints.ai_family = AF_INET; //IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = 0;
    hints.ai_protocol = 0; // Qualquer protocolo

    // Criar client socket
    clientSocket = criarSocketTCP();
    if (clientSocket < 0) {
        fprintf(stderr, "Erro ao criar socket: %d\n", errno);
        return EXIT_FAILURE;
    }

    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

    // Solicita ao SO o IP do servidor
    status = getaddrinfo(MAIL_SERVER, MAIL_PORT, &hints, &result);
    if (status != 0) {
        fprintf(stderr, "Erro ao localizar o servidor: %s\n", gai_strerror(status));
        return EXIT_FAILURE;
    }

    for (iterator = result; iterator != NULL; iterator = iterator->ai_next) {

        status = connect(clientSocket, iterator->ai_addr, iterator->ai_addrlen);
        if (status != -1) {
            break;
        }

        close(clientSocket);
    }

    freeaddrinfo(result);
    if (iterator == NULL) {
        fprintf(stderr, "Erro ao estabelecer a conexao: %d\n", errno);
        return EXIT_FAILURE;
    }
    // Recebe a resposta do servidor (220)
    status = receber(clientSocket, (char*)responseBuffer);

    memset(requestBuffer, 0, PACKET_SIZE);
    strcpy(requestBuffer, "EHLO smtp\n");
    status = enviar(clientSocket, (char*) requestBuffer);

    memset(responseBuffer, 0, PACKET_SIZE);
    status = receber(clientSocket, (char*)responseBuffer);


    memset(requestBuffer, 0, PACKET_SIZE);
    char *usernameAndPassword;
    usernameAndPassword = malloc(128 * sizeof(char));

    memset(usernameAndPassword, 0, 128);
    usernameAndPassword[0] = '\0';
    int userlen = strlen(MAIL_USER);
    memcpy(usernameAndPassword + 1, MAIL_USER, userlen);
    usernameAndPassword[ + 1 + userlen + 1] = '\0';
    int passlen = strlen(MAIL_PASS);
    memcpy(usernameAndPassword + 1 + userlen + 1, MAIL_PASS, passlen);

    base64_encode(b64, usernameAndPassword, userlen + passlen + 2);
    concat = stpcpy(requestBuffer, "AUTH PLAIN ");
    concat = stpcpy(concat, b64);
    concat = stpcpy(concat, "\n");

    enviar(clientSocket, requestBuffer);

    memset(responseBuffer, 0, PACKET_SIZE);
    receber(clientSocket, responseBuffer);

    memset(requestBuffer, 0, PACKET_SIZE);
    sprintf(requestBuffer, "MAIL FROM:<%s>\n", MAIL_FROM);
    enviar(clientSocket, requestBuffer);

    memset(responseBuffer, 0, PACKET_SIZE);
    receber(clientSocket, responseBuffer);

    memset(requestBuffer, 0, PACKET_SIZE);
    sprintf(requestBuffer, "RCPT TO:<%s>\n", MAIL_FROM);
    enviar(clientSocket, requestBuffer);

    memset(responseBuffer, 0, PACKET_SIZE);
    receber(clientSocket, responseBuffer);

    memset(requestBuffer, 0, PACKET_SIZE);
    sprintf(requestBuffer, "DATA\n");
    enviar(clientSocket, requestBuffer);

    memset(responseBuffer, 0, PACKET_SIZE);
    receber(clientSocket, responseBuffer);

    memset(requestBuffer, 0, PACKET_SIZE);

    concat = stpcpy(requestBuffer, "Date: ");
    data = time(NULL);
    strftime(concat, PACKET_SIZE - strlen(requestBuffer), "%a, %d %b %Y %T %z\n", localtime(&data));

    enviar(clientSocket, requestBuffer);

    memset(requestBuffer, 0, PACKET_SIZE);
    sprintf(requestBuffer, "From: \"%s\" <%s>\n", MAIL_FROM_NAME, MAIL_FROM);

    enviar(clientSocket, requestBuffer);

    memset(requestBuffer, 0, PACKET_SIZE);

    sprintf(requestBuffer, "Subject: %s\n", "Teste de envio em C");

    enviar(clientSocket, requestBuffer);

    memset(requestBuffer, 0, PACKET_SIZE);
    sprintf(requestBuffer, "To: \"%s\" <%s>\n", MAIL_FROM_NAME, MAIL_FROM);

    enviar(clientSocket, requestBuffer);

    memset(requestBuffer, 0, PACKET_SIZE);
    strcpy(requestBuffer, "Teste corpo de emeio\n");
    enviar(clientSocket, requestBuffer);

    // memset(requestBuffer, 0, PACKET_SIZE);
    // strcpy(requestBuffer, "MIME-Version: 1.0\n");
    // enviar(clientSocket, requestBuffer);

    // memset(requestBuffer, 0, PACKET_SIZE);

    // concat = stpcpy(requestBuffer, "Content-Type: multipart/related; boundary=\"X-=-=-=-text boundary\"\n");
    // concat = stpcpy(concat, "Content-Type: text/plain; charset=\"utf-8\"\n");
    // concat = stpcpy(concat, "Content-Transfer-Encoding: 7bit\n");
    // concat = stpcpy(concat, "<html><body><p>Corpo de e-mail cabuloso</p><img src=\"cid:image1\"></body></html>\n");
    // concat = stpcpy(concat, "--X-=-=-=-text boundary\n");


    // concat = stpcpy(concat, "Content-Type: image/jpeg\n");
    // concat = stpcpy(concat, "Content-Transfer-Encoding: base64\n");
    // concat = stpcpy(concat, "Content-ID: image1\n");
    // concat = stpcpy(concat, "Content-Disposition: inline; filename=\"arquivo.jpg\"\n");

    // fptr = fopen("./arquivo.jpg", "r");
    // fileReaderHelper = image;

    // while (fgets((char*)&fileReader, 1024, fptr)) {
    //     fileReaderHelper = stpcpy(fileReaderHelper, fileReader);
    // }

    // base64_encode((char*)&imageb64, image, fileReaderHelper - image);

    // concat = stpcpy(concat, imageb64);
    // concat = stpcpy(concat, "\n--X-=-=-=-text boundary\n");
    // enviar(clientSocket, requestBuffer);

    memset(requestBuffer, 0, PACKET_SIZE);
    concat = stpcpy(requestBuffer, "\r\n.\r\n");

    enviar(clientSocket, requestBuffer);

    memset(responseBuffer, 0, PACKET_SIZE);
    receber(clientSocket, responseBuffer);

    memset(requestBuffer, 0, PACKET_SIZE);
    sprintf(requestBuffer, "QUIT\n");

    enviar(clientSocket, requestBuffer);
    close(clientSocket);

    return EXIT_SUCCESS;
}
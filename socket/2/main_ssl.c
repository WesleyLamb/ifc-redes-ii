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
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

#define MAIL_SERVER "smtp.hostinger.com"
#define MAIL_PORT "465"
#define MAIL_CONNECTION_SECURITY 0
#define MAIL_USER "wesley.lamb@castorsoft.com.br"
#define MAIL_FROM "wesley.lamb@castorsoft.com.br"
#define MAIL_FROM_NAME "Wesley R. Lamb"

int main() {
    int status, err;
    struct addrinfo hints, *result, *iterator;
    int clientSocket;
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    char *requestBuffer, *responseBuffer, *b64, *concat;

    SSL_CTX *tls_ctx;
    SSL *tlsSocket;
    BIO *tls_bio;

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

    /** TLS */
    SSL_library_init();
    SSL_load_error_strings();
    ERR_load_BIO_strings();
    OpenSSL_add_all_algorithms();

    if ((tls_ctx = SSL_CTX_new(SSLv23_client_method())) == NULL)
    {
        fprintf(stderr, "Erro ao realizar o TLS handshake[1]: %d\n", errno);
        return EXIT_FAILURE;
    }

    SSL_CTX_set_options(tls_ctx,
                        SSL_OP_NO_SSLv2 |
                        SSL_OP_NO_SSLv3 |
                        SSL_OP_NO_TLSv1);

    SSL_CTX_set_mode(tls_ctx, SSL_MODE_AUTO_RETRY);
    X509_STORE_set_default_paths(SSL_CTX_get_cert_store(tls_ctx));

    if (err = ERR_peek_error() != 0)
    {
        fprintf(stderr, "Erro ao realizar o TLS handshake[2].");
        SSL_CTX_free(tls_ctx);
        return EXIT_FAILURE;
    }

    if ((tlsSocket = SSL_new(tls_ctx)) == NULL)
    {
        fprintf(stderr, "Erro ao realizar o TLS handshake[3].");
        SSL_CTX_free(tls_ctx);
        return EXIT_FAILURE;
    }

    if ((tls_bio = BIO_new_socket(clientSocket, 0)) == NULL)
    {
        fprintf(stderr, "Erro ao realizar o TLS handshake[4].");
        SSL_CTX_free(tls_ctx);
        SSL_free(tlsSocket);
        return EXIT_FAILURE;
    }

    SSL_set_bio(tlsSocket, tls_bio, tls_bio);
    SSL_set_connect_state(tlsSocket);
    if (SSL_connect(tlsSocket) != 1)
    {
        fprintf(stderr, "Erro ao realizar o TLS handshake[5].");
        SSL_CTX_free(tls_ctx);
        SSL_free(tlsSocket);
        return EXIT_FAILURE;
    }

    if (SSL_do_handshake(tlsSocket) != 1)
    {
        fprintf(stderr, "Erro ao realizar o TLS handshake[6].");
        SSL_CTX_free(tls_ctx);
        SSL_free(tlsSocket);
        return EXIT_FAILURE;
    }
    /** TLS */

    freeaddrinfo(result);
    if (iterator == NULL) {
        fprintf(stderr, "Erro ao estabelecer a conexao: %d\n", errno);
        return EXIT_FAILURE;
    }
    // Recebe a resposta do servidor (220)
    status = receberTLS(tlsSocket, (char*)responseBuffer);

    memset(requestBuffer, 0, PACKET_SIZE);
    strcpy(requestBuffer, "EHLO smtp\n");
    status = enviarTLS(tlsSocket, (char*) requestBuffer);

    memset(responseBuffer, 0, PACKET_SIZE);
    status = receberTLS(tlsSocket, (char*)responseBuffer);

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

    enviarTLS(tlsSocket, requestBuffer);

    memset(responseBuffer, 0, PACKET_SIZE);
    receberTLS(tlsSocket, responseBuffer);

    memset(requestBuffer, 0, PACKET_SIZE);
    sprintf(requestBuffer, "MAIL FROM:<%s>\n", MAIL_FROM);
    enviarTLS(tlsSocket, requestBuffer);

    memset(responseBuffer, 0, PACKET_SIZE);
    receberTLS(tlsSocket, responseBuffer);

    memset(requestBuffer, 0, PACKET_SIZE);
    sprintf(requestBuffer, "RCPT TO:<%s>\n", MAIL_FROM);
    enviarTLS(tlsSocket, requestBuffer);

    memset(responseBuffer, 0, PACKET_SIZE);
    receberTLS(tlsSocket, responseBuffer);

    memset(requestBuffer, 0, PACKET_SIZE);
    sprintf(requestBuffer, "DATA\n");
    enviarTLS(tlsSocket, requestBuffer);

    memset(responseBuffer, 0, PACKET_SIZE);
    receberTLS(tlsSocket, responseBuffer);

    memset(requestBuffer, 0, PACKET_SIZE);

    concat = stpcpy(requestBuffer, "Date: ");
    time_t *data;
    *data = time(NULL);
    strftime(concat, PACKET_SIZE - strlen(requestBuffer), "%a, %d %b %Y %T %z\n", localtime(data));

    enviarTLS(tlsSocket, requestBuffer);

    memset(requestBuffer, 0, PACKET_SIZE);
    sprintf(requestBuffer, "From: \"%s\" <%s>\n", MAIL_FROM_NAME, MAIL_FROM);

    enviarTLS(tlsSocket, requestBuffer);

    memset(requestBuffer, 0, PACKET_SIZE);

    sprintf(requestBuffer, "Subject: %s\n", "Teste de envio em C");

    enviarTLS(tlsSocket, requestBuffer);

    memset(requestBuffer, 0, PACKET_SIZE);
    sprintf(requestBuffer, "To: \"%s\" <%s>\n", MAIL_FROM_NAME, MAIL_FROM);

    enviarTLS(tlsSocket, requestBuffer);

    memset(requestBuffer, 0, PACKET_SIZE);
    sprintf(requestBuffer, "Teste corpo de emeio\n");
    enviarTLS(tlsSocket, requestBuffer);

    memset(requestBuffer, 0, PACKET_SIZE);
    sprintf(requestBuffer, "\r\n.\r\n");

    enviarTLS(tlsSocket, requestBuffer);

    memset(responseBuffer, 0, PACKET_SIZE);
    receberTLS(tlsSocket, responseBuffer);

    memset(requestBuffer, 0, PACKET_SIZE);
    sprintf(requestBuffer, "QUIT\n");

    enviarTLS(tlsSocket, requestBuffer);
    close(clientSocket);
    SSL_free(tlsSocket);
    SSL_CTX_free(tls_ctx);

    return EXIT_SUCCESS;
}
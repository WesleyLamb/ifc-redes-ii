#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "helpers.h"
#include <stdint.h>
#include <stdlib.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};
static char *decoding_table = NULL;
static int mod_table[] = {0, 2, 1};

int criarSocketTCP()
{
    return socket(AF_INET, SOCK_STREAM, 0);
}

int receber(int socket, char* resposta) {

    char* buffer;
    buffer = malloc(BUFSIZ * sizeof(char));
    char *concat = resposta;

    int status = 0;
    memset(buffer, 0, BUFSIZ);
    while (status = recv(socket, buffer, BUFSIZ, 0) > 0) {
        concat = stpcpy(concat, buffer);
        if (buffer[strlen(buffer) == '\0']) {
            break;
        }
        memset(buffer, 0, BUFSIZ);
        printf("Ultimo caractere: %d\n", (int) buffer[strlen(buffer)]);
    };
    if (status < 0) {
        fprintf(stderr, "Erro ao receber os dados: %d\n", status);
        return status;
    }
    fprintf(stdout, "Resposta: %s\n", resposta);
}

void base64_encode(char *dest, char* src, int srclen)
{
    int destlen;
    EVP_ENCODE_CTX *ectx = EVP_ENCODE_CTX_new();
    EVP_EncodeInit(ectx);
    EVP_EncodeBlock( dest, src, srclen);
    EVP_EncodeFinal(ectx, dest, &destlen);
    EVP_ENCODE_CTX_free(ectx);
}

int enviar(int socket, char *requisicao)
{
    int status = 0;

    status = send(socket, requisicao, strlen(requisicao), 0);
    fprintf(stdout, "Envio: %s\n", requisicao);
    if (status < 0) {
        fprintf(stderr, "Erro ao enviar os dados: %d\n", errno);
        return status;
    }

    return status;
}

void copy(char *dest, char *src, int len)
{
    for (int i = 0; i < len; i++)
    {
        if (src[i] == '\0')
        {
            break;
        }
        dest[i] = src[i];
    }
}
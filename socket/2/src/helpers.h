#include <openssl/ssl.h>

int criarSocketTCP();
int enviar(int socket, char* requisicao);
int receber(int socket, char* resposta);
int enviarTLS(SSL *socket, char* requisicao);
int receberTLS(SSL *socket, char* resposta);
void base64_encode(char *dest, char* src, int srclen);
void copy(char *dest, char *src, int len);
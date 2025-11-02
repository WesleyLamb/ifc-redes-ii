int criarSocketTCP();
int enviar(int socket, char* requisicao);
int receber(int socket, char* resposta);
void base64_encode(char *dest, char* src, int srclen);
void copy(char *dest, char *src, int len);
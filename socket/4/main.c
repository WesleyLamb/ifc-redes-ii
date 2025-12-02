/*
 * Copyright (c) 2023 Jan Wilmans, MIT License
 */

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })


// you can choose to send more or less dummy payload data
#define ICMP_PAYLOAD_LENGTH (26 /*- sizeof(struct icmphdr)*/)
struct ping_pkt
{
    struct icmphdr hdr;
    char payload[ICMP_PAYLOAD_LENGTH];
};

// the reason for this method is that bzero() is not standard C11 and I really
// do not like the fact that messing up the order of arguments to 'memset' to zero out memory
// can cause serious bugs. I would like memset_explicit even more, but C23 isn't available to me at this like
void zero_inititialize(void *data, int size)
{
    memset(data, 0, size);
}

// this will convert any binary data into a readable ascii form
// unprintable ascii characters are coverted to dots
const char *to_hex_string(const void *object, int size)
{

    const char *data = (const char *)object;
    static char buffer[1024];
    char *write_pointer = &buffer[0];
    // for (int i = 0; i < size; ++i)
    // {
    //     int bytes_written = sprintf(write_pointer, "%02X ", (uint8_t)data[i]);
    //     write_pointer += bytes_written;
    // }

    // *write_pointer = ';';
    // ++write_pointer;

    for (int i = 28; i < size; ++i)
    {
        char c = data[i];
        if (c < 32)
        {
            *write_pointer = '.';
            ++write_pointer;
            continue;
        }
        *write_pointer = c;
        ++write_pointer;
    }
    *write_pointer = '\0';
    return &buffer[0];
}

// lookup the dns-name of associated with an ipaddress
bool reverse_dns_lookup(const char *ipaddress, char *name_out, int size)
{
    int status = 0;
    struct sockaddr_in addr;
    zero_inititialize(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ipaddress);

    char buffer[NI_MAXHOST];
    if (status = getnameinfo((struct sockaddr *)&addr, sizeof(struct sockaddr_in), buffer, sizeof(buffer), NULL, 0, NI_NAMEREQD))
    {
        // printf("Não foi possível recuperar o DNS Reverso do HOST informado: %s\n", (gai_strerror(status)));
        return false;
    }

    strncpy(name_out, &buffer[0], size);
    return true;
}

unsigned short calculate_checksum(const struct ping_pkt *packet)
{
    const unsigned short *view = (const unsigned short *)packet;
    size_t size = sizeof(struct ping_pkt);

    unsigned int sum = 0;
    for (; size > 1; size -= 2)
    {
        sum += *view++;
    }
    if (size == 1)
    {
        sum += *(unsigned char *)view;
    }
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return ~sum;
}

char *dns_lookup_and_store_address(char *address, struct sockaddr_in *sock_addr, char *out_address)
{
    static char buffer[1024];

    zero_inititialize(&buffer, sizeof(buffer));
    uint16_t port = 0;
    struct hostent *host_entity = gethostbyname(address);
    if (host_entity == NULL)
    {
        printf("Erro ao realizar o DNS lookup para '%s': %d\n", address, errno);
        return NULL;
    }
    char *name = inet_ntoa(*(struct in_addr *)host_entity->h_addr_list[0]);
    strncpy(buffer, name, sizeof(buffer));
    if (sock_addr != NULL)
    {
        sock_addr->sin_family = host_entity->h_addrtype;
        sock_addr->sin_port = htons(port);
        sock_addr->sin_addr.s_addr = *(long *)host_entity->h_addr_list[0];
    }
    memcpy(out_address, &buffer, 1024);
}

int set_ttl(int socket_fd, int ttl)
{
    return setsockopt(socket_fd, SOL_IP, IP_TTL, &ttl, sizeof(ttl));
}

int set_receive_timeout(int socket_fd, int timeout_ms)
{
    int seconds = timeout_ms / 1000;
    int useconds = (timeout_ms - (seconds * 1000)) * 1000;
    struct timeval tv_out;
    zero_inititialize(&tv_out, sizeof(tv_out));
    tv_out.tv_sec = seconds;
    tv_out.tv_usec = useconds;
    return setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &tv_out, sizeof(tv_out));
}

void initialize_icmp_packet(struct ping_pkt *icmp_packet)
{
    zero_inititialize(icmp_packet, sizeof(*icmp_packet));
    icmp_packet->hdr.type = ICMP_ECHO;
    icmp_packet->hdr.un.echo.id = getpid();
    icmp_packet->hdr.un.echo.sequence = 0;

    // the payload is arbitrary, it can be any data but it is good practice to
    // send something recognizable like a string.
    // its important to make sure to calculate the checksum _after_ filling the payload.
    for (size_t i = 0; i < ICMP_PAYLOAD_LENGTH; ++i)
    {
        icmp_packet->payload[i] = (char)('a' + i);
    }
    icmp_packet->hdr.checksum = calculate_checksum(icmp_packet);
}

int icmp_send(int socket_fd, struct sockaddr_in *address, const void *data, size_t size)
{
    return sendto(socket_fd, data, size, 0, (struct sockaddr *)address, sizeof(*address));
}

int icmp_receive(int socket_fd, char *buffer, int buffer_size)
{
    return recvfrom(socket_fd, buffer, buffer_size, 0, NULL, NULL);
}

// when sending icmp ping packets using raw sockets verifing the echo.id is required
// otherwise you maybe looking at unrelated ping replys
bool verificar_icmp(const struct ping_pkt *sent, const struct ping_pkt *received, int expected_id)
{
    if (received->hdr.un.echo.id != expected_id)
    {
        printf("A reposta capturada não corresponde a resposta esperada. Id esperado %d, recebido. %d \n", expected_id, received->hdr.un.echo.id);
        return false;
    }
    if (memcmp(&sent->payload[0], &received->payload[0], ICMP_PAYLOAD_LENGTH) != 0)
    {
        printf("A mensagem de retorno não é um echo da mensagem enviada.\n");
        return false;
    }
    return true;
}

double get_difference_ms(const struct timespec *t1, const struct timespec *t2)
{
    double ns = (t2->tv_nsec - t1->tv_nsec) / 1000000.0;
    double ms = (t2->tv_sec - t1->tv_sec) * 1000.0;
    return ms + ns;
}

int icmp_ping(char *address, int timeout_ms, double *duration_ms, char *raw_data, int current_ttl)
{
    struct sockaddr_in sock_addr;
    zero_inititialize(&sock_addr, sizeof(sock_addr));
    char name[1024];
    dns_lookup_and_store_address(address, &sock_addr, (char*) &name);
    if (name == NULL)
    {
        return -1;
    }
    int socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (socket_fd < 0)
    {
        printf("Não foi possível criar o socket '%s': %d\n", address, errno);
        return -1;
    }

    if (set_ttl(socket_fd, current_ttl) != 0)
    {
        printf("Não foi possível definir o TTL de %d.\n", current_ttl);
        close(socket_fd);
        return -1;
    }
    if (set_receive_timeout(socket_fd, timeout_ms) != 0)
    {
        printf("Não foi possível definir o timeout de '%d'.\n", timeout_ms);
        close(socket_fd);
        return -1;
    }

    const uint16_t my_icmp_id = getpid();
    const int ip_header_length = 20;
    const int raw_icmp_response_length = ip_header_length + sizeof(struct ping_pkt);

    struct ping_pkt packet;
    initialize_icmp_packet(&packet);

    struct timespec start_timestamp;
    struct timespec stop_timestamp;
    clock_gettime(CLOCK_MONOTONIC, &start_timestamp);

    icmp_send(socket_fd, &sock_addr, &packet, sizeof(packet));

    bool done = false, received = false;
    int data_received = 0;
    while (!done)
    {
        char buffer[1024];
        data_received = icmp_receive(socket_fd, &buffer[0], raw_icmp_response_length);

        if (*duration_ms > timeout_ms)
        {
            done = true;
        }

        if (data_received > 0)
        {
            memcpy(raw_data, (char*) &buffer, 1024);
            received = true;
            // printf("Resposta: %s\n", to_hex_string(&buffer, data_received));
        }
        clock_gettime(CLOCK_MONOTONIC, &stop_timestamp);
        *duration_ms = get_difference_ms(&start_timestamp, &stop_timestamp);
    }
    close(socket_fd);
    if (!received) {
        return -1;
    }
    return 0;
}

void intToIp(char* dest, int* src)
{
    struct in_addr ip_addr;
    ip_addr.s_addr = *src;
    int iplen = strlen(inet_ntoa(ip_addr));
    memcpy(dest, inet_ntoa(ip_addr), iplen);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("usage: %s <host>\n", argv[0]);
        return -1;
    }

    // DNS Lookup pra pegar um domínio e converter para IP
    char *host = argv[1];
    char address[1024];
    dns_lookup_and_store_address(host, NULL, (char*) &address);

    // DNS Reverso pra pegar o nome "bruto" do servidor
    char name[1024];
    zero_inititialize(&name[0], sizeof(name));
    if (!reverse_dns_lookup(address, name, sizeof(name)))
    {
        // return -1;
    }

    printf("Disparando PING para %s. (%s)\n", address, name);

    int status_code = 0;
    const int timeout_ms = 500;
    char raw_data[1024];
    int current_ttl = 1;

    zero_inititialize(raw_data, 1024);
    int responseType = -1;
    int responseCode = -1;
    char responseAddress[128];
    char responseAddressName[128];
    int ip = 0;

    while (responseType != ICMP_ECHOREPLY || strcmp(responseAddress, host) == 0) {
        for (int i = 0; i < 3; i++) {
            zero_inititialize(raw_data, 1024);
            double duration = 0.0;
            int result = icmp_ping(host, timeout_ms, &duration, (char*)raw_data, current_ttl);

            if (result == 0) {
                responseType = raw_data[20];
                responseCode = raw_data[21];

                zero_inititialize((char*)&responseAddress, 128);
                zero_inititialize((char*)&responseAddressName, 128);
                memcpy(&ip, &raw_data[12], 4);
                intToIp(&responseAddress[0], &ip);

                // memcpy((char*)&responseAddress, , 4);

                reverse_dns_lookup(responseAddress, (char*)&responseAddressName, 128);

                if ((responseType == ICMP_ECHOREPLY) || (responseType == ICMP_TIME_EXCEEDED)) {
                    reverse_dns_lookup(responseAddress, (char*)&responseAddressName, 128);

                    printf("Resposta de %s (%s) - type: %d, code: %d, tempo decorrido: %.2fms (TTL: %d).\n", responseAddress, responseAddressName, responseType, responseCode, duration - 500, current_ttl);
                } else {
                    // Ou chegou ao destino e o mesmo não respondeu ou o destino é inalcançavel
                    printf("Resposta de %s - type: %d, code: %d, tempo decorrido: %.2fms (TTL: %d).\n", responseAddress,
                        responseType, responseCode, duration - 500, current_ttl);
                }
            } else {
                // Destino nem respondeu
                printf("Sem resposta de %s\n", host);
            }

        }
        current_ttl++;
    }

    return status_code;
}
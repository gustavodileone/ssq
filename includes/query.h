#define QUERY

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>

#include "a2s/types.h"

typedef struct PACKET {
    int32_t header;

    uint32_t id;
    uint8_t total;
    uint8_t number;
    uint16_t size;

    // TODO: compression

    char* payload;
    size_t payload_len;
} PACKET;

int query_read(char** read, void* buff, size_t n) {
    memcpy(buff, *read, n);
    *read = *read + n;

    return n;
}

int query_read_string(char** read, char** buff) {
    int n = strlen(*read) + 1;
    *buff = malloc(n);
    memcpy(*buff, *read, n);

    *read = *read + n;

    return n;
}

PACKET* query_packet_deserialize(char* response, size_t response_len) {
    PACKET* packet = malloc(sizeof(PACKET));

    int read = query_read(&response, &packet->header, sizeof(packet->header));
    read += query_read(&response, &packet->id, sizeof(packet->id));
    read += query_read(&response, &packet->total, sizeof(packet->total));
    read += query_read(&response, &packet->number, sizeof(packet->number));
    read += query_read(&response, &packet->size, sizeof(packet->size));

    packet->payload_len = response_len - read;
    packet->payload = malloc(packet->payload_len);
    memcpy(packet->payload, response, packet->payload_len);

    return packet;
}

int query_connect(char* host, char* port) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd == -1) {
        perror("query_connect()");
        exit(EXIT_FAILURE);
    }
    
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_CANONNAME;

    struct addrinfo* info;

    int result;
    if((result = getaddrinfo(host, port, &hints, &info)) != 0) {
        printf("%s\n", gai_strerror(result));
        perror("getaddrinfo()");
        exit(EXIT_FAILURE);
    }

    struct addrinfo* next = info;

    while(next != NULL) {
        if(connect(sockfd, next->ai_addr, next->ai_addrlen) == 0) break;
        next = next->ai_next;
    }

    freeaddrinfo(info);

    if(next == NULL) {
        fprintf(stderr, "ERR: couldn't connect to {%s}[%s]", host, port);
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

void query_wrapper_send(int sockfd, char* request, size_t request_len) {
    int sent = 0;
    int bytes;

    while(sent < request_len) {
        bytes = send(sockfd, request + sent, request_len - sent, 0);
        if(bytes == -1) {
            perror("query_request():");
            exit(EXIT_FAILURE);
        }

        sent += bytes;
    }
}

char* query_wrapper_recv(int sockfd, size_t* response_len) {
    char* response = malloc(*response_len);
    if(response == NULL) {
        fprintf(stderr, "query_request(): malloc returned NULL");
        exit(EXIT_FAILURE);
    }

    *response_len = recv(sockfd, response, *response_len, 0);
    if(*response_len == -1) {
        perror("query_request():");
        exit(EXIT_FAILURE);
    }

    return response;
}

int query_is_split(char* response) {
    int32_t packet_type;
    memcpy(&packet_type, response, sizeof(packet_type));

    return packet_type == PACKET_SPLIT;
}

char* query_squash_packets(PACKET** packets, size_t packets_sz, size_t response_len) {
    char* response = malloc(response_len);
    if(response == NULL) {
        fprintf(stderr, "query_squash_packets(): malloc() returned NULL.\n");
        exit(EXIT_FAILURE);
    }
    
    int offset = 0;
    for(int i = 0; i < packets_sz; i++) {
        PACKET* packet = packets[i];

        memcpy(response + offset, packet->payload, packet->payload_len);
        offset += packet->payload_len;
    }

    return response;
}

char* query_request(int sockfd, char* request, size_t request_len, size_t* response_len) {
    query_wrapper_send(sockfd, request, request_len);

    char* response = query_wrapper_recv(sockfd, response_len);
    if(!query_is_split(response)) return response;

    PACKET* first_packet = query_packet_deserialize(response, *response_len);
    free(response);

    PACKET** packets = malloc(sizeof(PACKET) * first_packet->total);
    if(packets == NULL) {
        fprintf(stderr, "Err: couldn't allocate memory for packets.");
        exit(EXIT_FAILURE);
    }

    packets[0] = first_packet;
    int packet_old = first_packet->number;

    int total_response = *response_len;

    for(int i = 1; i < first_packet->total; i++) {
        size_t packet_response_len = PACKET_SIZE;

        response = query_wrapper_recv(sockfd, &packet_response_len);
        total_response += packet_response_len;

        PACKET* packet = query_packet_deserialize(response, packet_response_len);
        free(response);

        if((packet_old + 1) != packet->number) {
            fprintf(stderr, "PACKET OUT OF ORDER!\n");
            exit(EXIT_FAILURE);
        }

        packets[i] = packet;
        packet_old = packet->number;
    }

    response = query_squash_packets(packets, first_packet->total, total_response);

    return response;
}

int query_is_challenge(char* response, size_t response_len) {
    if(response_len < A2S_CHALLENGE_LENGTH) return -1;
    return response[A2S_CHALLENGE_LENGTH] == A2S_CHALLENGE_CHAR;
}

char* query_request_cycle(char* host, char* port, char* request, size_t request_len, size_t* response_len, int challenge_start, int challenge_offset) {
    int sockfd = query_connect(host, port);

    *response_len = PACKET_SIZE;
    char* response = query_request(sockfd, request, request_len - challenge_offset, response_len);
    if(query_is_challenge(response, *response_len)) {
        memcpy(&request[challenge_start], &response[A2S_CHALLENGE_START], A2S_CHALLENGE_LENGTH);
        free(response);
        
        *response_len = PACKET_SIZE;
        response = query_request(sockfd, request, request_len, response_len);
    }

    close(sockfd);

    return response;
}
#include <stdint.h>

#ifndef ENUMS
    #include "enums.h"
#endif

#ifndef QUERY
    #include "../query.h"
#endif

#ifndef TYPES
    #include "types.h"
#endif

#define A2S_INFO_HEADER 0x54
#define A2S_INFO_REQUEST 29
#define A2S_INFO_CHALLENGE_START (A2S_INFO_REQUEST - A2S_CHALLENGE_LENGTH)

#define A2S_EDF_PORT 0x80
#define A2S_EDF_STEAM_ID 0x10
#define A2S_EDF_SOURCE_TV 0x40
#define A2S_EDF_KEYWORDS 0x20
#define A2S_EDF_GAME_ID 0x01

typedef struct A2S_INFO {
    uint8_t header;
    uint8_t protocol;
    char* name;
    char* map;
    char* folder;
    char* game;
    uint16_t id;
    uint8_t players;
    uint8_t max_players;
    uint8_t bots;
    uint8_t server_type;
    uint8_t environment;
    uint8_t visibility;
    uint8_t vac;
    char* version;

    uint8_t edf;
    uint16_t port;
    uint64_t steam_id;
    uint16_t sourcetv_port;
    char* sourcetv_name;
    char* keywords;
    uint64_t game_id;
} A2S_INFO;

A2S_INFO* a2s_info_deserialize(char* response, size_t response_len) {
    response = response + PACKET_HEADER_LENGTH;

    A2S_INFO* info = malloc(sizeof(A2S_INFO));
    if(info == NULL) {
        fprintf(stderr, "Failed to allocate memory.");
        return NULL;
    }

    query_read(&response, &info->header, sizeof(info->header));
    query_read(&response, &info->protocol, sizeof(info->protocol));
    query_read_string(&response, &info->name);
    query_read_string(&response, &info->map);
    query_read_string(&response, &info->folder);
    query_read_string(&response, &info->game);
    query_read(&response, &info->id, sizeof(info->id));
    query_read(&response, &info->players, sizeof(info->players));
    query_read(&response, &info->max_players, sizeof(info->max_players));
    query_read(&response, &info->bots, sizeof(info->bots));
    query_read(&response, &info->server_type, sizeof(info->server_type));
    query_read(&response, &info->environment, sizeof(info->environment));
    query_read(&response, &info->visibility, sizeof(info->visibility));
    query_read(&response, &info->vac, sizeof(info->vac));
    query_read_string(&response, &info->version);

    query_read(&response, &info->edf, sizeof(info->edf));

    if(info->edf & A2S_EDF_PORT) {
        query_read(&response, &info->port, sizeof(info->port));
    }

    if(info->edf & A2S_EDF_STEAM_ID) {
        query_read(&response, &info->steam_id, sizeof(info->steam_id));
    }

    if(info->edf & A2S_EDF_SOURCE_TV) {
        query_read(&response, &info->sourcetv_port, sizeof(info->sourcetv_port));
        query_read_string(&response, &info->sourcetv_name);
    }

    if(info->edf & A2S_EDF_KEYWORDS) {
        query_read_string(&response, &info->keywords);
    }

    if(info->edf & A2S_EDF_GAME_ID) {
        query_read(&response, &info->game_id, sizeof(info->game_id));
    }

    return info;
}

A2S_INFO* a2s_info(char* host, char* port) {
    char request[A2S_INFO_REQUEST] = {
        0xFF, 0xFF, 0xFF, 0xFF, A2S_INFO_HEADER,
        'S', 'o', 'u', 'r', 'c', 'e', ' ',
        'E', 'n', 'g', 'i', 'n', 'e', ' ',
        'Q', 'u', 'e', 'r', 'y', '\0'
    };

    size_t response_len;
    char* response = query_request_cycle(host, port, request, sizeof(request), &response_len, A2S_INFO_CHALLENGE_START, 4);

    A2S_INFO* info = a2s_info_deserialize(response, response_len);
    free(response);

    return info;
}

void a2s_info_print(A2S_INFO* info) {
    printf("header: %c\n", info->header);
    printf("protocol: %d\n", info->protocol);
    printf("name: %s\n", info->name);
    printf("map: %s\n", info->map);
    printf("folder: %s\n", info->folder);
    printf("game: %s\n", info->game);
    printf("id: %d\n", info->id);
    printf("players: %d\n", info->players);
    printf("max_players: %d\n", info->max_players);
    printf("bots: %d\n", info->bots);
    printf("server_type: %s\n", a2s_get_server_type(info->server_type));
    printf("environment: %s\n", a2s_get_environment(info->environment));
    printf("visibility: %d\n", info->visibility);
    printf("vac: %d\n", info->vac);
    printf("version: %s\n", info->version);

    printf("edf (Extra Data Flag): %x\n", info->edf);

    if(info->edf & A2S_EDF_PORT) {
        printf("port: %d\n", info->port);
    }

    if(info->edf & A2S_EDF_STEAM_ID) {
        printf("steam_id: %ld\n", info->steam_id);
    }

    if(info->edf & A2S_EDF_SOURCE_TV) {
        printf("sourcetv_port: %d\n", info->sourcetv_port);
        printf("sourcetv_name: %s\n", info->sourcetv_name);
    }

    if(info->edf & A2S_EDF_KEYWORDS) {
        printf("keywords: %s\n", info->keywords);
    }

    if(info->edf & A2S_EDF_GAME_ID) {
        printf("game_id: %ld\n", info->game_id);
    }
}

void a2s_info_free(A2S_INFO* info) {
    free(info);
}
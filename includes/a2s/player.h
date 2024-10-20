#include <stdint.h>

#ifndef QUERY
    #include "../query.h"
#endif

#ifndef TYPES
    #include "types.h"
#endif

#define A2S_PLAYER_HEADER 0x55
#define A2S_PLAYER_REQUEST_LENGTH 9
#define A2S_PLAYER_CHALLENGE_START (A2S_PLAYER_REQUEST_LENGTH - A2S_CHALLENGE_LENGTH)

typedef struct A2S_PLAYER {
    int8_t index;
    char* name;
    int32_t score;
    float duration;
} A2S_PLAYER;

typedef struct A2S_PLAYER_LIST {
    uint8_t header;
    uint8_t players;
    A2S_PLAYER** players_list;
} A2S_PLAYER_LIST;

A2S_PLAYER_LIST* a2s_player_deserialize(char* response, size_t response_len) {
    A2S_PLAYER_LIST* players = malloc(sizeof(A2S_PLAYER_LIST));
    
    response = response + PACKET_HEADER_LENGTH;

    query_read(&response, &players->header, sizeof(players->header));
    query_read(&response, &players->players, sizeof(players->players));

    A2S_PLAYER** list = malloc(sizeof(A2S_PLAYER) * players->players);
    for(size_t i = 0; i < players->players; i++) {
        A2S_PLAYER* player = malloc(sizeof(A2S_PLAYER));

        query_read(&response, &player->index, sizeof(player->index));
        query_read_string(&response, &player->name);
        query_read(&response, &player->score, sizeof(player->score));
        query_read(&response, &player->duration, sizeof(player->duration));

        list[i] = player;
    }

    players->players_list = list;

    return players;
}

A2S_PLAYER_LIST* a2s_player(char* host, char* port) {
    char request[A2S_PLAYER_REQUEST_LENGTH] = {
        0xFF, 0xFF, 0xFF, 0xFF, A2S_PLAYER_HEADER,
        0xFF, 0xFF, 0xFF, 0xFF
    };

    size_t response_len;
    char* response = query_request_cycle(host, port, request, sizeof(request), &response_len, A2S_PLAYER_CHALLENGE_START, 0);

    A2S_PLAYER_LIST* players = a2s_player_deserialize(response, response_len);
    free(response);

    return players;
}

void a2s_players_print(A2S_PLAYER_LIST* players) {
    printf("header: %c\n", players->header);
    printf("players: %d\n", players->players);

    for(size_t i = 0; i < players->players; i++) {
        A2S_PLAYER* player = players->players_list[i];

        printf("\n");
        printf("index: %d\n", player->index);
        printf("name: %s\n", player->name);
        printf("score: %d\n", player->score);
        printf("duration: %f\n", player->duration);
    }
}

void a2s_players_free(A2S_PLAYER_LIST* players) {
    for(size_t i = 0; i < players->players; i++) {
        free(players->players_list[i]);
    }

    free(players);
}
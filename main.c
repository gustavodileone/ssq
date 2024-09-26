#include <stdio.h>
#include <stdlib.h>

#include "includes/a2s/player.h"
#include "includes/a2s/info.h"
#include "includes/a2s/rules.h"

#define DEFAULT_PORT "27015"

int main(int argc, char** argv) {
    if(argc < 2) {
        printf("%s: <addr> [port]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char* addr = argv[1];
    char* port = argc > 2 ? argv[2] : DEFAULT_PORT;

    A2S_INFO* info = a2s_info(addr, port);
    a2s_info_print(info);
    a2s_info_free(info);

    printf("\n##########\n\n");

    A2S_PLAYER_LIST* players = a2s_player(addr, port);
    a2s_players_print(players);
    a2s_players_free(players);

    printf("\n##########\n\n");

    A2S_RULES_LIST* rules = a2s_rules(addr, port);
    a2s_rules_print(rules);
    a2s_rules_free(rules);

    return EXIT_SUCCESS;
}
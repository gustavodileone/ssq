#include <stdint.h>

#ifndef QUERY
    #include "../query.h"
#endif

#ifndef TYPES
    #include "types.h"
#endif

#define A2S_RULES_HEADER 0x56
#define A2S_RULES_REQUEST_LENGTH 9
#define A2S_RULES_CHALLENGE_START (A2S_RULES_REQUEST_LENGTH - A2S_CHALLENGE_LENGTH)

typedef struct A2S_RULE {
    char* name;
    char* value;
} A2S_RULE;

typedef struct A2S_RULES_LIST {
    uint8_t header;
    uint16_t rules;

    A2S_RULE** rules_list;
} A2S_RULES_LIST;

A2S_RULES_LIST* a2s_rules_deserialize(char* response, size_t response_len) {
    A2S_RULES_LIST* rules = (A2S_RULES_LIST*) malloc(sizeof(A2S_RULES_LIST));

    response = response + PACKET_HEADER_LENGTH;

    query_read(&response, &rules->header, sizeof(rules->header));
    query_read(&response, &rules->rules, sizeof(rules->rules));

    rules->rules_list = (A2S_RULE**) malloc(sizeof(A2S_RULE) * rules->rules);

    for(int i = 0; i < rules->rules; i++) {
        A2S_RULE* rule = (A2S_RULE*) malloc(sizeof(A2S_RULE));

        query_read_string(&response, &rule->name);
        query_read_string(&response, &rule->value);

        rules->rules_list[i] = rule;
    }

    return rules;
}

A2S_RULES_LIST* a2s_rules(char* host, char* port) {
    char request[A2S_RULES_REQUEST_LENGTH] = {
        0xFF, 0xFF, 0xFF, 0xFF, A2S_RULES_HEADER,
        0xFF, 0xFF, 0xFF, 0xFF
    };

    size_t response_len;
    char* response = query_request_cycle(host, port, request, sizeof(request), &response_len, A2S_RULES_CHALLENGE_START);

    A2S_RULES_LIST* rules = a2s_rules_deserialize(response, response_len);
    free(response);

    return rules;
}

void a2s_rules_print(A2S_RULES_LIST* rules) {
    printf("header: %c\n", rules->header);
    printf("rules: %d\n", rules->rules);

    for(int i = 0; i < rules->rules; i++) {
        A2S_RULE* rule = rules->rules_list[i];

        printf("\n");
        printf("name: %s\n", rule->name);
        printf("value: %s\n", rule->value);
    }
}

void a2s_rules_free(A2S_RULES_LIST* rules) {
    for(size_t i = 0; i < rules->rules; i++) {
        free(rules->rules_list[i]);
    }

    free(rules);
}
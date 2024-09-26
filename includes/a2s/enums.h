#define ENUMS

#include <stdint.h>

typedef enum A2S_ENVIRONMENT {
    A2S_ENVIRONMENT_LINUX = 'l',
    A2S_ENVIRONMENT_WINDOWS = 'w',
    A2S_ENVIRONMENT_MAC = 'm',
    A2S_ENVIRONMENT_MAC_ALT = 'o',
} A2S_ENVIRONMENT;

typedef enum A2S_SERVER {
    A2S_SERVER_DEDICATED = 'd',
    A2S_SERVER_NON_DEDICATED = 'l',
    A2S_SERVER_HLTV = 'p',
} A2S_SERVER;

char* a2s_get_environment(uint8_t env) {
    switch (env) {
        case A2S_ENVIRONMENT_LINUX:
            return "Linux";
        break;

        case A2S_ENVIRONMENT_WINDOWS:
            return "Windows";
        break;

        case A2S_ENVIRONMENT_MAC || A2S_ENVIRONMENT_MAC_ALT:
            return "macOS";
        break;
    
        default:
            return "Unknown";
        break;
    }
}

char* a2s_get_server_type(uint8_t srv) {
    switch (srv) {
        case A2S_SERVER_DEDICATED:
            return "Dedicated";
        break;

        case A2S_SERVER_NON_DEDICATED:
            return "Non dedicated";
        break;

        case A2S_SERVER_HLTV:
            return "SourceTV relay";
        break;

        default:
            return "Unknown";
        break;
    }
}
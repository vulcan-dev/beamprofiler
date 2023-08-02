#ifndef BP_CONFIG_H
#define BP_CONFIG_H

#include "common/bp_string.h"

typedef struct config_t
{
    // connection
    unsigned char ip[4];
    int port;

    // profiler
    int fps_limit;
    float history;
} config_t;

bool config_load(config_t* config);
void config_save(config_t* config);
bp::string config_get_ip(config_t* config);

#endif
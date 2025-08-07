#ifndef PS1_CONFIG_H

#include <stdbool.h>
#include <rlso.h>

typedef struct PS1Config {
    bool nocolor;
    bool fixspacing;
    int exitcode;
    So_Fx fmt_time;
    So_Fx fmt_user;
    So_Fx fmt_path;
    So_Fx fmt_icon;
    Color col_path;
} PS1Config;

#define PS1_CONFIG_H
#endif


#ifndef PS1_CONFIG_H

#include <stdbool.h>
#include <rphii/color.h>
#include <rphii/str.h>

typedef struct PS1Config {
    bool nocolor;
    bool fixspacing;
    StrFmtX fmt_time;
    StrFmtX fmt_user;
    StrFmtX fmt_path;
    StrFmtX fmt_icon;
    Color col_path;
} PS1Config;

#define PS1_CONFIG_H
#endif


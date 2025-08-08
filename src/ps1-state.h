#ifndef PS1_STATE_H

#include <rlso.h>
#include <rlarg.h>
#include "ps1-config.h"

typedef struct PS1State {
    PS1Config config;
    PS1Config preset;
    VSo subs;
    So **icons;
    So home;
    struct {
        struct ArgXGroup *icons;
    } dynarg;
} PS1State;

#define PS1_STATE_H
#endif


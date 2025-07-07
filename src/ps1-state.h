#ifndef PS1_STATE_H

#include <rphii/str.h>
#include <rphii/arg.h>
#include "ps1-config.h"

typedef struct PS1State {
    PS1Config config;
    PS1Config preset;
    VStr subs;
    Str **icons;
    struct {
        struct ArgXGroup *icons;
    } dynarg;
} PS1State;

#define PS1_STATE_H
#endif


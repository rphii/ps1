#define _GNU_SOURCE
#include <unistd.h>
#include <linux/limits.h>
#include <rlso.h>
#include <rlarg.h>
#include <rlc/array.h>
#include <time.h>
#include <pwd.h>

#include "ps1-state.h"

void icon_free(So **so) {
    free(*so);
}

int ps1_sub(void *void_state) {
    PS1State *state = (PS1State *)void_state;
    size_t len = array_len(state->subs);
    So recent = array_at(state->subs, len - 1);
    array_resize(state->icons, len);
    So **icon = array_it(state->icons, len - 1);
    *icon = malloc(sizeof(**icon));
    if(argx_get(state->dynarg.icons, recent)) return 0;
    struct ArgX *x=argx_init(state->dynarg.icons, 0, recent, so(""));
    argx_str(x, *icon, 0);
    return 0;
}

int main(const int argc, const char **argv) {
    
    PS1State state = {
        .config = {0},
        .preset = {
            .fmt_time.fg = {{ 0x767676ff }},
            .fmt_user.fg = {{ 0xd3777dff }},
            .fmt_icon.fg = {{ 0xffff00ff }},
            .fmt_path.fg = {{ 0x87af87ff }},
        }
    };

    int err = 0;
    struct Arg *arg = arg_new();
    struct ArgX *x = 0;
    struct ArgXGroup *o = 0;
    bool exit_early = false;
    arg_init(arg, so_l(argv[0]), so("Pretty PS1 print written in C"),
            so("Project page: " F("https://github.com/rphii/ps1", FG_BL_B UL) "\n"
                "To use it, put this in your .bashrc:\n"
                "  PROMPT_COMMAND='PS1=\"$(ps1 -X $?)\"'"
                ));

    arg_init_show_help(arg, false);

    o=argx_group(arg, so("Options"), false);
    argx_builtin_opt_help(o);
    argx_builtin_opt_source(o, so("/etc/ps1/ps1.conf"));
    argx_builtin_opt_source(o, so("$HOME/.config/rphiic/colors.conf"));
    argx_builtin_opt_source(o, so("$HOME/.config/ps1/ps1.conf"));
    argx_builtin_opt_source(o, so("$XDG_CONFIG_HOME/ps1/ps1.conf"));
    x=argx_init(o, 'C', so("nocolor"), so("output without color"));
      argx_bool(x, &state.config.nocolor, 0);
    x=argx_init(o, 'X', so("exitcode"), so("set exit code of ps1"));
      argx_int(x, &state.config.exitcode, 0);

    x=argx_init(o, 0, so("fmt-time"), so("time formatting"));
      argx_builtin_opt_fmtx(x, &state.config.fmt_time, &state.preset.fmt_time);
    x=argx_init(o, 0, so("fmt-user"), so("user formatting"));
      argx_builtin_opt_fmtx(x, &state.config.fmt_user, &state.preset.fmt_user);
    x=argx_init(o, 0, so("fmt-icon"), so("icon formatting"));
      argx_builtin_opt_fmtx(x, &state.config.fmt_icon, &state.preset.fmt_icon);
    x=argx_init(o, 0, so("fmt-path"), so("path formatting"));
      argx_builtin_opt_fmtx(x, &state.config.fmt_path, &state.preset.fmt_path);

    x=argx_init(o, 0, so("sub"), so("subscribe a path"));
      argx_vstr(x, &state.subs, 0);
      argx_func(x, 0, ps1_sub, &state, true, false);
    x=argx_init(o, 0, so("icon"), so("define an icon for a subscribed path"));
      state.dynarg.icons=argx_opt(x, 0, 0);

    o=argx_group(arg, so("Environment Variables"), false);
    argx_builtin_env_compgen(o);
    x=argx_env(o, so("HOME"), so("home path"), false);
      argx_str(x, &state.home, 0);


    o=argx_group(arg, so("Color Adjustments"), true);
    argx_builtin_opt_rice(o);

    TRYC(arg_parse(arg, argc, argv, &exit_early));
    if(exit_early) goto clean;


    state.config.fmt_time.bashsafe = true;
    state.config.fmt_time.nocolor = &state.config.nocolor;
    state.config.fmt_user.bashsafe = true;
    state.config.fmt_user.nocolor = &state.config.nocolor;
    state.config.fmt_icon.bashsafe = true;
    state.config.fmt_icon.nocolor = &state.config.nocolor;
    state.config.fmt_path.bashsafe = true;
    state.config.fmt_path.nocolor = &state.config.nocolor;

    uid_t uid = getuid();
    struct passwd *pw = getpwuid(uid);

    So out = SO;
    char ccwd[PATH_MAX];
    So login = so_l(pw ? pw->pw_name : "(?)");
    getcwd(ccwd, sizeof(ccwd));
    //So home = so_l(secure_getenv("HOME"));
    So home = state.home;
    So cwd = so_l(ccwd);
    
    time_t rawtime;
    time(&rawtime);
    struct tm *timeinfo = localtime(&rawtime);

#if 1
    /* format time */
    so_fmt_fx(&out, state.config.fmt_time, 0, "%02u:%02u", timeinfo->tm_hour, timeinfo->tm_min);
    so_push(&out, ' ');
#endif

#if 1
    /* format user */
    so_fmt_fx(&out, state.config.fmt_user, 0, "%.*s", SO_F(login));
    so_push(&out, ' ');
#endif

    /* format path */
    So path = SO;
    if(!so_cmp0(cwd, home)) {
        so_extend(&path, so("~"));
        so_extend(&path, so_i0(cwd, home.len));
    } else {
        path = cwd;
    }

#if 1
    /* format last icon */
#if 1
    So icon = so("");
    for(size_t i = 0; i < array_len(state.subs); ++i) {
        So path0 = array_at(state.subs, i);
        So *icon0 = array_at(state.icons, i);
        //printff("CMP[%.*s|%.*s]", SO_F(path),SO_F(path0));
        if(!so_cmp0(path, path0)) {
            icon = *icon0;
            //printff("GOT ICON:[%.*s]", SO_F(icon));
        }
        
    }
    if(icon.len) {
        so_fmt_fx(&out, state.config.fmt_icon, 0, "%.*s", SO_F(icon));
        so_push(&out, ' ');
    }
#else
    So icon = so("");
    if(!so_cmp0(path, so("~"))) icon = so("󱂟");
    if(!so_cmp0(path, so("~/dev"))) icon = so("");
    if(!so_cmp0(path, so("~/Downloads"))) icon = so("󱃩");
    if(!so_cmp0(path, so("/var/db/repos/gentoo"))) icon = so("");
    if(!so_cmp0(path, so("~/.config"))) icon = so("");
    so_fmt_fx(&out, state.config.fmt_icon, 0, "%.*s", SO_F(icon));
    so_push(&out, ' ');
#endif
#endif

#if 1
    size_t depth = 0;
    So_Fx fmt_path = state.config.fmt_path;
    for(So splice = {0}; so_splice(path, &splice, '/'); ++depth) {
        if(!splice.str) continue;
        bool single = (splice.str == path.str && ((!so_cmp(path, so("/"))) || (splice.str + splice.len == path.str + path.len)));
        bool last = ((splice.str + splice.len == path.str + path.len));
        bool first = !(splice.str > path.str);
        //if(last) col_path.rgba = 0x11ff11ff;
        char *folder = ((single && *splice.str == '/') || !first) ? "/" : "";
        fmt_path.bold = single;
        so_fmt_fx(&out, fmt_path, 0, "%s", folder);
        if(last && !single) fmt_path.fg.rgba = 0xffffffff;
        fmt_path.bold = last;
        so_fmt_fx(&out, fmt_path, 0, "%.*s", SO_F(splice));
        fmt_path.fg.r += 10;
        fmt_path.fg.g += 10;
        fmt_path.fg.b += 10;
    }
#endif
    so_push(&out, ' ');

    so_print(out);

clean:
    so_free(&out);
    so_free(&path);
    arg_free(&arg);
    vso_free(&state.subs);
    array_free_set(state.icons, So *, (ArrayFree)icon_free);
    array_free(state.icons);
    return state.config.exitcode;
    //return err;
error:
    goto clean;
}


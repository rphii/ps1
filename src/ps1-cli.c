#define _GNU_SOURCE
#include <unistd.h>
#include <linux/limits.h>
#include <rphii/str.h>
#include <rphii/arg.h>
#include <time.h>
#include <pwd.h>

#include "ps1-state.h"

int ps1_sub(void *void_state) {
    PS1State *state = (PS1State *)void_state;
    size_t len = array_len(state->subs);
    Str recent = array_at(state->subs, len - 1);
    array_resize(state->icons, len);
    Str **icon = array_it(state->icons, len - 1);
    *icon = malloc(sizeof(**icon));
    struct ArgX *x=argx_init(state->dynarg.icons, 0, recent, str(""));
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
    arg_init(arg, str_l(argv[0]), str("Pretty PS1 print written in C"),
            str("Project page: " F("https://github.com/rphii/ps1", FG_BL_B UL) "\n"
                "To use it, put this in your .bashrc:\n"
                "  PROMPT_COMMAND='PS1=\"$(ps1 -X $?)\"'"
                ));

    arg_init_show_help(arg, false);

    o=argx_group(arg, str("Options"), false);
    argx_builtin_opt_help(o);
    argx_builtin_opt_source(o, str("/etc/ps1/ps1.conf"));
    argx_builtin_opt_source(o, str("$HOME/.config/rphiic/colors.conf"));
    argx_builtin_opt_source(o, str("$HOME/.config/ps1/ps1.conf"));
    argx_builtin_opt_source(o, str("$XDG_CONFIG_HOME/ps1/ps1.conf"));
    x=argx_init(o, 'C', str("nocolor"), str("output without color"));
      argx_bool(x, &state.config.nocolor, 0);
    x=argx_init(o, 'X', str("exitcode"), str("set exit code of ps1"));
      argx_int(x, &state.config.exitcode, 0);

    x=argx_init(o, 0, str("fmt-time"), str("time formatting"));
      argx_builtin_opt_fmtx(x, &state.config.fmt_time, &state.preset.fmt_time);
    x=argx_init(o, 0, str("fmt-user"), str("user formatting"));
      argx_builtin_opt_fmtx(x, &state.config.fmt_user, &state.preset.fmt_user);
    x=argx_init(o, 0, str("fmt-icon"), str("icon formatting"));
      argx_builtin_opt_fmtx(x, &state.config.fmt_icon, &state.preset.fmt_icon);
    x=argx_init(o, 0, str("fmt-path"), str("path formatting"));
      argx_builtin_opt_fmtx(x, &state.config.fmt_path, &state.preset.fmt_path);

    x=argx_init(o, 0, str("sub"), str("subscribe a path"));
      argx_vstr(x, &state.subs, 0);
      argx_func(x, 0, ps1_sub, &state, true, false);
    x=argx_init(o, 0, str("icon"), str("define an icon for a subscribed path"));
      state.dynarg.icons=argx_opt(x, 0, 0);

    o=argx_group(arg, str("Environment Variables"), false);
    argx_builtin_env_compgen(o);

    o=argx_group(arg, str("Color Adjustments"), true);
    argx_builtin_opt_rice(o);

    TRYC(arg_parse(arg, argc, argv, &exit_early));
    if(exit_early) goto clean;

    state.config.fmt_time.bashsafe = true;
    state.config.fmt_user.bashsafe = true;
    state.config.fmt_icon.bashsafe = true;
    state.config.fmt_path.bashsafe = true;

    uid_t uid = getuid();
    struct passwd *pw = getpwuid(uid);

    Str out = STR_DYN();
    char ccwd[PATH_MAX];
    Str login = str_l(pw ? pw->pw_name : "(?)");
    getcwd(ccwd, sizeof(ccwd));
    Str home = str_l(secure_getenv("HOME"));
    Str cwd = str_l(ccwd);
    
    time_t rawtime;
    time(&rawtime);
    struct tm *timeinfo = localtime(&rawtime);

#if 1
    /* format time */
    str_fmtx(&out, state.config.fmt_time, "%02u:%02u", timeinfo->tm_hour, timeinfo->tm_min);
    str_push(&out, ' ');
#endif

#if 1
    /* format user */
    str_fmtx(&out, state.config.fmt_user, "%.*s", STR_F(login));
    str_push(&out, ' ');
#endif

    /* format path */
    Str path = STR_DYN();
    if(!str_cmp0(cwd, home)) {
        str_extend(&path, str("~"));
        str_extend(&path, str_i0(cwd, home.len));
    } else {
        path = cwd;
    }

#if 1
    /* format last icon */
#if 1
    Str icon = str("");
    for(size_t i = 0; i < array_len(state.subs); ++i) {
        Str path0 = array_at(state.subs, i);
        Str icon0 = *array_at(state.icons, i);
        //printff("CMP[%.*s|%.*s]", STR_F(path),STR_F(path0));
        if(!str_cmp0(path, path0)) {
            icon = icon0;
            //printff("GOT ICON:[%.*s]", STR_F(icon));
        }
        
    }
    if(icon.len) {
        str_fmtx(&out, state.config.fmt_icon, "%.*s", STR_F(icon));
        str_push(&out, ' ');
    }
#else
    Str icon = str("");
    if(!str_cmp0(path, str("~"))) icon = str("󱂟");
    if(!str_cmp0(path, str("~/dev"))) icon = str("");
    if(!str_cmp0(path, str("~/Downloads"))) icon = str("󱃩");
    if(!str_cmp0(path, str("/var/db/repos/gentoo"))) icon = str("");
    if(!str_cmp0(path, str("~/.config"))) icon = str("");
    str_fmtx(&out, state.config.fmt_icon, "%.*s", STR_F(icon));
    str_push(&out, ' ');
#endif
#endif

#if 1
    size_t depth = 0;
    StrFmtX fmt_path = state.config.fmt_path;
    for(Str splice = {0}; str_splice(path, &splice, '/'); ++depth) {
        if(!splice.str) continue;
        bool single = (splice.str == path.str && ((!str_cmp(path, str("/"))) || (splice.str + splice.len == path.str + path.len)));
        bool last = ((splice.str + splice.len == path.str + path.len));
        bool first = !(splice.str > path.str);
        //if(last) col_path.rgba = 0x11ff11ff;
        char *folder = ((single && *splice.str == '/') || !first) ? "/" : "";
        fmt_path.bold = single;
        str_fmtx(&out, fmt_path, "%s", folder);
        if(last && !single) fmt_path.fg.rgba = 0xffffffff;
        fmt_path.bold = last;
        str_fmtx(&out, fmt_path, "%.*s", STR_F(splice));
        fmt_path.fg.r += 10;
        fmt_path.fg.g += 10;
        fmt_path.fg.b += 10;
    }
#endif
    str_push(&out, ' ');

    str_print(out);

clean:
    str_free(&out);
    str_free(&path);
    return state.config.exitcode;
    //return err;
error:
    goto clean;
}


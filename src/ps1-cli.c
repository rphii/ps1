#define _GNU_SOURCE
#include <unistd.h>
#include <linux/limits.h>
#include <rphii/str.h>
#include <rphii/arg.h>
#include <time.h>
#include <pwd.h>

#include "ps1-config.h"

int main(const int argc, const char **argv) {
    
    PS1Config config = {0};
    PS1Config preset = {
        .fmt_time.fg = {{ 0x767676ff }},
        .fmt_user.fg = {{ 0xd3777dff }},
        .fmt_icon.fg = {{ 0xffff00ff }},
        .fmt_path.fg = {{ 0x87af87ff }},
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

    o=argx_group(arg, str("Options"));
    argx_builtin_opt_source(o, str("/etc/ps1/ps1.conf"));
    argx_builtin_opt_source(o, str("$HOME/.config/rphiic/colors.conf"));
    argx_builtin_opt_source(o, str("$HOME/.config/ps1/ps1.conf"));
    argx_builtin_opt_source(o, str("$XDG_CONFIG_HOME/ps1/ps1.conf"));
    argx_builtin_opt_help(o);
    x=argx_init(o, 'C', str("nocolor"), str("output without color"));
      argx_bool(x, &config.nocolor, 0);
    x=argx_init(o, 'X', str("exitcode"), str("set exit code of ps1"));
      argx_int(x, &config.exitcode, 0);

    x=argx_init(o, 0, str("fmt-time-fg"), str("color of time foreground"));
      argx_col(x, &config.fmt_time.fg, &preset.fmt_time.fg);
    x=argx_init(o, 0, str("fmt-time-bg"), str("color of time background"));
      argx_col(x, &config.fmt_time.bg, &preset.fmt_time.bg);
    x=argx_init(o, 0, str("fmt-time-bold"), str("time bold"));
      argx_bool(x, &config.fmt_time.bold, &preset.fmt_time.bold);
    x=argx_init(o, 0, str("fmt-time-italic"), str("time italic"));
      argx_bool(x, &config.fmt_time.italic, &preset.fmt_time.italic);
    x=argx_init(o, 0, str("fmt-time-underline"), str("time underline"));
      argx_bool(x, &config.fmt_time.underline, &preset.fmt_time.underline);

    x=argx_init(o, 0, str("fmt-user-fg"), str("color of user foreground"));
      argx_col(x, &config.fmt_user.fg, &preset.fmt_user.fg);
    x=argx_init(o, 0, str("fmt-user-bg"), str("color of user background"));
      argx_col(x, &config.fmt_user.bg, &preset.fmt_user.bg);
    x=argx_init(o, 0, str("fmt-user-bold"), str("user bold"));
      argx_bool(x, &config.fmt_user.bold, &preset.fmt_user.bold);
    x=argx_init(o, 0, str("fmt-user-italic"), str("user italic"));
      argx_bool(x, &config.fmt_user.italic, &preset.fmt_user.italic);
    x=argx_init(o, 0, str("fmt-user-underline"), str("user underline"));
      argx_bool(x, &config.fmt_user.underline, &preset.fmt_user.underline);

    x=argx_init(o, 0, str("fmt-icon-fg"), str("color of icon foreground"));
      argx_col(x, &config.fmt_icon.fg, &preset.fmt_icon.fg);
    x=argx_init(o, 0, str("fmt-icon-bg"), str("color of icon background"));
      argx_col(x, &config.fmt_icon.bg, &preset.fmt_icon.bg);
    x=argx_init(o, 0, str("fmt-icon-bold"), str("icon bold"));
      argx_bool(x, &config.fmt_icon.bold, &preset.fmt_icon.bold);
    x=argx_init(o, 0, str("fmt-icon-italic"), str("icon italic"));
      argx_bool(x, &config.fmt_icon.italic, &preset.fmt_icon.italic);
    x=argx_init(o, 0, str("fmt-icon-underline"), str("icon underline"));
      argx_bool(x, &config.fmt_icon.underline, &preset.fmt_icon.underline);

    x=argx_init(o, 0, str("fmt-path-fg"), str("color of path foreground"));
      argx_col(x, &config.fmt_path.fg, &preset.fmt_path.fg);
    x=argx_init(o, 0, str("fmt-path-bg"), str("color of path background"));
      argx_col(x, &config.fmt_path.bg, &preset.fmt_path.bg);
    x=argx_init(o, 0, str("fmt-path-bold"), str("path bold"));
      argx_bool(x, &config.fmt_path.bold, &preset.fmt_path.bold);
    x=argx_init(o, 0, str("fmt-path-italic"), str("path italic"));
      argx_bool(x, &config.fmt_path.italic, &preset.fmt_path.italic);
    x=argx_init(o, 0, str("fmt-path-underline"), str("path underline"));
      argx_bool(x, &config.fmt_path.underline, &preset.fmt_path.underline);

    o=argx_group(arg, str("Environment Variables"));
    argx_builtin_env_compgen(o);

    o=argx_group(arg, str("Color Adjustments"));
    argx_builtin_opt_rice(o);

    TRYC(arg_parse(arg, argc, argv, &exit_early));
    if(exit_early) goto clean;

    config.fmt_time.bashsafe = true;
    config.fmt_user.bashsafe = true;
    config.fmt_icon.bashsafe = true;
    config.fmt_path.bashsafe = true;

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
    str_fmtx(&out, config.fmt_time, "%02u:%02u", timeinfo->tm_hour, timeinfo->tm_min);
    str_push(&out, ' ');
#endif

#if 1
    /* format user */
    str_fmtx(&out, config.fmt_user, "%.*s", STR_F(login));
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
    Str icon = str("");
    if(!str_cmp0(path, str("~"))) icon = str("󱂟");
    if(!str_cmp0(path, str("~/dev"))) icon = str("");
    if(!str_cmp0(path, str("~/Downloads"))) icon = str("󱃩");
    if(!str_cmp0(path, str("/var/db/repos/gentoo"))) icon = str("");
    str_fmtx(&out, config.fmt_icon, "%.*s", STR_F(icon));
    str_push(&out, ' ');
#endif

#if 1
    size_t depth = 0;
    StrFmtX fmt_path = config.fmt_path;
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
    return config.exitcode;
    //return err;
error:
    goto clean;
}


#define _GNU_SOURCE
#include <unistd.h>
#include <linux/limits.h>
#include <rphii/str.h>
#include <rphii/arg.h>
#include <time.h>
#include <pwd.h>

typedef struct PS1Config {
    bool nocolor;
} PS1Config;

int main(const int argc, const char **argv) {

    int err = 0;
    PS1Config config = {0};
    struct Arg *arg = arg_new();
    struct ArgX *x = 0;
    bool exit_early = false;
    arg_init(arg, str_l(argv[0]), str("Pretty PS1 print written in C"),
            str("Project page: " F("https://github.com/rphii/ps1", FG_BL_B UL) "\n"
                "To use it, put this in your .bashrc:\n"
                "  export PS1='$(ps1 -C)\\[\\r$(ps1)\\] '"
                ));
    arg_init_width(arg, 100, 0);
    arg_init_show_help(arg, false);
    x=argx_init(arg_opt(arg), 'h', str("help"), str("display this help"));
      argx_help(x, arg);
    x=argx_init(arg_opt(arg), 'C', str("nocolor"), str("output without color"));
      argx_bool(x, &config.nocolor, 0);

    TRYC(arg_parse(arg, argc, argv, &exit_early));
    if(exit_early) goto clean;

    uid_t uid = getuid();
    struct passwd *pw = getpwuid(uid);

    Str out = STR_DYN();
    char ccwd[PATH_MAX];
    Str login = str_l(pw ? pw->pw_name : "(?)");
    getcwd(ccwd, sizeof(ccwd));
    Str home = str_l(secure_getenv("HOME"));
    Str cwd = str_l(ccwd);

    Color col_time = {{ 0x767676ff }};
    Color col_user = {{ 0xd3777dff }};
    Color col_path = {{ 0x87af87ff }};
    Color col_icon = {{ 0xffff00ff }};

    StrFmtX x_time = config.nocolor ? (StrFmtX){0} : (StrFmtX){ .fg = col_time };
    StrFmtX x_user = config.nocolor ? (StrFmtX){0} : (StrFmtX){ .fg = col_user };
    StrFmtX x_icon = config.nocolor ? (StrFmtX){0} : (StrFmtX){ .fg = col_icon };
    
    time_t rawtime;
    time(&rawtime);
    struct tm *timeinfo = localtime(&rawtime);

    /* format time */
    str_fmtx(&out, x_time, "%02u:%02u", timeinfo->tm_hour, timeinfo->tm_min);
    str_push(&out, ' ');

    /* format user */
    str_fmtx(&out, x_user, "%.*s", STR_F(login));
    str_push(&out, ' ');

    /* format path */
    Str path = STR_DYN();
    if(!str_cmp0(cwd, home)) {
        str_extend(&path, str("~"));
        str_extend(&path, str_i0(cwd, home.len));
    } else {
        path = cwd;
    }

    /* format last icon */
    Str icon = str("");
    if(!str_cmp0(path, str("~"))) icon = str("󱂟");
    if(!str_cmp0(path, str("~/dev"))) icon = str("");
    if(!str_cmp0(path, str("~/Downloads"))) icon = str("󱃩");
    if(!str_cmp0(path, str("/var/db/repos/gentoo"))) icon = str("");
    str_fmtx(&out, x_icon, "%.*s", STR_F(icon));
    str_push(&out, ' ');

    size_t depth = 0;
    for(Str splice = {0}; str_splice(path, &splice, '/'); ++depth) {
        if(!splice.str) continue;
        bool single = (splice.str == path.str && ((!str_cmp(path, str("/"))) || (splice.str + splice.len == path.str + path.len)));
        bool last = ((splice.str + splice.len == path.str + path.len));
        bool first = !(splice.str > path.str);
        //if(last) col_path.rgba = 0x11ff11ff;
        char *folder = ((single && *splice.str == '/') || !first) ? "/" : "";
        str_fmtx(&out, config.nocolor ? (StrFmtX){0} : (StrFmtX){ .fg = col_path, .bold = single }, "%s", folder);
        if(last && !single) col_path.rgba = 0xffffffff;
        str_fmtx(&out, config.nocolor ? (StrFmtX){0} : (StrFmtX){ .fg = col_path, .bold = last }, "%.*s", STR_F(splice));
        col_path.r += 10;
        col_path.g += 10;
        col_path.b += 10;
    }
    //str_push(&out, ' ');

    str_print(out);

clean:
    str_free(&out);
    str_free(&path);
    return err;
error:
    ERR_CLEAN;
}


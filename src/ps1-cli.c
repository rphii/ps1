#define _GNU_SOURCE
#include <unistd.h>
#include <linux/limits.h>
#include <rphii/str.h>
#include <rphii/arg.h>
#include <time.h>
#include <pwd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <rphii/file.h>

#include <ps1-config.h>


int get_pos(int *x, int *y) {

    char buf[30]={0};
    int ret, i, pow;
    char ch;

    *y = 0; *x = 0;

    struct termios term, restore;

    tcgetattr(0, &term);
    tcgetattr(0, &restore);
    term.c_lflag &= ~(ICANON|ECHO);
    tcsetattr(0, TCSANOW, &term);

    write(1, "\033[6n", 4);

    for( i = 0, ch = 0; ch != 'R'; i++ )
    {
        ret = read(0, &ch, 1);
        if ( !ret ) {
            tcsetattr(0, TCSANOW, &restore);
            fprintf(stderr, "getpos: error reading response!\n");
            return 1;
        }
        buf[i] = ch;
        //printf("buf[%d]: \t%c \t%d\n", i, ch, ch);
    }

    if (i < 2) {
        tcsetattr(0, TCSANOW, &restore);
        //printf("i < 2\n");
        return(1);
    }

    for( i -= 2, pow = 1; buf[i] != ';'; i--, pow *= 10)
        *x = *x + ( buf[i] - '0' ) * pow;

    for( i-- , pow = 1; buf[i] != '['; i--, pow *= 10)
        *y = *y + ( buf[i] - '0' ) * pow;

    tcsetattr(0, TCSANOW, &restore);
    return 0;
}

int main(const int argc, const char **argv) {
    
    PS1Config preset = {
        .fmt_time.fg = {{ 0x767676ff }},
    };

    int err = 0;
    PS1Config config = {0};
    struct Arg *arg = arg_new();
    struct ArgX *x = 0;
    bool exit_early = false;
    arg_init(arg, str_l(argv[0]), str("Pretty PS1 print written in C"),
            str("Project page: " F("https://github.com/rphii/ps1", FG_BL_B UL) "\n"
                "To use it, put this in your .bashrc:\n"
                "  PROMPT_COMMAND='PS1=\"$(ps1)\"'"
                ));
    arg_init_width(arg, 100, 0);
    arg_init_show_help(arg, false);
    x=argx_init(arg_opt(arg), 'h', str("help"), str("display this help"));
      argx_help(x, arg);
    x=argx_init(arg_opt(arg), 'C', str("nocolor"), str("output without color"));
      argx_bool(x, &config.nocolor, 0);
    x=argx_init(arg_opt(arg), 0, str("fmt-time-fg"), str("color of time foreground"));
      argx_col(x, &config.fmt_time.fg, &preset.fmt_time.fg);
    x=argx_init(arg_opt(arg), 0, str("fmt-time-bg"), str("color of time background"));
      argx_col(x, &config.fmt_time.bg, &preset.fmt_time.bg);
    x=argx_init(arg_opt(arg), 0, str("fmt-time-bold"), str("time bold"));
      argx_bool(x, &config.fmt_time.bold, &preset.fmt_time.bold);
    x=argx_init(arg_opt(arg), 0, str("fmt-time-italic"), str("time italic"));
      argx_bool(x, &config.fmt_time.italic, &preset.fmt_time.italic);
    x=argx_init(arg_opt(arg), 0, str("fmt-time-underline"), str("time underline"));
      argx_bool(x, &config.fmt_time.underline, &preset.fmt_time.underline);

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

    //Color col_time = {{ 0x767676ff }};
    Color col_user = {{ 0xd3777dff }};
    Color col_path = {{ 0x87af87ff }};
    Color col_icon = {{ 0xffff00ff }};

    StrFmtX x_time = config.fmt_time;
    x_time.nocolor = &config.nocolor;
    x_time.bashsafe = true;
    StrFmtX x_user = config.nocolor ? (StrFmtX){0} : (StrFmtX){ .bashsafe = true, .fg = col_user };
    StrFmtX x_icon = config.nocolor ? (StrFmtX){0} : (StrFmtX){ .bashsafe = true, .fg = col_icon };
    
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
        str_fmtx(&out, config.nocolor ? (StrFmtX){0} : (StrFmtX){ .bashsafe = true, .fg = col_path, .bold = single }, "%s", folder);
        if(last && !single) col_path.rgba = 0xffffffff;
        str_fmtx(&out, config.nocolor ? (StrFmtX){0} : (StrFmtX){  .bashsafe = true,.fg = col_path, .bold = last }, "%.*s", STR_F(splice));
        col_path.r += 10;
        col_path.g += 10;
        col_path.b += 10;
    }
    str_push(&out, ' ');

    str_print(out);

clean:
    str_free(&out);
    str_free(&path);
    return err;
error:
    ERR_CLEAN;
}


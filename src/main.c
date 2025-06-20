#define _GNU_SOURCE
#include <unistd.h>
#include <linux/limits.h>
#include <rphii/str.h>
#include <time.h>
#include <pwd.h>

int main(const int argc, const char **argv) {

    uid_t uid = getuid();
    struct passwd *pw = getpwuid(uid);

    Str out = STR_DYN(), tmp = STR_DYN();;
    char ccwd[PATH_MAX];
    Str login = str_l(pw ? pw->pw_name : "(?)");
    getcwd(ccwd, sizeof(ccwd));
    Str home = str_l(secure_getenv("HOME"));
    Str cwd = str_l(ccwd);

    Color col_err =  { 0xff0000ff };
    Color col_time = { 0x767676ff };
    Color col_user = { 0xd3777dff };
    Color col_path = { 0x87af87ff };
    Color col_icon = { 0xffff00ff };
    
    time_t rawtime;
    time(&rawtime);
    struct tm *timeinfo = localtime(&rawtime);

    /* format time */
    str_fmtx(&out, (StrFmtX){ .fg = col_time }, "%02u:%02u", timeinfo->tm_hour, timeinfo->tm_min);
    str_push(&out, ' ');

    /* format user */
    str_fmtx(&out, (StrFmtX){ .fg = col_user, .bold = true }, "%.*s", STR_F(login));
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
    str_fmtx(&out, (StrFmtX){ .fg = col_icon, .bold = true }, "%.*s", STR_F(icon));
    str_push(&out, ' ');

    size_t depth = 0;
    for(Str splice = {0}; str_splice(path, &splice, '/'); ++depth) {
        if(!splice.str) continue;
        bool single = (splice.str == path.str && ((!str_cmp(path, str("/"))) || (splice.str + splice.len == path.str + path.len)));
        bool last = ((splice.str + splice.len == path.str + path.len));
        bool first = !(splice.str > path.str);
        //if(last) col_path.rgba = 0x11ff11ff;
        char *folder = ((single && *splice.str == '/') || !first) ? "/" : "";
        str_fmtx(&out, (StrFmtX){ .fg = col_path, .bold = single }, "%s", folder);
        if(last && !single) col_path.rgba = 0xffffffff;
        str_fmtx(&out, (StrFmtX){ .fg = col_path, .bold = last }, "%.*s", STR_F(splice));
        col_path.r += 10;
        col_path.g += 10;
        col_path.b += 10;
    }
    str_push(&out, ' ');

    str_print(out);

    str_free(&out);
    str_free(&path);
    return 0;
}


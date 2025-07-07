# WORK IN PROGRESS! LIKELY TO BE UNSTABLE!

# ps1

## usage

Set your `PS1` in your `~/.bashrc`:

```sh
PROMPT_COMMAND='PS1="$(ps1 -X $?)"'
```

- we forward the exit code of the previous program into ps1 so we retain the exit code

## install

```
meson setup build
meson install -C build
```

## example config

e.g. `[~/.config/ps1/ps1.config]` or see `ps1 -h --source` for other paths

```
# subscribe to certain paths (before assigning icons)
sub = ~
sub = ~/dev
sub = ~/Downloads 
sub = ~/.config
sub = /var/db/repos/gentoo

# now assign a few icons
icon = ~ = 󱂟
icon = ~/dev = 
icon = ~/Downloads = 󱃩
icon = ~/.config = 
icon = /var/db/repos/gentoo = 
```


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


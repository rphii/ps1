# ps1

## usage

Set your `PS1` in your `~/.bashrc`:

```sh
PROMPT_COMMAND='PS1="$(ps1)"'
```

Explanation:
- first we call the program to print the output, but without colors and no formatting (`-C`)
- we start a block that ignores formatting widths (required because we output colored text) via `\[` .. until `\]`
- reset the cursor position to the start with `\r`
- then, we call the regular command (`ps1`) which colorizes the output

## install

```
meson setup build
meson install -C build
```


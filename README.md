# GentleOS/16

A fantasy operating system for vintage 16-bit PCs,
built for tinkering with old hardware on the bare metal.

You can find more information on its [website](https://luke8086.dev/gentleos/16).

It has a 32-bit sibling with a more advanced GUI, called
[GentleOS/32](https://github.com/luke8086/gentleos32).

<img src="https://luke8086.dev/gentleos/images/gentleos16-photo-t1100fd-nobg.webp" width="300">

## Building

The only prerequisite is [DOSBox](https://www.dosbox.com/), the
entire toolchain (OpenWatcom, NASM, Perl) is included in the repo.

To build GentleOS, start DOSBox from the source directory and run
the following commands:

```dos
Z:\>MOUNT C .
Z:\>C:
C:\>ENV
C:\>AUTOGEN
C:\>WMAKE
```

## Testing

In DOSBox, run:

```dos
C:\>BOOT GT16FD14.IMG
```

or just:

```dos
C:\>GT16
```

## Development notes

- The official compiler is [OpenWatcom 1.9](https://www.openwatcom.org/),
  because it can be freely distributed, but
  [Turbo C 2.01](https://duckduckgo.com/?q=Turbo+C+2x)
  is also supported and it's much faster.
  You can install it to `C:\TMP\TC` and use with `WMAKE TC=1`

- For faster compilation, you can try setting `cycles=fixed 99999`
  in the `[cpu]` section of your DOSBox config file

- For a quick turnaround, GentleOS can be started as a COM file
  with `GT16.COM`. Pressing `Shift-Q` returns back to DOS.

## Attributions

- Assets in [vendor/icons8](vendor/icons8) have been sourced from
  [Icons8](https://icons8.com/) using the
  [free license](https://web.archive.org/web/20260325111643/https://icons8.com/license)
  and modified

- Assets in [vendor/mona](vendor/mona) have been extracted from the
  [Mona Font](https://github.com/MonadABXY/mona-font) and modified
  ([LICENSE](vendor/mona/LICENSE.txt))

- Assets in [vendor/int10h](vendor/int10h) have been extracted from the
  [The Ultimate Oldschool PC Font Pack](https://int10h.org/oldschool-pc-fonts/)
  and modified ([LICENSE](vendor/int10h/LICENSE.txt))

- Sokoban levels in [vendor/sokoban](vendor/sokoban) have been created by
  David W. Skinner and obtained from the
  [Sasquatch collection](https://web.archive.org/web/20040725085419/https://users.bentonrea.com/~sasquatch/sokoban/)
  ([README](vendor/sokoban/README.txt))

- Assets in [vendor/misc](vendor/misc) have been found online and modified
  ([README](vendor/misc/README.txt))

## Contributors

-  Alexander Rau ([l00nix](https://github.com/l00nix)) -
   Tested and made a release for [HP 200LX](https://github.com/l00nix/gentleos-hp200lx)

## License

Except where otherwise noted, GentleOS/16 is licensed under [GPLv2](LICENSE).

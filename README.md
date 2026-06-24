# GentleOS/16

A hobby operating system for vintage 16-bit PCs,
built for tinkering with old hardware on the bare metal.

You can find more information on its [website](https://luke8086.dev/gentleos16).

It's a simplified version of [GentleOS/32](https://github.com/luke8086/gentleos32),
targetting i386+ devices.

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

## Development notes

- The official compiler is [OpenWatcom 1.9](https://www.openwatcom.org/),
  because it can be freely distributed, but
  [Turbo C 2.01](https://duckduckgo.com/?q=Turbo+C+2x)
  is also supported and it's much faster.
  You can install it to `C:\TMP\TC` and use with `WMAKE TC=1`

- For faster compilation, you can try setting `cycles=fixed 99999`
  in the `[cpu]` section of your DOSBox config file

- For a quick turnaround, GentleOS can be started as a COM file
  with `BUILD\GENTLEOS.COM`. Pressing `Shift-Q` returns back to DOS.

## Attributions

- All images in [vendor/icons8](vendor/icons8) have been sourced from
  [Icons8](https://icons8.com/) using the
  [free license](https://web.archive.org/web/20260325111643/https://icons8.com/license)
  and modified

- All images in [vendor/mona](vendor/mona) have been extracted from the
  [Mona Font](https://github.com/MonadABXY/mona-font) and modified
  ([LICENSE](vendor/mona/LICENSE.txt))

- All fonts in [vendor/int10h](vendor/int10h) have been extracted from the
  [The Ultimate Oldschool PC Font Pack](https://int10h.org/oldschool-pc-fonts/)
  and modified ([LICENSE](vendor/int10h/LICENSE.txt))

- The [Atari Small](vendor/atarism) font by [Tom Fine](https://hea-www.harvard.edu/~fine/)
  has been obtained from https://hea-www.harvard.edu/~fine/Tech/x11fonts.html
  ([LICENSE](vendor/atarism/LICENSE.txt))

- [Font 4x6](vendor/font4x6) by [Luiz Bills](https://www.luizpb.com/) has been
  sourced from https://github.com/luizbills/font4x6
  ([LICENSE](vendor/font4x6/LICENSE.txt))

## Contributors

-  Alexander Rau ([l00nix](https://github.com/l00nix)) -
   Tested and made a release for [HP 200LX](https://github.com/l00nix/gentleos-hp200lx)

## License

Except where otherwise noted, GentleOS/16 is licensed under [GPLv2](LICENSE).

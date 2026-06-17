# GentleOS/16

A hobby operating system for vintage 16-bit PCs.

Its goal is to provide a simple platform for tinkering with retro
hardware and running graphical interactive apps on bare metal.

At minimum, it only requires an 8086+ CPU, 192 KB of RAM, and a CGA display
supporting 320x200x4 mode.

By design it's entirely monolithic, mostly configured at compile time,
and only supports standard PC devices: CGA/VGA, keyboard, PC speaker.
The only future plans are bugfixes, optimizations, and adding more apps.

GentleOS/16 is a sibling of [GentleOS/32](https://github.com/luke8086/gentleos32),
a slightly more advanced 32-bit OS that targets i386+ devices.

For details on building and running, see [USAGE.md](USAGE.md).

<img src="doc/machimg/t1100fd.webp" width="400">

## Gallery

Amstrad PPC 512 (NEC V30 CPU, 512KB RAM, 640x200 STN LCD):

<img src="doc/machimg/ppc512.webp" width="360"> <img src="doc/machimg/ppc512-2.webp" width="360">

All apps running on Tandy 1100FD (NEC V20 CPU, 640KB RAM, 640x200 STN LCD):

<p>
  <img src="doc/appimg/launcher.webp" width="240">
  <img src="doc/appimg/clock.webp" width="240">
  <img src="doc/appimg/calendar.webp" width="240">
  <img src="doc/appimg/calc.webp" width="240">
  <img src="doc/appimg/fonts.webp" width="240">
  <img src="doc/appimg/keys.webp" width="240">
  <img src="doc/appimg/sounds.webp" width="240">
  <img src="doc/appimg/mines.webp" width="240">
  <img src="doc/appimg/pairs.webp" width="240">
  <img src="doc/appimg/mahjong.webp" width="240">
  <img src="doc/appimg/snake.webp" width="240">
  <img src="doc/appimg/tetris.webp" width="240">
  <img src="doc/appimg/freecell.webp" width="240">
  <img src="doc/appimg/klondike.webp" width="240">
  <img src="doc/appimg/bjack.webp" width="240">
  <img src="doc/appimg/setup.webp" width="240">
</p>

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

# GentleOS/16 - Usage and development

## Pre-built images

For a quick test, you can find pre-built images of GentleOS in
[Releases](https://github.com/luke8086/gentleos/releases).

## Building

The only prerequisite is [DOSBox](https://www.dosbox.com/), the
entire toolchain (OpenWatcom, NASM, Perl) is included in the repo.

To build GentleOS, start DOSBox from the source directory and run
the following commands:

```dos
Z:\>MOUNT C .
Z:\>C:
C:\>ENV
C:\>COPY _CONFIG.H CONFIG.H
C:\>AUTOGEN
C:\>WMAKE
```

## Running in DOSBox

GentleOS can be booted in DOSBox as a standalone OS:

```dos
C:\>BOOT BUILD\FD1440.IMG
```

## Running in QEMU

Run:

```bash
$ qemu-system-i386 -m 1 -drive format=raw,if=floppy,file=BUILD/FD1440.IMG -serial stdio
```

For audio support on Macs, also add:

```bash
-audiodev coreaudio,id=snd0 -machine pcspk-audiodev=snd0
```

## Running on real devices

> [!WARNING]
> The author takes no responsibility for any lost data
> or damaged hardware. Proceed at your own risk, and only if you
> fully understand what you're doing.

To prepare a bootable HDD/floppy/pendrive, run the following command.
Note it'll **permanently destroy** data on the target disk, and there
will be no confirmation prompt.

```bash
dd if=BUILD/DISK.IMG of=<TARGET DISK> bs=32k conv=fsync status=progress
```

There's no need to copy the entire `FDxxx.IMG` images, they're just
padded with zeros for use in emulators.

Alternatively, if you have [Gotek](https://www.gotekemulator.com/) with
[FlashFloppy](https://github.com/keirf/flashfloppy), you can directly
use `BUILD/FD720.IMG` or `BUILD/FD1440.IMG` as raw disk images.

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

# DreamerOS

A gold-and-navy themed Linux distribution built on Arch Linux, with a
custom graphical installer (C++/Qt6) instead of a bare Arch install. Three
editions — **Game**, **Develop**, and **Scratch** — each shaped for a
different kind of machine.

This repository holds the installer source, the original page-by-page
mockups, the boot splash animation, and every design decision made along
the way — the same way you'd browse the Linux kernel's own source on
GitHub.

## Repository layout

```
installer/          Standalone Qt6/C++ installer (no external installer
                     framework — talks to parted/pacstrap/arch-chroot
                     directly). This is the current, actively developed
                     installer.

legacy-calamares/    An earlier version of the same installer, built as a
                     set of Calamares view-module plugins instead. Kept
                     for reference; not the version DreamerOS ships.

assets/
  mockups/           Every installer page as a static design mockup,
                      before it became code. original-pages/ is the
                      first-pass package; new-pages/ is everything added
                      since (network page, wallpaper picker, NTP toggle,
                      About screen, etc.)
  wallpapers/        The 30-file two-color wallpaper set.
  splash/            150 individual PNG frames for the boot animation
                      (split into frames/part1/ and frames/part2/, 75
                      each, purely because GitHub's web upload UI caps
                      out at 100 files per drop — no meaning to the
                      split beyond that), plus a compiled preview GIF.
  logo.png           The DreamerOS gear+penguin badge, extracted with a
                      transparent background.

docs/
  SESSION-NOTES.txt        Full narrative of how this project came
                             together — decisions, bugs found and fixed,
                             and the methodology behind the mockups and
                             splash animation.
  BUILD_NOTES.txt          Notes from the original package.
  DECISIONS_LOG.txt        Formal decision log from the original package.
```

## Editions

| Edition | For | Notes |
|---|---|---|
| **Game** | Gamers | Lightweight, but won't save an old machine. MangoHud is optional. |
| **Develop** | Builders | Full desktop, dev tools ready from first boot. |
| **Scratch** | Minimal setups | Stripped KDE, terminal + Brave only. Runs on 4GB RAM / 32GB storage. |

## Building the installer

See [`installer/README.md`](installer/README.md) for build instructions
(Qt6, CMake, needs to run on an Arch-based system since it shells out to
`pacstrap`/`arch-chroot`/`parted`).

## Status

Everything here is design-and-code stage. No archiso profile exists yet to
turn this into a bootable `.iso` — that's the next piece of work. Nothing
in `installer/` has been compiled yet outside of this repository; it's
written to a real, buildable shape but needs an actual Arch + Qt6
toolchain to confirm it compiles cleanly.

## License

GPLv3 — matches the Linux kernel and most of what DreamerOS is built on.
See `LICENSE` (added via GitHub when the repository was created).

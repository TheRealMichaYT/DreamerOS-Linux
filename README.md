DreamerOS

A gold-and-navy themed Linux distribution built on Arch Linux, with a custom graphical installer (C++/Qt6) instead of a bare Arch install. Three editions — Game, Develop, and Scratch — each shaped for a different kind of machine.

This repository holds the installer source, a selection of the page mockups that shaped its design, the boot splash animation, and the project's documentation — the same way you'd browse the Linux kernel's own source on GitHub.

Repository layout
installer/          Standalone Qt6/C++ installer (no external installer
                     framework — talks to parted/pacstrap/arch-chroot
                     directly). This is the current, actively developed
                     installer.

legacy-calamares/    An earlier version of the same installer, built as a
                     set of Calamares view-module plugins instead. Kept
                     for reference; not the version DreamerOS ships.

assets/
  mockups/           Installer pages/ — page mockups showing how the
                      installer's design evolved: network page, wallpaper
                      picker, NTP toggle, disk encryption, About screen.
  wallpapers/        The 30-file two-color wallpaper set.
  splash/            preview.gif — a compiled preview of the 150-frame
                      boot animation.
  logo.png           The DreamerOS gear+penguin badge, extracted with a
                      transparent background.

docs/
  SESSION-NOTES.txt        Narrative of how this project came together —
                             decisions, bugs found and fixed, and the
                             methodology behind the mockups and splash
                             animation.
  BUILD_NOTES.txt          Notes from the original package.
  DECISIONS_LOG.txt        Formal decision log from the original package.
Editions
Edition	For	Notes
Game	Gamers	Lightweight, but won't save an old machine. MangoHud is optional.
Develop	Builders	Full desktop, dev tools ready from first boot.
Scratch	Minimal setups	Stripped KDE, terminal + Brave only. Runs on 4GB RAM / 32GB storage.
Building the installer

See installer/README.md for build instructions (Qt6, CMake, needs to run on an Arch-based system since it shells out to pacstrap/arch-chroot/parted).

Status

Everything here is design-and-code stage. No archiso profile exists yet to turn this into a bootable .iso — that's the next piece of work. Nothing in installer/ has been compiled yet outside of this repository; it's written to a real, buildable shape but needs an actual Arch + Qt6 toolchain to confirm it compiles cleanly.

License

GPLv3 — matches the Linux kernel and most of what DreamerOS is built on. See LICENSE.

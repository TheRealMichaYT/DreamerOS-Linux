# DreamerOS Installer — standalone C++ (no Calamares)

This is a full rewrite of the earlier Calamares-plugin version as a single
standalone Qt6 application. Nothing here depends on libcalamares — this is
its own executable that talks to the system directly (`parted`, `pacstrap`,
`arch-chroot`, `mkfs`, NetworkManager over D-Bus, `grub-install`).

## Why this exists

The earlier version was a set of Calamares view-module plugins
(`modules/dreamernetwork`, `modules/dreamervariant`, `modules/dreamerdesktop`)
plus YAML config for Calamares's own built-in modules (users, partition,
hostname, summary, finished). That's still a perfectly reasonable way to
build an Arch installer — Calamares is what most Arch-based distros use.
This version exists because the explicit ask was "no Calamares."

## What's real vs. reference implementation

Everything here is real, compilable-shape C++ using real system commands —
this isn't pseudocode. That said, like any reference implementation:

- Error handling is present but minimal (a production build would surface
  `errorOut` strings from `PartitionUtil` in the UI, not just fail silently)
- `PartitionPage`'s "Manual" mode has no mountpoint-table UI yet — the
  `InstallerState::partitioning` fields it would write into already exist,
  the widget to fill them in doesn't
- `LocalePage::paintWorldMap()` is declared but not implemented — the map
  image displays, but isn't yet wired to clicking → setting `region`/`timezone`
- No packaged app icon / desktop file for a "real" ISO overlay yet

None of this was compiled or run — there's no Arch/Qt6 toolchain in this
sandbox. It needs a real build on the NUC to know if it compiles cleanly.

## Project layout

```
dreameros-installer/
  CMakeLists.txt
  src/
    main.cpp                 — entry point, loads the stylesheet
    MainWindow.{h,cpp}        — page stack + NEXT/BACK, replaces Calamares ViewManager
    InstallerPage.h           — base class every page implements
    InstallerState.{h,cpp}    — shared state, replaces Calamares GlobalStorage
    InstallWorker.{h,cpp}     — QThread running the real install (page10)
    PartitionUtil.{h,cpp}     — lsblk/parted/mkfs wrappers
    NetworkManagerClient.{h,cpp} — NetworkManager D-Bus wrapper
    pages/
      WelcomePage        — page01
      NetworkPage        — page01b
      LocalePage         — page02 (+ NTP toggle)
      VariantPage        — page03 + page03b (sub-page handling via handleNext/handleBack)
      UsersPage          — page04a-d
      HostnamePage       — page05
      DesktopPage        — page06/06b/06c/06d (self-skips on Scratch)
      PartitionPage      — page07
      SummaryPage        — page08
      InstallPage        — page09 + page10 (live log, no fake progress bar)
      FinishedPage       — page11
  resources/
    dreameros.qss         — the gold-and-navy theme as a Qt stylesheet
    resources.qrc
    dreameros-logo.png     — real extracted logo asset
    world-map.png          — PLACEHOLDER, replace before shipping
```

## Building on the NUC

```bash
sudo pacman -S --needed base-devel cmake qt6-base qt6-tools

cd dreameros-installer
mkdir build && cd build
cmake ..
make -j$(nproc)
```

The resulting `dreameros-installer` binary needs to run as root (or via
`sudo`) to actually partition disks, pacstrap, and chroot — same privilege
requirement Calamares itself has.

## What still needs real system assets

`InstallWorker` and the pages reference these paths, which need to exist on
whatever live ISO this ships inside (same as the Calamares version needed):

- `/usr/share/dreameros/wallpapers/*.png` — the 30-wallpaper set + the
  default diagonal-lines wallpaper
- `/usr/share/dreameros/assets/black.png` — Scratch's fixed wallpaper

## Not done here

- The archiso profile that actually builds a bootable `.iso` around this
  binary — that's a separate piece of work, not part of "build the pages."
- The ABI-mismatch bug noted earlier in the project — moot now, since this
  version isn't a Calamares plugin anymore, so there's no Calamares-version
  header mismatch to have. Worth double-checking there isn't an equivalent
  Qt6-version mismatch between what's on the NUC and what the live ISO ships.

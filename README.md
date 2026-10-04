# DeskCat

<p align="center">
  <img src="docs/cat-animation.webp" alt="DeskCat animation" width="480">
</p>

DeskCat is a lightweight work-and-break timer for Windows and Linux, built
with C++20, Qt 6, and QML. After the initial setup, it stays in the system
tray, counts down the work interval, and displays a full-screen cat during
breaks.

## Features

- separate work and break durations;
- smooth cat entrance, settling down, sleeping, waking up, and leaving;
- full-screen break mode on every connected monitor;
- background dimming and a strict mode without an early-finish button;
- a click reaction and an annoyed reaction to four rapid clicks;
- synchronized footsteps and purring while walking, plus background purring
  during breaks;
- three independent switches for these sounds;
- a mandatory meow during the annoyed four-click reaction;
- work-timer pause and system-tray controls;
- persistent settings and per-user autostart;
- accurate monotonic-clock countdowns, including after the computer resumes
  from sleep.

Run the prepared portable build from:

```text
dist\DeskCat.exe
```

To preview an eight-second break:

```powershell
.\dist\DeskCat.exe --demo
```

To clear all settings and disable autostart:

```powershell
.\dist\DeskCat.exe --reset-settings
```

## Building

Requirements: Visual Studio 2022 with MSVC, CMake 3.24+, and Qt 6.8+ for
MSVC 2022 x64 with the Qt Multimedia module.

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 `
  -DCMAKE_PREFIX_PATH="C:\Qt\6.8.3\msvc2022_64" `
  -DBUILD_TESTING=ON
cmake --build build --config Release --parallel 4
ctest --test-dir build -C Release --output-on-failure
```

To prepare a portable directory after building:

```powershell
C:\Qt\6.8.3\msvc2022_64\bin\windeployqt.exe `
  --release --qmldir qml dist\DeskCat.exe
```

Sprites are stored in `assets/sprites`. Every frame uses a 640 x 640 px cell;
the player scales them for the current screen resolution without modifying the
source images.

## Windows installer and signing

The release package is built by `tools/package_windows.ps1`. It creates a
per-user Inno Setup installer, signs both `DeskCat.exe` and the installer, and
verifies their Authenticode signatures. The final wizard page contains a
checked **Launch DeskCat** option; that launch always opens the settings window.

With a certificate from the Windows certificate store:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass `
  -File .\tools\package_windows.ps1 `
  -CertificateThumbprint "CERTIFICATE_SHA1_THUMBPRINT"
```

With a PFX file (keep its password outside the repository):

```powershell
$env:DESKCAT_SIGN_PASSWORD = "PFX password"
powershell.exe -NoProfile -ExecutionPolicy Bypass `
  -File .\tools\package_windows.ps1 `
  -CertificatePath "C:\certificates\deskcat.pfx"
Remove-Item Env:\DESKCAT_SIGN_PASSWORD
```

The script requires Inno Setup 6 or 7 and `signtool.exe` from the Windows SDK.
Unsigned local packages are possible only when explicitly requested with
`-AllowUnsigned`. Output is written to `artifacts\windows`.

The uninstaller removes the application files, shortcuts, saved settings, and
the `HKCU\Software\Microsoft\Windows\CurrentVersion\Run\DeskCat` autostart
value. `DeskCat.exe --reset-settings` performs the same complete per-user state
reset and then opens the initial settings screen. `DeskCat.exe --uninstall` is
the non-interactive cleanup entry point used by the uninstaller.

## Linux packages

DeskCat is available as both Flatpak and AppImage. The Linux build retains the
system tray controls, per-user autostart, sounds, multi-monitor overlays, and
background dimming. Native and AppImage installs use the XDG autostart
directory; Flatpak uses the user-approved Background portal.

Linux binaries must be produced in a Linux environment. From Windows, the
provided wrapper builds both formats in Linux containers through Docker
Desktop:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass `
  -File .\tools\package_linux.ps1 -Format All
```

To build only one format, select it explicitly:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass `
  -File .\tools\package_linux.ps1 -Format AppImage

powershell.exe -NoProfile -ExecutionPolicy Bypass `
  -File .\tools\package_linux.ps1 -Format Flatpak
```

To build natively on Linux, install `flatpak` and `flatpak-builder`, then run:

```bash
bash packaging/linux/build-flatpak.sh
```

The packages are written to `artifacts/linux`. Install and run a local Flatpak
bundle with:

```bash
flatpak install --user artifacts/linux/DeskCat-0.1.0-x86_64.flatpak
flatpak run com.deskcat.DeskCat
```

Run the AppImage with:

```bash
chmod +x artifacts/linux/DeskCat-0.1.0-x86_64.AppImage
./artifacts/linux/DeskCat-0.1.0-x86_64.AppImage
```

On desktops without a status-tray implementation, DeskCat keeps the settings
window accessible and closing it exits the application. GNOME users who want a
tray icon need a StatusNotifier/AppIndicator shell extension. Wayland controls
window activation and stacking, so a compositor may limit whether a timed
overlay can force itself above another full-screen application; X11 and KDE
Plasma generally provide the most complete always-on-top behavior.
# PauseMeow portable for Windows

PauseMeow runs without an installer or administrator access.

## Start

1. Extract the entire ZIP into a normal user-writable directory.
2. Keep that directory in a stable location so the saved autostart path remains
   valid.
3. Run `PauseMeow.exe`.

The GitHub build is unsigned, so Windows may display a SmartScreen warning.
Review the warning normally; do not disable Windows security features.

## Update

Exit PauseMeow, extract the newer archive, and replace the application files.
Keep the directory path unchanged when possible. If the directory is moved,
launch PauseMeow once so it can repair an enabled autostart entry.

## Remove

Run `PauseMeow.exe --reset-settings`, exit PauseMeow, and delete the extracted
directory. This clears saved settings and the per-user autostart entry.

For licensing, privacy, and support information, see `LICENSE`, `PRIVACY.md`,
`THIRD_PARTY_NOTICES.md`, and the Support section in `README.md`.

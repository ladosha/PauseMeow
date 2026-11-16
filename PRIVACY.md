# Privacy

PauseMeow runs locally and does not require an account.

## Data handled by the application

PauseMeow stores only the settings needed to operate the timer, including work
and break durations, the optional break message, language, sound preferences,
display preferences, and autostart preference.

- On Windows, settings are stored for the current user through Qt's `QSettings`.
- On Linux, settings are stored in the current user's standard configuration
  directory.
- When autostart is enabled, PauseMeow creates the platform-specific per-user
  startup registration required to launch with `--background`.

## Network activity

PauseMeow does not include analytics, advertising, telemetry, crash reporting,
cloud synchronization, or application-initiated transmission of personal data.
The application does not upload the optional break message or other settings.

Build and packaging tools may download development dependencies, but those tools
are not run by the distributed application.

## Removing local data

Run `PauseMeow.exe --reset-settings` to clear settings and disable autostart
before deleting a portable Windows copy. On Linux, disabling autostart removes
PauseMeow's per-user XDG autostart entry.

This document will be updated if a future release adds any networked feature.

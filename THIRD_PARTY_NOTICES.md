# Third-party notices

PauseMeow itself is licensed under the MIT License. The following bundled
runtime components retain their own licenses.

## Qt 6

PauseMeow uses dynamically linked Qt 6.8 components, including Qt Core, GUI,
Multimedia, Network, QML, Quick, Quick Controls, Widgets, and platform/QML
plugins selected by the deployment tools.

Qt Community Edition modules used by PauseMeow are provided primarily under the
GNU Lesser General Public License version 3 (LGPL-3.0), with alternative license
options and component-specific exceptions described by the Qt project.

The Qt libraries are dynamically linked. In the Windows portable package they
can be replaced directly beside `PauseMeow.exe`. An AppImage can be extracted
with `--appimage-extract`, its libraries replaced, and the directory repacked.
The complete LGPL-3.0 text is included in `THIRD_PARTY_LICENSES`.

- Project: https://www.qt.io/
- License information: https://doc.qt.io/qt-6.8/licensing.html
- Source archives: https://download.qt.io/archive/qt/6.8/6.8.3/submodules/

PauseMeow does not modify Qt. The corresponding Qt 6.8.3 source archives are
available from the Qt source-archive link above.

## FFmpeg

The Windows portable package and Linux AppImage may include FFmpeg shared
libraries supplied with Qt Multimedia. Those libraries are separate works and
retain the FFmpeg project's applicable LGPL terms. The LGPL-2.1 and LGPL-3.0
texts are included in `THIRD_PARTY_LICENSES`.

- Project: https://ffmpeg.org/
- Legal information: https://ffmpeg.org/legal.html
- Source archives: https://ffmpeg.org/releases/

PauseMeow does not modify FFmpeg. The release packages preserve the original
shared-library filenames and allow replacement of those libraries in the same
way as the Qt libraries.

## Runtime inventory

Deployment tools include the Qt plugins and runtime libraries needed by the
application. Their upstream copyright and license notices remain in force; this
document and the included license texts supplement rather than replace them.

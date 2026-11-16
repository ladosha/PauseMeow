#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
project_root="$(cd -- "$script_dir/../.." && pwd)"
build_dir="$project_root/build-linux-appimage"
artifact_dir="$project_root/artifacts/linux"
app_dir="$artifact_dir/PauseMeow.AppDir"
qt_root="${QT_ROOT:-/opt/Qt/6.8.3/gcc_64}"
linuxdeploy="${LINUXDEPLOY:-linuxdeploy-x86_64.AppImage}"
version="$(sed -nE 's/^project\(PauseMeow VERSION ([0-9]+\.[0-9]+\.[0-9]+).*/\1/p' "$project_root/CMakeLists.txt")"

command -v cmake >/dev/null || {
    echo "cmake is required." >&2
    exit 1
}
command -v ninja >/dev/null || {
    echo "ninja is required." >&2
    exit 1
}
command -v "$linuxdeploy" >/dev/null || {
    echo "linuxdeploy was not found: $linuxdeploy" >&2
    exit 1
}

rm -rf -- "$build_dir" "$app_dir"
mkdir -p "$build_dir" "$artifact_dir"

cmake \
    -S "$project_root" \
    -B "$build_dir" \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="$qt_root" \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DBUILD_TESTING=ON
cmake --build "$build_dir" --parallel
QT_QPA_PLATFORM=offscreen ctest \
    --test-dir "$build_dir" \
    --output-on-failure
DESTDIR="$app_dir" cmake --install "$build_dir"
cp \
    "$app_dir/usr/share/metainfo/com.vladmas.PauseMeow.metainfo.xml" \
    "$app_dir/usr/share/metainfo/com.vladmas.PauseMeow.appdata.xml"

# linuxdeploy-plugin-qt does not discover Qt Multimedia backends from QML
# imports. Put the FFmpeg backend in Qt's deployed plugin tree explicitly and
# ask linuxdeploy to collect its FFmpeg and other shared-library dependencies.
multimedia_plugin="$app_dir/usr/plugins/multimedia/libffmpegmediaplugin.so"
mkdir -p "$(dirname -- "$multimedia_plugin")"
cp "$qt_root/plugins/multimedia/libffmpegmediaplugin.so" "$multimedia_plugin"

export QML_SOURCES_PATHS="$project_root/qml"
export QMAKE="$qt_root/bin/qmake"
export LD_LIBRARY_PATH="$qt_root/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export QML2_IMPORT_PATH="$qt_root/qml${QML2_IMPORT_PATH:+:$QML2_IMPORT_PATH}"
export EXTRA_PLATFORM_PLUGINS="libqwayland-egl.so;libqwayland-generic.so;libqoffscreen.so"
export LDAI_OUTPUT="PauseMeow-${version}-x86_64.AppImage"
export APPIMAGE_EXTRACT_AND_RUN=1

(
    cd "$artifact_dir"
    "$linuxdeploy" \
        --appdir "$app_dir" \
        --deploy-deps-only "$multimedia_plugin" \
        --plugin qt \
        --output appimage
)

smoke_log="$(mktemp)"
trap 'rm -f -- "$smoke_log"' EXIT
QT_QPA_PLATFORM=offscreen \
APPIMAGE_EXTRACT_AND_RUN=1 \
    "$artifact_dir/$LDAI_OUTPUT" --smoke-test 2>&1 | tee "$smoke_log"
if grep -Fq "No QtMultimedia backends found" "$smoke_log"; then
    echo "The AppImage has no Qt Multimedia backend." >&2
    exit 1
fi

echo "Created: $artifact_dir/$LDAI_OUTPUT"

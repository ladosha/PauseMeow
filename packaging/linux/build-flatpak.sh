#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
project_root="$(cd -- "$script_dir/../.." && pwd)"
manifest="$script_dir/com.deskcat.DeskCat.yml"
build_dir="$project_root/build-linux-flatpak"
repo_dir="$project_root/artifacts/linux/flatpak-repo"
artifact_dir="$project_root/artifacts/linux"
app_id="com.deskcat.DeskCat"
version="$(sed -nE 's/^project\(DeskCat VERSION ([0-9]+\.[0-9]+\.[0-9]+).*/\1/p' "$project_root/CMakeLists.txt")"
arch="$(flatpak --default-arch)"
bundle="$artifact_dir/DeskCat-${version}-${arch}.flatpak"
flatpak_scope=(--user)
builder_extra=()

if [[ "${DESKCAT_FLATPAK_SYSTEM:-0}" == "1" ]]; then
    flatpak_scope=()
    builder_extra=(--disable-rofiles-fuse)
    if [[ ! -s /etc/machine-id ]]; then
        dbus-uuidgen --ensure=/etc/machine-id
    fi
    if [[ -z "${DBUS_SESSION_BUS_ADDRESS:-}" ]]; then
        exec dbus-run-session -- bash "$0" "$@"
    fi
fi

command -v flatpak >/dev/null || {
    echo "flatpak is required." >&2
    exit 1
}
command -v flatpak-builder >/dev/null || {
    echo "flatpak-builder is required." >&2
    exit 1
}

mkdir -p "$artifact_dir"
flatpak remote-add "${flatpak_scope[@]}" --if-not-exists flathub \
    https://flathub.org/repo/flathub.flatpakrepo

flatpak-builder \
    "${flatpak_scope[@]}" \
    "${builder_extra[@]}" \
    --force-clean \
    --install-deps-from=flathub \
    --repo="$repo_dir" \
    "$build_dir" \
    "$manifest"

flatpak build-bundle \
    "$repo_dir" \
    "$bundle" \
    "$app_id" \
    master

echo "Created: $bundle"

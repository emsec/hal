#!/bin/sh
# Build and install the HAL flatpak from the current working tree (can be run from any directory).
exec flatpak-builder --user --install --force-clean --state-dir ~/.cache/hal-flatpak/state ~/.cache/hal-flatpak/build "$(dirname "$(readlink -f "$0")")/io.github.emsec.hal.yml" "$@"

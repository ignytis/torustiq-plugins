#!/usr/bin/env bash
# Symlinks the built plugin libraries into the development home of the torustiq host.
# Usage: scripts/link_dev.sh [build_dir] [torustiq_home]
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
build_dir="$(cd "${1:-$root/build}" && pwd)"
home="${2:-$root/../torustiq/dev/torustiq_home}"
plugins_dir="$home/plugins"

case "$(uname -s)" in
  Darwin) ext=dylib ;;
  MINGW* | MSYS* | CYGWIN*) ext=dll ;;
  *) ext=so ;;
esac

mkdir -p "$plugins_dir"
plugins_dir="$(cd "$plugins_dir" && pwd)"
for name in hello_input uppercase_transformer print_output; do
  lib="$build_dir/lib$name.$ext"
  [[ -f "$lib" ]] || { echo "not built: $lib" >&2; exit 1; }
  ln -sf "$lib" "$plugins_dir/lib$name.$ext"
  echo "linked $plugins_dir/lib$name.$ext -> $lib"
done

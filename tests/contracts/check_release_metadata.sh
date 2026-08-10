#!/bin/sh
# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later
set -eu
root=${1:?source root required}
fail() { echo "release-metadata: $*" >&2; exit 1; }
version=$(sed -n "s/^[[:space:]]*version: '\([^']*\)'.*/\1/p" "$root/meson.build" | head -n 1)
[ "$version" = 3.0.0 ] || fail "project version is '$version', expected 3.0.0"
grep -F '## libpkgtransaction 3.0.0' "$root/HISTORY.md" >/dev/null || fail 'HISTORY omits 3.0.0'
grep -F "soversion: '3'" "$root/src/meson.build" >/dev/null || fail 'shared library is not SONAME 3'
block() { sed -n "/^[[:space:]]*'$1',[[:space:]]*$/,/^[[:space:]]*)/p" "$root/meson.build"; }
for spec in 'libpkgsource >=3.0.1 <4.0.0' 'libpkgresolve >=3.0.0 <4.0.0' 'libpkgstate >=3.1.0 <4.0.0'; do
  set -- $spec; dep=$1; lo=$2; hi=$3; b=$(block "$dep");
  printf '%s\n' "$b" | grep -F "'$lo'" >/dev/null || fail "$dep omits $lo"
  printf '%s\n' "$b" | grep -F "'$hi'" >/dev/null || fail "$dep omits $hi"
done
grep -F 'requires: [' "$root/src/meson.build" >/dev/null || fail 'pkg-config requirements are not dependency-object backed'

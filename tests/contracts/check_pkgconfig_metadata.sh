#!/bin/sh
# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later
set -eu
build_root=${1:?build root required}
metadata=$build_root/meson-private/libpkgtransaction.pc
fail() {
  echo "metadata-test: $*" >&2
  if [ -s "${metadata:-}" ]; then echo '--- generated metadata ---' >&2; cat "$metadata" >&2; echo '--- end generated metadata ---' >&2; fi
  exit 1
}
if [ ! -s "$metadata" ]; then metadata=$(find "$build_root" -type f -name libpkgtransaction.pc -print | sed -n '1p'); fi
[ -n "${metadata:-}" ] && [ -s "$metadata" ] || fail 'generated libpkgtransaction.pc was not found'
[ "$(sed -n 's/^Name:[[:space:]]*//p' "$metadata")" = 'libpkgtransaction' ] || fail 'wrong module name'
[ "$(sed -n 's/^Version:[[:space:]]*//p' "$metadata")" = '4.0.0' ] || fail 'wrong module version'
normalize() { sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//' -e 's/[[:space:]][[:space:]]*/ /g' -e 's/ *\([<>]=\|[<>=]\) */ \1 /' -e '/^$/d'; }
requires=$(sed -n 's/^Requires:[[:space:]]*//p' "$metadata" | tr ',' '\n' | normalize)
expected='libpkgsource >= 4.0.0
libpkgsource < 5.0.0
libpkgresolve >= 4.0.0
libpkgresolve < 5.0.0
libpkgstate >= 3.1.0
libpkgstate < 4.0.0'
for requirement in \
  'libpkgsource >= 4.0.0' 'libpkgsource < 5.0.0' \
  'libpkgresolve >= 4.0.0' 'libpkgresolve < 5.0.0' \
  'libpkgstate >= 3.1.0' 'libpkgstate < 4.0.0'; do
  count=$(printf '%s\n' "$requires" | grep -Fxc "$requirement" || true)
  [ "$count" -eq 1 ] || fail "metadata contains $count copies of '$requirement', expected exactly one"
done
[ "$(printf '%s\n' "$requires" | LC_ALL=C sort)" = "$(printf '%s\n' "$expected" | LC_ALL=C sort)" ] || fail 'public requirements are not the exact transaction dependency intervals'
private=$(sed -n 's/^Requires\.private:[[:space:]]*//p' "$metadata" | tr ',' '\n' | normalize)
[ "$private" = libcrypto ] || fail "private requirements are '$private', expected libcrypto"
printf ' %s \n' "$(sed -n 's/^Libs:[[:space:]]*//p' "$metadata")" | grep -F ' -lpkgtransaction ' >/dev/null || fail 'metadata omits -lpkgtransaction'

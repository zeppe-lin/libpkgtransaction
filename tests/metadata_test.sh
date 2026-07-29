#!/bin/sh
# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later
set -eu

build_root=${1:?build root required}
pc=$(find "$build_root" -name libpkgtransaction.pc -type f -print -quit)
if [ -z "$pc" ]; then
  echo 'metadata-test: libpkgtransaction.pc not found' >&2
  exit 1
fi
fail()
{
  echo "metadata-test: $1" >&2
  echo '--- generated metadata ---' >&2
  cat "$pc" >&2
  exit 1
}
grep -Eq '^Name:[[:space:]]+libpkgtransaction$' "$pc" || fail 'wrong module name'
grep -Eq '^Version:[[:space:]]+[0-9]+\.[0-9]+\.[0-9]+$' "$pc" || fail 'missing version'
grep -Eq '^Libs:.*-lpkgtransaction([[:space:]]|$)' "$pc" || fail 'missing transaction library'
grep -Eq '(^|[[:space:],])libpkgresolve[[:space:]]*>=[[:space:]]*1\.0\.0([[:space:],]|$)' "$pc" ||
  fail 'missing resolver authority floor'
grep -Eq '(^|[[:space:],])libpkgsource[[:space:]]*>=[[:space:]]*2\.0\.0([[:space:],]|$)' "$pc" ||
  fail 'missing check-program source authority floor'
# libpkgstate may be emitted directly, privately, or de-duplicated through the
# public libpkgresolve closure. The source contract separately pins it exactly.

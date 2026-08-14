#!/bin/sh
# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later
set -eu
root=${1:?source root required}
fail() { echo "ci-contract: $*" >&2; exit 1; }
workflow=$root/.github/workflows/ci.yml
driver=$root/ci/configure-and-test.sh
[ -s "$workflow" ] || fail 'hosted CI workflow is absent'
[ -x "$driver" ] || fail 'qualification driver is absent or not executable'
for mode in 'GCC shared' 'GCC static' 'Clang shared' 'Clang static' 'GCC release'; do grep -F "$mode" "$workflow" >/dev/null || fail "CI omits $mode"; done
grep -F 'address,undefined' "$workflow" >/dev/null || fail 'CI omits ASan/UBSan qualification'
[ "$(grep -c 'repository: zeppe-lin/libpkgsource, ref: v4.1.0' "$workflow")" -eq 2 ] || fail 'CI omits source-4.1 tag in one matrix'
[ "$(grep -c 'repository: zeppe-lin/libpkgcatalog, ref: v4.0.0' "$workflow")" -eq 2 ] || fail 'CI omits catalog-4 tag in one matrix'
[ "$(grep -c 'repository: zeppe-lin/libpkgresolve, ref: v4.0.0' "$workflow")" -eq 2 ] || fail 'CI omits resolver-4 tag in one matrix'
grep -F 'f74df278b47b48e798c3de01c922c59b58319d13' "$workflow" >/dev/null || fail 'CI omits current state authority pin'
! grep -F 'ref: master' "$workflow" >/dev/null || fail 'floating resolver authority remains'
! grep -F '9e030b49606d5ac78bb1ee0c868681a37d0726cc' "$workflow" >/dev/null || fail 'obsolete resolver tree pin remains'
grep -F 'pkg-config --static --libs libpkgtransaction' "$driver" >/dev/null || fail 'static installed consumer does not use pkg-config --static'
grep -F 'tests/installed/consumer.cpp' "$driver" >/dev/null || fail 'installed consumer is not executed'
grep -F 'LIBPKGRESOLVE_SOURCE' "$driver" >/dev/null || fail 'resolver source is not built in isolated qualification prefix'

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
for pin in d5f30663a4e56c2319f301ca762741106dea1bd0 16976cac176f576871e327d5d2f6fe9d9dfa0666 f74df278b47b48e798c3de01c922c59b58319d13 9e030b49606d5ac78bb1ee0c868681a37d0726cc; do grep -F "$pin" "$workflow" >/dev/null || fail "CI omits authority pin/tree $pin"; done
grep -F 'pkg-config --static --libs libpkgtransaction' "$driver" >/dev/null || fail 'static installed consumer does not use pkg-config --static'
grep -F 'tests/installed/consumer.cpp' "$driver" >/dev/null || fail 'installed consumer is not executed'
grep -F 'LIBPKGRESOLVE_SOURCE' "$driver" >/dev/null || fail 'resolver source is not built in isolated qualification prefix'

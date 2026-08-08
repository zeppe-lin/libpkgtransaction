#!/bin/sh
# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later
set -eu
root=${1:?source root required}
meson=$root/tests/meson.build

for directory in unit integration fixtures support contracts; do
  [ -d "$root/tests/$directory" ] || {
    echo "test-layout: missing qualification role: $directory" >&2
    exit 1
  }
done
for escaped in "$root"/tests/*.cpp "$root"/tests/*.h "$root"/tests/check_*.sh; do
  [ ! -e "$escaped" ] || {
    echo "test-layout: uncategorized test source: $escaped" >&2
    exit 1
  }
done
for suite in unit integration header contract; do
  grep -F "suite: '$suite'" "$meson" >/dev/null || {
    echo "test-layout: Meson omits $suite suite" >&2
    exit 1
  }
done
for path in \
  unit/model_test.cpp \
  integration/classification_test.cpp \
  integration/lifecycle_test.cpp \
  integration/construction_ordering_test.cpp \
  integration/lifecycle_ordering_test.cpp \
  integration/adversarial_result_test.cpp \
  contracts/check_test_layout.sh; do
  grep -F "$path" "$meson" "$root/TESTING.md" >/dev/null || {
    echo "test-layout: qualification wiring omits $path" >&2
    exit 1
  }
done
for support_file in \
  "$root/tests/fixtures/source_state.h" \
  "$root/tests/fixtures/transaction.h" \
  "$root/tests/support/test.h" \
  "$root/tests/support/program_query.h"; do
  [ -s "$support_file" ] || {
    echo "test-layout: missing categorized support material: $support_file" >&2
    exit 1
  }
done

#!/bin/sh
# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later
set -eu
root=${1:?source root required}

if grep -R -E '#include <(filesystem|archive|yaml|libpkgplan|libpkgapply|libpkgbuild)' \
    "$root/include" "$root/src" >/dev/null; then
  echo 'authority-contract: forbidden acquisition/planning/execution dependency' >&2
  exit 1
fi
if grep -R -E 'Pkgfile|fakeroot|pkgman\.conf|/var/lib/pkg/db|build_and_run' \
    "$root/include" "$root/src" >/dev/null; then
  echo 'authority-contract: historical compatibility entered native code' >&2
  exit 1
fi
grep -q "'libpkgresolve'" "$root/meson.build"
grep -q "version: '>=1.0.0'" "$root/meson.build"
grep -q "'libpkgsource'" "$root/meson.build"
grep -q "version: '>=2.0.0'" "$root/meson.build"
grep -q "'libpkgstate'" "$root/meson.build"
grep -q "version: '>=2.1.0'" "$root/meson.build"
grep -q 'target_upgrade_node' "$root/src/composer.cpp"
grep -q 'remove or upgrade action' "$root/src/composer.cpp"
grep -q 'old_pre_remove' "$root/tests/composer_test.cpp"
grep -q 'incoming_post_install' "$root/tests/composer_test.cpp"
grep -q 'require_check_authority' "$root/src/composer.cpp"
grep -q 'missing_check_program' "$root/tests/check_program_test.cpp"

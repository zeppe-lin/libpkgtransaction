#!/bin/sh
# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later
set -eu
root=${1:?source root required}
fail() { echo "abi-contract: $*" >&2; exit 1; }
manifest=$root/abi/libpkgtransaction.exports
[ -s "$manifest" ] || fail 'reviewed ELF ABI manifest is absent'
[ "$(sed -n '/^_Z[A-Za-z0-9_]*$/p' "$manifest" | wc -l)" -eq 78 ] || fail 'reviewed ELF ABI manifest must contain exactly 78 symbols'
[ "$(LC_ALL=C sort -u "$manifest" | wc -l)" -eq 78 ] || fail 'reviewed ELF ABI manifest contains duplicate symbols'
! grep -E '^_ZNSt|^_ZN9__gnu_cxx' "$manifest" >/dev/null || fail 'standard-library implementation symbol entered ABI manifest'
demangled=$(mktemp)
trap 'rm -f "$demangled"' EXIT HUP INT TERM
c++filt < "$manifest" > "$demangled"
for private in \
  'convergence_policy::convergence_policy(' \
  'transaction_request::transaction_request(' \
  'transaction_node::transaction_node(' \
  'transaction_edge::transaction_edge(' \
  'transaction_edge::requirement(' \
  'transaction_edge::phase(' \
  'runtime_cohort::runtime_cohort(' \
  'transaction_program::transaction_program('; do
  ! grep -F "pkgtransaction::$private" "$demangled" >/dev/null || fail "private $private entered public ABI manifest"
done
for identity in transaction_request_identity transaction_node_identity transaction_edge_identity runtime_cohort_identity transaction_program_identity; do
  ! grep -F "pkgtransaction::$identity::$identity(std::__cxx11::basic_string" "$demangled" >/dev/null || fail "private $identity constructor entered public ABI manifest"
done
grep -F 'pkgtransaction::compose(pkgtransaction::transaction_request)' "$demangled" >/dev/null || fail 'compose is absent from reviewed ABI'
grep -F 'typeinfo for pkgtransaction::error' "$demangled" >/dev/null || fail 'public error RTTI is absent from reviewed ABI'
grep -F "soversion: '3'" "$root/src/meson.build" >/dev/null || fail 'SONAME generation is not 3'
grep -F -- '--version-script=' "$root/src/meson.build" >/dev/null || fail 'reviewed ELF export manifest is not linked'

# Testing

`libpkgtransaction` qualification is separated by evidence role. A large
composer test is not treated as equivalent to several independently diagnosable
contract and behavior boundaries.

## Qualification roles

- `tests/unit/` covers transaction-owned value semantics that do not require a
  real resolver pass: identity validation, ordering, canonical policy values,
  enum spellings, and policy refusal.
- `tests/integration/` feeds the transaction boundary with genuine
  `libpkgresolve` results built from deterministic source/catalog/state fiction.
  These tests prove classification, ordering, lifecycle binding, runtime-cycle
  projection, request admission, public program views, exact check authority,
  and fail-closed handling at the resolver/transaction seam.
- `tests/fixtures/` constructs deterministic authority through public
  `libpkgsource`, `libpkgcatalog`, `libpkgstate`, and `libpkgresolve` APIs. The
  fixtures do not replace those dependencies with transaction-local mocks.
- `tests/support/` contains assertions and graph queries only. Support code must
  not manufacture authority or encode production policy.
- `tests/contracts/` checks dependency direction, release/pkg-config metadata,
  and the qualification topology itself.
- the `header` suite compiles every installed public header as a standalone
  translation unit rather than proving only the umbrella header.

This division is deliberate. Caller-side execution of transaction programs
belongs in callers such as `pkgctl`; adding an upward dependency from
`libpkgtransaction` to an orchestrator would invert the production boundary.
The library instead qualifies its callee seam with the real resolver, while
callers must independently qualify their consumption of transaction authority.

## Behavioral matrix

The native suite proves:

- all five transaction identity types accept only canonical lowercase SHA-256
  values and retain comparison semantics;
- convergence-policy normalization, empty/duplicate refusal, stable ordering,
  explicit-removal admission, unknown-installed refusal, and selected/removal
  conflicts, including lifecycle-selected dependencies;
- build, install, upgrade, retain, remove, check, and lifecycle classification;
- build/check-only goals do not accidentally create target installation work;
- exact installed release plus source-snapshot authority retains the package,
  while either release drift or source-snapshot drift requires upgrade;
- build-scoped requirements precede construction, including installed
  build-environment retention;
- check-scoped requirements precede the build whose result later supplies the
  check input, with no redundant direct requirement edge into the check node;
- runtime requirements retain their exact witnesses and point from the required
  completion node to the issuer completion node;
- lifecycle-scoped requirements precede the exact lifecycle node while
  package-local phase order remains distinct from requirement order;
- incoming install lifecycle and historical remove lifecycle bind around one
  upgrade action under their respective authorities;
- removal lifecycle is refused without a remove/upgrade action;
- runtime cycles become sorted cohorts with retained witnesses and no fabricated
  internal precedence, while construction and lifecycle cycles are refused;
- exact check-program authority is retained and missing or forged check source
  authority is refused;
- program nodes and edges are canonical-identity sorted, `find()` resolves every
  retained node, `nodes_for()` is package-exact, edge evidence matches edge kind,
  and equivalent input reproduces the same program identity;
- a resolved goal that names an unknown selection is refused instead of being
  silently projected; and
- every public header is self-contained under the declared dependency closure.

## Running

Run all evidence roles:

```sh
meson test -C build --print-errorlogs
```

Run one role while diagnosing a failure:

```sh
meson test -C build --suite unit --print-errorlogs
meson test -C build --suite integration --print-errorlogs
meson test -C build --suite header --print-errorlogs
meson test -C build --suite contract --print-errorlogs
```

Release qualification requires separate GCC/Clang shared and static build directories,
GCC and Clang ASan+UBSan builds, and execution of the installed pkg-config
consumer. Shared qualification additionally proves the exact 78-symbol ABI
surface and direct `libpkgsource.so.4`, `libpkgcatalog.so.3`,
`libpkgresolve.so.3`, and `libpkgstate.so.4` edges. The x86-64 ABI-layout
contract pins both the foreign by-value values and transaction carriers that
retain them. A passing unit or source-contract subset is not release proof for
the resolver/state-to-transaction composition seam.

The hosted qualification driver builds the dependency closure into an isolated
prefix before configuring this repository. It does not reuse a host-installed
resolver or state library. The installed consumer then resolves one real catalog
candidate, seals a transaction request, composes a program, and catches a public
`pkgtransaction::error` across the installed DSO boundary.

The categorized sources are intentionally pinned by
`tests/contracts/check_test_layout.sh`; new test code should enter the role that
matches the evidence it proves rather than accumulate again in `tests/` root.

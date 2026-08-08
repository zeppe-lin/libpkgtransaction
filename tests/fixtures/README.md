# Test fixtures

The fixture headers construct deterministic fiction through the real public
`libpkgsource`, `libpkgcatalog`, `libpkgstate`, and `libpkgresolve` APIs. They do
not replace those libraries with mocks.

`source_state.h` owns reusable source/catalog/installed-state fiction.
`transaction.h` adds real resolver request/result construction used by the
transaction integration suite.

Tests should keep assertions and graph queries in `tests/support/`; fixture code
establishes authority but does not decide expected transaction semantics.

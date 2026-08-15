# libpkgtransaction

`libpkgtransaction` is the native Zeppe-Lin cross-package transaction authority.

It consumes one sealed `libpkgresolve::resolution_result` plus explicit
convergence policy and produces an immutable package-operation program. The
program classifies build, check, install, upgrade, retain, remove, and
lifecycle work; admits check work only when the exact selected source snapshot
carries a check program; binds installed removal lifecycle and incoming
installation lifecycle around one exact upgrade action; orders build input
authority before construction and check input authority before check; retains
typed ordering witnesses; represents runtime cycles as explicit cohorts; and
seals the complete graph under a domain-separated identity.

The library does not discover collections, resolve dependencies, inspect
archives, construct `libpkgplan` requests, execute programs, mutate filesystems,
or publish installed state.

## Authority chain

```text
libpkgcatalog      available package authority
libpkgsource       exact build, check, and lifecycle program authority
libpkgstate        installed package authority
libpkgresolve      exact selections and typed closures
libpkgtransaction cross-package actions and partial order
pkgctl             obtains artifacts and observations
libpkgplan         plans one exact package transition
libpkgapply        applies one exact package transition
```

A transaction program is not a promise of transaction-wide atomicity. It is an
authoritative operation graph for later orchestration.

## Build

```sh
meson setup build
meson compile -C build
meson test -C build --print-errorlogs
```

Shared and static libraries use separate build directories. Use
`-Ddefault_library=static -Dlink_mode=static` for a static build.

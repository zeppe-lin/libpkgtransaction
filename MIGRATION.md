# Migration

`libpkgtransaction` has no historical transaction format or compatibility API.

The library consumes native `libpkgresolve` results only. Existing pkgman action
lists, shell command queues, package-directory traversal order, and inferred
removal behavior are not accepted as native authority.

A future compatibility frontend may translate an explicitly captured legacy
intent into a native resolution request and convergence policy. Compatibility
logic must remain outside this library.

## 1.1.0

No API, ABI, or stored-format migration is required. Rebuild consumers against
the new release to obtain complete upgrade lifecycle phase edges. Programs that
previously attempted to compose installed `pre_remove` or `post_remove` goals
alongside an upgrade were rejected as unbound; they are now represented around
the exact upgrade action while retaining historical installed authority.

## 2.0.0

Install `libpkgsource.so.2`, `libpkgcatalog.so.2`, and `libpkgresolve.so.2`, then
rebuild consumers against `libpkgtransaction.so.2`. `transaction_node` retains
resolver selections by value, so the source/catalog/resolve ABI transition is
part of its public layout. No transaction request or identity migration is
required. Explicit check goals now require exact source check-program authority.

## 2.1.0

No API or ABI migration is required. Rebuild transaction consumers and do not
reuse persisted transaction-program identities for check goals: check-scoped
requirements now precede the issuer build node, so affected edge and program
identities change. This closes the ordering required by the existing build and
check authority contracts; it does not add check execution.

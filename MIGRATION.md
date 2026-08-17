# Migration

`libpkgtransaction` has no historical transaction format or compatibility API.

The library consumes native `libpkgresolve` results only. Existing pkgman action
lists, shell command queues, package-directory traversal order, and inferred
removal behavior are not accepted as native authority.

A future compatibility frontend may translate an explicitly captured legacy
intent into a native resolution request and convergence policy. Compatibility
logic must remain outside this library.

## Unreleased

No API or ABI migration is required. Rebuild transaction consumers and do not
reuse persisted transaction-program identities produced by the previous graph
semantics. Check-scoped requirements now target the exact check node rather than
issuer construction. Runtime requirements crossing a cyclic runtime component
are projected over the complete cohort condensation boundary. Both corrections
change affected edge and program identities without changing identity domains.
Build requests may still retain logical check-input authority; callers realize
those inputs independently for the check phase.

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


## 3.0.0

For this release, install `libpkgsource.so.3`,
`libpkgcatalog.so.3`, `libpkgresolve.so.3`, and `libpkgstate.so.4`) and rebuild
all transaction consumers against `libpkgtransaction.so.3`. The transaction 2
ABI cannot be widened to these providers: `transaction_authority` and
`transaction_node` retain resolver/state values by value and their layouts
changed. No compatibility shim or cross-generation carrier is provided. The
transaction identity domains and graph semantics are unchanged.

## 4.0.0

Install `libpkgsource.so.4`, `libpkgcatalog.so.4`, and `libpkgresolve.so.4`, then
rebuild transaction consumers against `libpkgtransaction.so.4`. The resolver-4
carrier embeds the source-realization/catalog-4 authority transition, and
`transaction_node` retains resolver selections by value. No source-3,
catalog-3, or resolver-3 carrier is translated into the new transaction ABI.
Transaction identity domains and graph semantics are unchanged.

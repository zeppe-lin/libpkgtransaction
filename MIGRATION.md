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

## 1.2.0

No `libpkgtransaction` value layout or stored-format migration is required.
The library now links directly against `libpkgsource >= 2.0.0` and rejects an
explicit check goal unless the selected catalog source carries an exact check
program. Consumers should rebuild against the new source-authority closure.
Programs created for non-check transactions retain their existing model and
identity domains.

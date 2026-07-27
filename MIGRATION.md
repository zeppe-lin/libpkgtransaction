# Migration

`libpkgtransaction` has no historical transaction format or compatibility API.

The library consumes native `libpkgresolve` results only. Existing pkgman action
lists, shell command queues, package-directory traversal order, and inferred
removal behavior are not accepted as native authority.

A future compatibility frontend may translate an explicitly captured legacy
intent into a native resolution request and convergence policy. Compatibility
logic must remain outside this library.

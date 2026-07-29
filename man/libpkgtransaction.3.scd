libpkgtransaction(3)

# NAME

libpkgtransaction - native cross-package transaction authority

# SYNOPSIS

*#include <libpkgtransaction/libpkgtransaction.h>*

# DESCRIPTION

*libpkgtransaction* consumes one sealed resolution result and explicit
convergence policy. It returns an immutable program of package-level actions,
typed ordering edges, runtime cohorts, and a complete program identity.

Explicit check work is admitted only when the exact selected catalog source
snapshot carries a check program. The program bytes remain source authority;
the transaction graph binds them through the resolver selection.

# BOUNDARY

The library does not resolve dependencies, inspect artifacts, construct
package-local filesystem plans, execute actions, mutate filesystems, publish
state, or promise transaction-wide atomicity.

# ERRORS

Contract and composition failures throw *pkgtransaction::error*. Branch on
*error_code*, not diagnostic text.

# SEE ALSO

*pkgtransaction_request*(3), *pkgtransaction_program*(3),
*pkgtransaction_semantics*(7), *libpkgsource*(3), *libpkgresolve*(3),
*libpkgplan*(3)

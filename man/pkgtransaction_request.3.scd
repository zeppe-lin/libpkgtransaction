pkgtransaction_request(3)

# NAME

pkgtransaction_request - sealed transaction composition request

# DESCRIPTION

*transaction_request::seal()* binds one exact resolution result to one explicit
convergence policy.

*preserve_unselected* infers no removals. *remove_explicit* names exact installed
packages to remove. *converge_exact* treats the complete desired target
selection set as authoritative and removes every unselected installed package.

Explicit removals reject duplicate names, unknown installed packages, and
packages simultaneously selected for non-removal target work.

# SEE ALSO

*libpkgtransaction*(3), *pkgtransaction_program*(3),
*pkgtransaction_semantics*(7)

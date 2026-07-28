pkgtransaction_program(3)

# NAME

pkgtransaction_program - immutable package-operation graph

# DESCRIPTION

*transaction_program* retains normalized operation nodes, ordering edges,
runtime cohorts, the exact request, and a domain-separated identity.

Nodes distinguish build, check, install, upgrade, retain, remove, and exact
lifecycle actions. Requirement edges retain resolver witnesses. Phase edges
retain package-local phase relationships, including installed removal and
incoming installation lifecycle around one upgrade action.

A runtime cohort retains a strongly connected runtime group and its internal
requirement witnesses without claiming an order among its members.

# SEE ALSO

*libpkgtransaction*(3), *pkgtransaction_request*(3),
*pkgtransaction_semantics*(7)

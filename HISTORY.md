# History

## libpkgtransaction 2.0.0

Exact check-program authority and the required ABI rebuild.

- advances `libpkgtransaction` to SONAME 2 because `transaction_node` retains
  `pkgresolve::selected_package` by value;
- requires `libpkgsource >= 2.0.0` and `libpkgresolve >= 2.0.0`;
- creates check nodes only for explicit check-goal members whose selected
  catalog source snapshot carries a check program;
- validates that the selected package, release, and source snapshot agree with
  the retained catalog candidate before admitting check work;
- exposes the exact non-executed source program through
  `transaction_node::check_program()`;
- rejects missing check programs and forged source bindings with typed errors;
- binds changed check-program bytes transitively into request, node, and
  program identities through source-snapshot authority;
- preserves transaction semantics and identity domains while requiring a
  generation-2 source/catalog/resolve binary closure.

## libpkgtransaction 1.1.0

Upgrade lifecycle authority correction.

- binds exact installed `pre_remove` and `post_remove` lifecycle nodes to the
  upgrade action for the same target package;
- keeps incoming `pre_install` and `post_install` nodes bound to that same
  upgrade action under catalog-candidate authority;
- retains historical installed authority for removal lifecycle material;
- emits all four phase edges without inventing order among pre-actions or
  among post-actions;
- rejects a contradictory program containing both remove and upgrade actions
  for one installed package;
- preserves the public API, identity domains, and `libpkgtransaction.so.1`.

## libpkgtransaction 1.0.0

Initial native cross-package transaction authority.

- seals exact resolution results with explicit convergence policy;
- distinguishes preservation, explicit removal, and exact convergence;
- classifies build, check, install, upgrade, retain, remove, and lifecycle work;
- prevents build/check-only goals from implying target installation;
- retains source and installed authority through resolver selections;
- projects typed requirement witnesses into ordering edges;
- retains package-local phase ordering separately from dependency ordering;
- represents runtime cycles as explicit cohorts without fake precedence;
- refuses construction and lifecycle cycles that cannot form a partial order;
- publishes domain-separated request, node, edge, cohort, and program identities;
- deliberately excludes discovery, resolution, artifact inspection, planning,
  execution, state mutation, and historical compatibility.

# History

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

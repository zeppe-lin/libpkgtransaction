pkgtransaction_semantics(7)

# NAME

pkgtransaction_semantics - native action and ordering rules

# CLASSIFICATION

Build- and check-only goals do not imply target installation. Run and lifecycle
work may require target install, upgrade, or retain actions. Exact installed
authority is retained without reconstruction.

Removal is controlled only by explicit convergence policy. Absence from an
additive resolution result never implies removal.

# ORDERING

Requirement edges preserve typed resolver scopes. Phase edges order build,
check, package action, and explicitly requested lifecycle nodes. A lifecycle
node without its matching install, upgrade, or remove action is rejected.
Storage order is deterministic but is not execution precedence.

# CYCLES

Runtime cycles become explicit cohorts. Build/check cycles among catalog
construction authorities and lifecycle-requirement cycles are refused.

# AUTHORITY

The program is an immutable cross-package operation graph. It is not a
filesystem plan, execution log, rollback receipt, or installed-state update.

# SEE ALSO

*libpkgtransaction*(3), *libpkgresolve*(3), *libpkgplan*(3), *libpkgapply*(3)

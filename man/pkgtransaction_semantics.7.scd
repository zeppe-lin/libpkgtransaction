pkgtransaction_semantics(7)

# NAME

pkgtransaction_semantics - native action and ordering rules

# CLASSIFICATION

Build- and check-only goals do not imply target installation. Run and lifecycle
work may require target install, upgrade, or retain actions. Exact installed
authority is retained without reconstruction.

An explicit check goal requires catalog-candidate authority whose exact source
snapshot contains a check program. Missing programs and selection-to-candidate
source mismatches are composition errors. Declared check requirements alone do
not constitute executable check authority.

Removal is controlled only by explicit convergence policy. Absence from an
additive resolution result never implies removal.

# ORDERING

Requirement edges preserve typed resolver scopes. Build- and check-scoped
package requirements both precede the issuer build node because the sealed
build result retains both input classes. The check node inherits exact check
inputs through the build-before-check phase edge; no redundant requirement edge
is emitted directly to the check node.

Phase edges order build, check, package action, and explicitly requested
lifecycle nodes. A lifecycle node without its matching install, upgrade, or
remove action is rejected.

For an upgrade, installed *pre_remove* and incoming *pre_install* nodes precede
the single upgrade action. Installed *post_remove* and incoming *post_install*
nodes follow it. Installed removal nodes retain historical installed authority;
incoming installation nodes retain catalog-candidate authority. No order is
inferred within either side of the action boundary.

Storage order is deterministic but is not execution precedence.

# CYCLES

Runtime cycles become explicit cohorts. Build/check cycles among catalog
construction authorities and lifecycle-requirement cycles are refused.

# AUTHORITY

The program is an immutable cross-package operation graph. It is not a
filesystem plan, execution log, rollback receipt, or installed-state update.
It does not define a check environment or claim that any check was executed.

# SEE ALSO

*libpkgtransaction*(3), *libpkgsource*(3), *libpkgresolve*(3),
*libpkgplan*(3), *libpkgapply*(3)

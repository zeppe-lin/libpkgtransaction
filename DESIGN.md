# Native package-transaction authority

## Scope

`libpkgtransaction` answers one question:

> Given one sealed resolution result and explicit convergence policy, which
> package-level actions are required, and what partial order constrains them?

The answer is an immutable `transaction_program`. The library neither performs
those actions nor converts them into package-local filesystem plans.

## Request authority

A sealed `transaction_request` retains:

- one exact `libpkgresolve::resolution_result`;
- one explicit convergence policy;
- normalized explicit-removal package references where applicable;
- a domain-separated request identity.

Three convergence modes are native:

- `preserve_unselected` never infers removal;
- `remove_explicit` removes only the named installed packages;
- `converge_exact` removes installed packages absent from the complete desired
  target selection set.

An additive install request must not use `converge_exact` accidentally. The
caller is responsible for choosing policy according to command intent.

## Action classification

Catalog authority produces a build action only when construction is actually
needed: for build-environment selections, explicit build/check work, or a target
install/upgrade. A build- or check-only target goal does not imply installation.

Target selections are classified as:

- `install` when desired target authority is absent;
- `upgrade` when a different installed release or source snapshot exists;
- `retain` when exact compatible installed authority already exists.

Installed build-environment selections become retain actions. Installed target
selections become retain actions when they satisfy run or lifecycle work.
Removal-only lifecycle roots do not become retain actions.

Check nodes are created only for explicit check-goal members. The selected
catalog candidate must agree exactly with the resolver selection's package,
release, and source-snapshot identity, and that exact source snapshot must
carry a check program. A check goal without a program is rejected rather than
silently omitted or represented as executable work. `transaction_node::
check_program()` exposes the retained non-executed program only for check
nodes.

Lifecycle nodes are created only for explicit lifecycle-goal members; the
library does not invent lifecycle execution merely because a package declares
a program. Every lifecycle node must bind to the corresponding install,
upgrade, or remove node; orphan lifecycle work is rejected. During upgrade,
incoming `pre_install` and `post_install` nodes retain catalog-candidate
authority, while installed `pre_remove` and `post_remove` nodes retain the
exact historical installed authority. All four bind to the single upgrade
action for that package.

## Ordering

Requirement edges retain the exact resolver requirement-edge identity and
scope. Build- and check-scoped package requirements both precede the issuer's
build node. This is an authority rule, not an execution convenience: the sealed
`libpkgbuild::build_request` retains both input classes, and the later check
request projects its exact check-input set from the successful build result. A
check node therefore inherits those inputs through its own build-before-check
phase edge rather than receiving a second direct requirement edge.

Phase edges distinguish:

- build before check;
- build before install or upgrade;
- check before install or upgrade;
- pre-action lifecycle before the package action;
- package action before post-action lifecycle.

An upgrade therefore has one action boundary: old installed `pre_remove` and
incoming `pre_install` precede it; old installed `post_remove` and incoming
`post_install` follow it. These edges constrain phase order without choosing an
order between the two pre-actions or between the two post-actions.

The graph is a partial order. Deterministic storage order is not execution
precedence.

## Cycles

Runtime requirement cycles are collapsed into explicit `runtime_cohort` values.
Internal runtime witnesses remain retained by the cohort and are not emitted as
cyclic ordering edges.

Build/check cycles among catalog authorities are refused. An installed
selection breaks construction because no construction node is required for that
selection. Lifecycle-requirement cycles are refused.

## Identity domains

Version 1 defines separate SHA-256 domains for:

- transaction requests;
- operation nodes;
- ordering edges;
- runtime cohorts;
- complete transaction programs.

Program identity binds the exact request and normalized node, edge, and cohort
identities. Check-program bytes are bound transitively through the selected
source-snapshot identity; they are not copied into a second transaction-owned
program representation.

## Deliberate omissions

Version 1 has no:

- collection or recipe discovery;
- dependency resolution;
- version comparison or alternative-provider policy;
- artifact acquisition, cache lookup, or image inspection;
- package-local filesystem planning;
- build, lifecycle, or application execution;
- check environment construction or check-result evidence;
- state mutation or publication;
- transaction-wide rollback or atomicity claim;
- historical pkgman compatibility.

## Version 2 ABI boundary

`transaction_node` retains `pkgresolve::selected_package` by value. Resolver
2 embeds the catalog/source ABI transition, so transaction node layout is not
compatible with SONAME 1 even though the check-program accessor adds no field
of its own. Version 2 forbids a mixed source/catalog/resolve authority closure
and preserves transaction identities and graph semantics.

## Version 4 ABI boundary

`transaction_node` retains `pkgresolve::selected_package` by value. Resolver 4
retains catalog-4/source-4 authority, so the resolver carrier transition is a
transaction carrier transition even when the visible transaction graph and
identity domains do not change.

Version 4 advances the transaction SONAME to `libpkgtransaction.so.4`, requires
`libpkgresolve >= 4.0.0, < 5.0.0`, and rejects resolver-3/catalog-3 carriers.
No compatibility reconstruction is performed.

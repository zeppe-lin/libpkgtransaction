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

Check nodes are created only for explicit check-goal members. Lifecycle nodes
are created only for explicit lifecycle-goal members; the library does not
invent lifecycle execution merely because a package declares a program. Every
lifecycle node must bind to the corresponding install, upgrade, or remove node;
orphan lifecycle work is rejected.

## Ordering

Requirement edges retain the exact resolver requirement-edge identity and
scope. Phase edges distinguish:

- build before check;
- build before install or upgrade;
- check before install or upgrade;
- pre-action lifecycle before the package action;
- package action before post-action lifecycle.

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
identities.

## Deliberate omissions

Version 1 has no:

- collection or recipe discovery;
- dependency resolution;
- version comparison or alternative-provider policy;
- artifact acquisition, cache lookup, or image inspection;
- package-local filesystem planning;
- build, lifecycle, or application execution;
- state mutation or publication;
- transaction-wide rollback or atomicity claim;
- historical pkgman compatibility.

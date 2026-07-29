# Testing

The native suite covers:

- identity validation and domain separation;
- convergence-policy normalization and duplicate rejection;
- explicit-removal admission and selected/removal conflicts;
- build, check, install, upgrade, retain, remove, and lifecycle classification;
- build/check-only goals without accidental target installation;
- exact source-program retention on explicit check nodes;
- refusal of check goals without a source check program;
- refusal of forged check selections whose source identity differs from their
  retained catalog candidate;
- check-program identity propagation into request, node, and program identity;
- exact convergence versus preservation of unselected packages;
- phase and requirement ordering;
- complete upgrade ordering with installed remove and incoming install
  lifecycle authority around one upgrade action;
- rejection of lifecycle work without its matching package action;
- distinction between removal roots and lifecycle dependency packages;
- runtime-cycle cohorts without cyclic ordering edges;
- construction- and lifecycle-cycle refusal;
- standalone public headers;
- release, metadata, and authority-boundary contracts.

Run:

```sh
meson test -C build --print-errorlogs
```

Sanitizer, shared, and static builds should use separate build directories.

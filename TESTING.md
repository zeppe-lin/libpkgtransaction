# Testing

The native suite covers:

- identity validation and domain separation;
- convergence-policy normalization and duplicate rejection;
- explicit-removal admission and selected/removal conflicts;
- build, check, install, upgrade, retain, remove, and lifecycle classification;
- build/check-only goals without accidental target installation;
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

# Contributing

Changes must preserve the authority boundaries documented in `DESIGN.md`.

In particular:

- do not add discovery, parsing, planning, execution, or state mutation;
- retain requirement scopes and lifecycle actions exactly;
- bind check nodes to the exact selected source program and refuse missing or
  mismatched check authority;
- do not convert deterministic storage order into execution precedence;
- add identity and invariant tests for every semantic field;
- keep public headers valid under strict C++17 compilation;
- use GPL-3.0-or-later SPDX identifiers and Alexandr Savca copyright lines.

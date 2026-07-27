// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "fixture.h"

#include <libpkgtransaction/libpkgtransaction.h>

namespace fixture {

inline pkgresolve::resolution_request resolution_request(
    pkgcatalog::catalog_snapshot catalog,
    pkgstate::snapshot state,
    std::vector<pkgresolve::resolution_goal> goals,
    pkgresolve::installed_preference preference =
        pkgresolve::installed_preference::retain_compatible,
    std::string build = "x86_64",
    std::string target = "x86_64")
{
  return pkgresolve::resolution_request::seal(
      std::move(catalog), std::move(state),
      pkgresolve::architecture_context(
          pkgsource::architecture_reference(std::move(build)),
          pkgsource::architecture_reference(std::move(target))),
      std::move(goals), pkgresolve::resolution_policy(preference));
}

inline pkgresolve::resolution_result resolution(
    pkgcatalog::catalog_snapshot catalog,
    pkgstate::snapshot state,
    std::vector<pkgresolve::resolution_goal> goals,
    pkgresolve::installed_preference preference =
        pkgresolve::installed_preference::retain_compatible)
{
  return pkgresolve::resolve(resolution_request(
      std::move(catalog), std::move(state), std::move(goals), preference));
}

inline std::size_t count_action(
    const pkgtransaction::transaction_program& program,
    pkgtransaction::transaction_action_kind action)
{
  return static_cast<std::size_t>(std::count_if(
      program.nodes().begin(), program.nodes().end(),
      [action](const auto& node) { return node.action() == action; }));
}

} // namespace fixture

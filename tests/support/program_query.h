// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <libpkgtransaction/libpkgtransaction.h>

#include <algorithm>
#include <cstddef>
#include <optional>
#include <string_view>

namespace test_support {

inline std::size_t count_action(
    const pkgtransaction::transaction_program& program,
    pkgtransaction::transaction_action_kind action)
{
  return static_cast<std::size_t>(std::count_if(
      program.nodes().begin(), program.nodes().end(),
      [action](const auto& node) { return node.action() == action; }));
}

inline const pkgtransaction::transaction_node* find_node(
    const pkgtransaction::transaction_program& program,
    std::string_view package,
    pkgtransaction::transaction_action_kind action,
    std::optional<pkgsource::lifecycle_action> lifecycle = std::nullopt)
{
  const auto found = std::find_if(
      program.nodes().begin(), program.nodes().end(), [&](const auto& node) {
        return node.package().name() == package && node.action() == action &&
               node.lifecycle() == lifecycle;
      });
  return found == program.nodes().end() ? nullptr : &*found;
}

inline bool has_phase(
    const pkgtransaction::transaction_program& program,
    const pkgtransaction::transaction_node& before,
    const pkgtransaction::transaction_node& after,
    pkgtransaction::phase_order_kind order)
{
  return std::any_of(
      program.edges().begin(), program.edges().end(), [&](const auto& edge) {
        return edge.kind() == pkgtransaction::transaction_edge_kind::phase &&
               edge.before() == before.identity() &&
               edge.after() == after.identity() && edge.phase_order() == order;
      });
}

inline bool has_requirement(
    const pkgtransaction::transaction_program& program,
    const pkgtransaction::transaction_node& before,
    const pkgtransaction::transaction_node& after,
    pkgsource::requirement_scope_kind scope)
{
  return std::any_of(
      program.edges().begin(), program.edges().end(), [&](const auto& edge) {
        return edge.kind() == pkgtransaction::transaction_edge_kind::requirement &&
               edge.before() == before.identity() &&
               edge.after() == after.identity() && edge.scope() &&
               edge.scope()->kind() == scope && edge.requirement_witness();
      });
}

} // namespace test_support

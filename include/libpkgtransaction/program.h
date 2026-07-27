// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <cstdint>
#include <optional>
#include <variant>
#include <vector>

#include <libpkgresolve/result.h>
#include <libpkgstate/installed_package.h>

#include <libpkgtransaction/identity.h>
#include <libpkgtransaction/model.h>
#include <libpkgtransaction/request.h>

namespace pkgtransaction {

namespace detail { class program_builder; }

using transaction_authority = std::variant<
    pkgresolve::selected_package,
    pkgstate::installed_package>;

class transaction_node final {
public:
  [[nodiscard]] transaction_action_kind action() const noexcept;
  [[nodiscard]] pkgresolve::resolution_environment environment() const noexcept;
  [[nodiscard]] const transaction_authority& authority() const noexcept;
  [[nodiscard]] const pkgresolve::selected_package* selection() const noexcept;
  [[nodiscard]] const pkgstate::installed_package* installed() const noexcept;
  [[nodiscard]] const pkgsource::package_reference& package() const noexcept;
  [[nodiscard]] const std::optional<pkgsource::lifecycle_action>&
  lifecycle() const noexcept;
  [[nodiscard]] const std::vector<pkgresolve::selection_reason>&
  reasons() const noexcept;
  [[nodiscard]] const transaction_node_identity& identity() const noexcept;
private:
  friend class detail::program_builder;
  transaction_node(transaction_action_kind action,
                   pkgresolve::resolution_environment environment,
                   transaction_authority authority,
                   pkgsource::package_reference package,
                   std::optional<pkgsource::lifecycle_action> lifecycle,
                   std::vector<pkgresolve::selection_reason> reasons,
                   transaction_node_identity identity);
  transaction_action_kind action_;
  pkgresolve::resolution_environment environment_;
  transaction_authority authority_;
  pkgsource::package_reference package_;
  std::optional<pkgsource::lifecycle_action> lifecycle_;
  std::vector<pkgresolve::selection_reason> reasons_;
  transaction_node_identity identity_;
};

enum class phase_order_kind {
  build_before_check,
  build_before_target,
  check_before_target,
  pre_lifecycle_before_action,
  action_before_post_lifecycle,
};

[[nodiscard]] std::string_view to_string(phase_order_kind value) noexcept;

class transaction_edge final {
public:
  [[nodiscard]] transaction_edge_kind kind() const noexcept;
  [[nodiscard]] const transaction_node_identity& before() const noexcept;
  [[nodiscard]] const transaction_node_identity& after() const noexcept;
  [[nodiscard]] const std::optional<pkgsource::requirement_scope>&
  scope() const noexcept;
  [[nodiscard]] const std::optional<pkgresolve::requirement_edge_identity>&
  requirement_witness() const noexcept;
  [[nodiscard]] const std::optional<phase_order_kind>& phase_order() const noexcept;
  [[nodiscard]] const transaction_edge_identity& identity() const noexcept;
private:
  friend class detail::program_builder;
  [[nodiscard]] static transaction_edge requirement(
      transaction_node_identity before,
      transaction_node_identity after,
      pkgsource::requirement_scope scope,
      pkgresolve::requirement_edge_identity witness,
      transaction_edge_identity identity);
  [[nodiscard]] static transaction_edge phase(
      transaction_node_identity before,
      transaction_node_identity after,
      phase_order_kind order,
      transaction_edge_identity identity);
  transaction_edge(transaction_edge_kind kind,
                   transaction_node_identity before,
                   transaction_node_identity after,
                   std::optional<pkgsource::requirement_scope> scope,
                   std::optional<pkgresolve::requirement_edge_identity> witness,
                   std::optional<phase_order_kind> phase,
                   transaction_edge_identity identity);
  transaction_edge_kind kind_;
  transaction_node_identity before_;
  transaction_node_identity after_;
  std::optional<pkgsource::requirement_scope> scope_;
  std::optional<pkgresolve::requirement_edge_identity> witness_;
  std::optional<phase_order_kind> phase_;
  transaction_edge_identity identity_;
};

class runtime_cohort final {
public:
  [[nodiscard]] const std::vector<transaction_node_identity>& members() const noexcept;
  [[nodiscard]] const std::vector<pkgresolve::requirement_edge_identity>&
  witnesses() const noexcept;
  [[nodiscard]] const runtime_cohort_identity& identity() const noexcept;
private:
  friend class detail::program_builder;
  runtime_cohort(std::vector<transaction_node_identity> members,
                 std::vector<pkgresolve::requirement_edge_identity> witnesses,
                 runtime_cohort_identity identity);
  std::vector<transaction_node_identity> members_;
  std::vector<pkgresolve::requirement_edge_identity> witnesses_;
  runtime_cohort_identity identity_;
};

class transaction_program final {
public:
  [[nodiscard]] const transaction_request& request() const noexcept;
  [[nodiscard]] const std::vector<transaction_node>& nodes() const noexcept;
  [[nodiscard]] const std::vector<transaction_edge>& edges() const noexcept;
  [[nodiscard]] const std::vector<runtime_cohort>& runtime_cohorts() const noexcept;
  [[nodiscard]] const transaction_node* find(
      const transaction_node_identity& identity) const noexcept;
  [[nodiscard]] std::vector<const transaction_node*> nodes_for(
      const pkgsource::package_reference& package) const;
  [[nodiscard]] const transaction_program_identity& identity() const noexcept;
private:
  friend class detail::program_builder;
  transaction_program(transaction_request request,
                      std::vector<transaction_node> nodes,
                      std::vector<transaction_edge> edges,
                      std::vector<runtime_cohort> runtime_cohorts,
                      transaction_program_identity identity);
  transaction_request request_;
  std::vector<transaction_node> nodes_;
  std::vector<transaction_edge> edges_;
  std::vector<runtime_cohort> runtime_cohorts_;
  transaction_program_identity identity_;
};

} // namespace pkgtransaction

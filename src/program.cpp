// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#include <libpkgtransaction/program.h>

#include <libpkgtransaction/error.h>

#include <algorithm>
#include <utility>

namespace pkgtransaction {
transaction_node::transaction_node(
    transaction_action_kind action,
    pkgresolve::resolution_environment environment,
    transaction_authority authority,
    pkgsource::package_reference package,
    std::optional<pkgsource::lifecycle_action> lifecycle,
    std::vector<pkgresolve::selection_reason> reasons,
    transaction_node_identity identity)
    : action_(action), environment_(environment), authority_(std::move(authority)),
      package_(std::move(package)), lifecycle_(lifecycle), reasons_(std::move(reasons)),
      identity_(std::move(identity))
{
  if ((action_ == transaction_action_kind::lifecycle) != lifecycle_.has_value())
    throw error(error_code::inconsistent_authority,
                "lifecycle binding does not match transaction action");
  const auto* selected = selection();
  const auto* installed_value = installed();
  if (action_ == transaction_action_kind::remove) {
    if (!installed_value)
      throw error(error_code::inconsistent_authority,
                  "remove node lacks direct installed authority");
  } else if (!selected) {
    throw error(error_code::inconsistent_authority,
                "non-removal node lacks resolver selection authority");
  }
  if (selected && selected->package() != package_)
    throw error(error_code::inconsistent_authority,
                "node package differs from selected authority");
  if (installed_value && installed_value->release().name() != package_.name())
    throw error(error_code::inconsistent_authority,
                "node package differs from installed authority");
  if ((action_ == transaction_action_kind::build ||
       action_ == transaction_action_kind::check ||
       action_ == transaction_action_kind::install ||
       action_ == transaction_action_kind::upgrade) &&
      (!selected || selected->authority_kind() !=
          pkgresolve::selection_authority_kind::catalog_candidate))
    throw error(error_code::inconsistent_authority,
                "construction or incoming-target node lacks catalog authority");
  if ((action_ == transaction_action_kind::install ||
       action_ == transaction_action_kind::upgrade ||
       action_ == transaction_action_kind::lifecycle) &&
      environment_ != pkgresolve::resolution_environment::target)
    throw error(error_code::inconsistent_authority,
                "target mutation or lifecycle node is not target-qualified");
  std::sort(reasons_.begin(), reasons_.end());
}
transaction_action_kind transaction_node::action() const noexcept { return action_; }
pkgresolve::resolution_environment transaction_node::environment() const noexcept
{ return environment_; }
const transaction_authority& transaction_node::authority() const noexcept
{ return authority_; }
const pkgresolve::selected_package* transaction_node::selection() const noexcept
{ return std::get_if<pkgresolve::selected_package>(&authority_); }
const pkgstate::installed_package* transaction_node::installed() const noexcept
{ return std::get_if<pkgstate::installed_package>(&authority_); }
const pkgsource::package_reference& transaction_node::package() const noexcept
{ return package_; }
const std::optional<pkgsource::lifecycle_action>&
transaction_node::lifecycle() const noexcept { return lifecycle_; }
const std::vector<pkgresolve::selection_reason>&
transaction_node::reasons() const noexcept { return reasons_; }
const transaction_node_identity& transaction_node::identity() const noexcept
{ return identity_; }

std::string_view to_string(phase_order_kind value) noexcept
{
  switch (value) {
  case phase_order_kind::build_before_check: return "build-before-check";
  case phase_order_kind::build_before_target: return "build-before-target";
  case phase_order_kind::check_before_target: return "check-before-target";
  case phase_order_kind::pre_lifecycle_before_action:
    return "pre-lifecycle-before-action";
  case phase_order_kind::action_before_post_lifecycle:
    return "action-before-post-lifecycle";
  }
  return "unknown";
}

transaction_edge::transaction_edge(
    transaction_edge_kind kind,
    transaction_node_identity before,
    transaction_node_identity after,
    std::optional<pkgsource::requirement_scope> scope,
    std::optional<pkgresolve::requirement_edge_identity> witness,
    std::optional<phase_order_kind> phase,
    transaction_edge_identity identity)
    : kind_(kind), before_(std::move(before)), after_(std::move(after)),
      scope_(std::move(scope)), witness_(std::move(witness)), phase_(phase),
      identity_(std::move(identity))
{
  if (before_ == after_)
    throw error(error_code::inconsistent_authority,
                "transaction edge orders one node against itself");
  if ((kind_ == transaction_edge_kind::requirement) !=
          (scope_.has_value() && witness_.has_value()) ||
      (kind_ == transaction_edge_kind::phase) != phase_.has_value())
    throw error(error_code::inconsistent_authority,
                "transaction edge evidence does not match its kind");
}
transaction_edge transaction_edge::requirement(
    transaction_node_identity before,
    transaction_node_identity after,
    pkgsource::requirement_scope scope,
    pkgresolve::requirement_edge_identity witness,
    transaction_edge_identity identity)
{
  return transaction_edge(transaction_edge_kind::requirement,
                          std::move(before), std::move(after), std::move(scope),
                          std::move(witness), std::nullopt, std::move(identity));
}
transaction_edge transaction_edge::phase(
    transaction_node_identity before,
    transaction_node_identity after,
    phase_order_kind order,
    transaction_edge_identity identity)
{
  return transaction_edge(transaction_edge_kind::phase,
                          std::move(before), std::move(after), std::nullopt,
                          std::nullopt, order, std::move(identity));
}
transaction_edge_kind transaction_edge::kind() const noexcept { return kind_; }
const transaction_node_identity& transaction_edge::before() const noexcept
{ return before_; }
const transaction_node_identity& transaction_edge::after() const noexcept
{ return after_; }
const std::optional<pkgsource::requirement_scope>& transaction_edge::scope() const noexcept
{ return scope_; }
const std::optional<pkgresolve::requirement_edge_identity>&
transaction_edge::requirement_witness() const noexcept { return witness_; }
const std::optional<phase_order_kind>& transaction_edge::phase_order() const noexcept
{ return phase_; }
const transaction_edge_identity& transaction_edge::identity() const noexcept
{ return identity_; }

runtime_cohort::runtime_cohort(
    std::vector<transaction_node_identity> members,
    std::vector<pkgresolve::requirement_edge_identity> witnesses,
    runtime_cohort_identity identity)
    : members_(std::move(members)), witnesses_(std::move(witnesses)),
      identity_(std::move(identity))
{
  std::sort(members_.begin(), members_.end());
  std::sort(witnesses_.begin(), witnesses_.end());
  if (members_.empty() || witnesses_.empty())
    throw error(error_code::inconsistent_authority,
                "runtime cohort lacks members or witnesses");
  if (std::adjacent_find(members_.begin(), members_.end()) != members_.end() ||
      std::adjacent_find(witnesses_.begin(), witnesses_.end()) != witnesses_.end())
    throw error(error_code::inconsistent_authority,
                "runtime cohort contains duplicate authority");
}
const std::vector<transaction_node_identity>& runtime_cohort::members() const noexcept
{ return members_; }
const std::vector<pkgresolve::requirement_edge_identity>&
runtime_cohort::witnesses() const noexcept { return witnesses_; }
const runtime_cohort_identity& runtime_cohort::identity() const noexcept
{ return identity_; }

transaction_program::transaction_program(
    transaction_request request,
    std::vector<transaction_node> nodes,
    std::vector<transaction_edge> edges,
    std::vector<runtime_cohort> runtime_cohorts,
    transaction_program_identity identity)
    : request_(std::move(request)), nodes_(std::move(nodes)),
      edges_(std::move(edges)), runtime_cohorts_(std::move(runtime_cohorts)),
      identity_(std::move(identity)) {}
const transaction_request& transaction_program::request() const noexcept
{ return request_; }
const std::vector<transaction_node>& transaction_program::nodes() const noexcept
{ return nodes_; }
const std::vector<transaction_edge>& transaction_program::edges() const noexcept
{ return edges_; }
const std::vector<runtime_cohort>&
transaction_program::runtime_cohorts() const noexcept { return runtime_cohorts_; }
const transaction_node* transaction_program::find(
    const transaction_node_identity& identity) const noexcept
{
  const auto it = std::lower_bound(nodes_.begin(), nodes_.end(), identity,
      [](const transaction_node& node, const transaction_node_identity& value) {
        return node.identity() < value;
      });
  return it != nodes_.end() && it->identity() == identity ? &*it : nullptr;
}
std::vector<const transaction_node*> transaction_program::nodes_for(
    const pkgsource::package_reference& package) const
{
  std::vector<const transaction_node*> result;
  for (const auto& node : nodes_)
    if (node.package() == package)
      result.push_back(&node);
  return result;
}
const transaction_program_identity& transaction_program::identity() const noexcept
{ return identity_; }
} // namespace pkgtransaction

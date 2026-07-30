// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#include <libpkgtransaction/composer.h>

#include <libpkgtransaction/error.h>

#include "identity_support.h"

#include <algorithm>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <utility>

namespace pkgtransaction::detail {
class program_builder final {
public:
  static transaction_node node(
      transaction_action_kind action,
      pkgresolve::resolution_environment environment,
      transaction_authority authority,
      pkgsource::package_reference package,
      std::optional<pkgsource::lifecycle_action> lifecycle,
      std::vector<pkgresolve::selection_reason> reasons,
      transaction_node_identity identity)
  {
    return transaction_node(action, environment, std::move(authority),
                            std::move(package), lifecycle, std::move(reasons),
                            std::move(identity));
  }
  static transaction_edge requirement_edge(
      transaction_node_identity before,
      transaction_node_identity after,
      pkgsource::requirement_scope scope,
      pkgresolve::requirement_edge_identity witness,
      transaction_edge_identity identity)
  {
    return transaction_edge::requirement(
        std::move(before), std::move(after), std::move(scope),
        std::move(witness), std::move(identity));
  }
  static transaction_edge phase_edge(
      transaction_node_identity before,
      transaction_node_identity after,
      phase_order_kind order,
      transaction_edge_identity identity)
  {
    return transaction_edge::phase(
        std::move(before), std::move(after), order, std::move(identity));
  }
  static runtime_cohort cohort(
      std::vector<transaction_node_identity> members,
      std::vector<pkgresolve::requirement_edge_identity> witnesses,
      runtime_cohort_identity identity)
  {
    return runtime_cohort(std::move(members), std::move(witnesses),
                          std::move(identity));
  }
  static transaction_program program(
      transaction_request request,
      std::vector<transaction_node> nodes,
      std::vector<transaction_edge> edges,
      std::vector<runtime_cohort> cohorts,
      transaction_program_identity identity)
  {
    return transaction_program(std::move(request), std::move(nodes),
                               std::move(edges), std::move(cohorts),
                               std::move(identity));
  }
};
} // namespace pkgtransaction::detail

namespace pkgtransaction {
namespace {
using selection_id = pkgresolve::package_selection_identity;
using node_id = transaction_node_identity;

std::string key(const selection_id& selection,
                transaction_action_kind action,
                std::optional<pkgsource::lifecycle_action> lifecycle = std::nullopt)
{
  std::string value = selection.hex() + "/" + std::string(to_string(action));
  if (lifecycle)
    value += "/" + std::string(pkgsource::to_string(*lifecycle));
  return value;
}
std::string installed_key(const pkgstate::installed_package& installed,
                          transaction_action_kind action)
{
  return installed.identity().string() + "/" + std::string(to_string(action));
}
void write_scope(detail::identity_writer& writer,
                 const pkgsource::requirement_scope& scope)
{
  writer.text(pkgsource::to_string(scope.kind()));
  writer.boolean(scope.action().has_value());
  if (scope.action()) writer.text(pkgsource::to_string(*scope.action()));
}
transaction_node_identity node_identity(
    const transaction_request& request,
    transaction_action_kind action,
    pkgresolve::resolution_environment environment,
    const transaction_authority& authority,
    const std::optional<pkgsource::lifecycle_action>& lifecycle)
{
  detail::identity_writer writer;
  writer.text("libpkgtransaction/node/1");
  writer.text(request.identity().hex());
  writer.text(to_string(action));
  writer.text(pkgresolve::to_string(environment));
  if (const auto* selected = std::get_if<pkgresolve::selected_package>(&authority)) {
    writer.text("selection");
    writer.text(selected->identity().hex());
  } else {
    writer.text("installed");
    writer.text(std::get<pkgstate::installed_package>(authority).identity().string());
  }
  writer.boolean(lifecycle.has_value());
  if (lifecycle) writer.text(pkgsource::to_string(*lifecycle));
  return transaction_node_identity::from_sha256(writer.finish());
}
transaction_edge_identity edge_identity(
    const transaction_request& request,
    transaction_edge_kind kind,
    const node_id& before,
    const node_id& after,
    const std::optional<pkgsource::requirement_scope>& scope,
    const std::optional<pkgresolve::requirement_edge_identity>& witness,
    const std::optional<phase_order_kind>& phase)
{
  detail::identity_writer writer;
  writer.text("libpkgtransaction/edge/1");
  writer.text(request.identity().hex());
  writer.text(to_string(kind));
  writer.text(before.hex());
  writer.text(after.hex());
  writer.boolean(scope.has_value());
  if (scope) write_scope(writer, *scope);
  writer.boolean(witness.has_value());
  if (witness) writer.text(witness->hex());
  writer.boolean(phase.has_value());
  if (phase) writer.text(to_string(*phase));
  return transaction_edge_identity::from_sha256(writer.finish());
}
runtime_cohort_identity cohort_identity(
    const transaction_request& request,
    const std::vector<node_id>& members,
    const std::vector<pkgresolve::requirement_edge_identity>& witnesses)
{
  detail::identity_writer writer;
  writer.text("libpkgtransaction/runtime-cohort/1");
  writer.text(request.identity().hex());
  writer.number(members.size());
  for (const auto& member : members) writer.text(member.hex());
  writer.number(witnesses.size());
  for (const auto& witness : witnesses) writer.text(witness.hex());
  return runtime_cohort_identity::from_sha256(writer.finish());
}
transaction_program_identity program_identity(
    const transaction_request& request,
    const std::vector<transaction_node>& nodes,
    const std::vector<transaction_edge>& edges,
    const std::vector<runtime_cohort>& cohorts)
{
  detail::identity_writer writer;
  writer.text("libpkgtransaction/program/1");
  writer.text(request.identity().hex());
  writer.number(nodes.size());
  for (const auto& node : nodes) writer.text(node.identity().hex());
  writer.number(edges.size());
  for (const auto& edge : edges) writer.text(edge.identity().hex());
  writer.number(cohorts.size());
  for (const auto& cohort : cohorts) writer.text(cohort.identity().hex());
  return transaction_program_identity::from_sha256(writer.finish());
}

bool bytes_equal_hex(const pkgstate::digest_bytes& bytes,
                     const std::string& hex)
{
  if (bytes.size() * 2 != hex.size()) return false;
  const auto digit = [](char value) -> unsigned int {
    if (value >= '0' && value <= '9')
      return static_cast<unsigned int>(value - '0');
    if (value >= 'a' && value <= 'f')
      return static_cast<unsigned int>(value - 'a' + 10);
    return 16;
  };
  for (std::size_t index = 0; index < bytes.size(); ++index) {
    const unsigned int high = digit(hex[index * 2]);
    const unsigned int low = digit(hex[index * 2 + 1]);
    if (high > 15 || low > 15 ||
        bytes[index] != static_cast<std::uint8_t>((high << 4U) | low))
      return false;
  }
  return true;
}

bool exact_installed(const pkgresolve::selected_package& selection,
                     const pkgstate::installed_package& installed)
{
  return bytes_equal_hex(installed.release().identity().bytes(),
                         selection.release().identity().hex()) &&
         bytes_equal_hex(installed.control().source().snapshot().bytes(),
                         selection.source_snapshot().hex());
}

bool target_operation_required(
    const std::vector<pkgresolve::selection_reason>& reasons)
{
  return std::any_of(reasons.begin(), reasons.end(), [](const auto& reason) {
    return reason.scope().kind() == pkgsource::requirement_scope_kind::run ||
           reason.scope().kind() == pkgsource::requirement_scope_kind::lifecycle;
  });
}
bool removal_only(const std::vector<pkgresolve::selection_reason>& reasons)
{
  if (reasons.empty()) return false;
  return std::all_of(reasons.begin(), reasons.end(), [](const auto& reason) {
    if (reason.kind() != pkgresolve::selection_reason_kind::direct_goal &&
        reason.kind() != pkgresolve::selection_reason_kind::profile_goal)
      return false;
    if (reason.scope().kind() != pkgsource::requirement_scope_kind::lifecycle ||
        !reason.scope().action()) return false;
    return *reason.scope().action() == pkgsource::lifecycle_action::pre_remove ||
           *reason.scope().action() == pkgsource::lifecycle_action::post_remove;
  });
}

void require_check_authority(const pkgresolve::selected_package& selection)
{
  const auto* candidate = selection.candidate();
  if (!candidate)
    throw error(error_code::inconsistent_authority,
                "check goal lacks catalog-candidate authority: " +
                selection.package().name());
  if (candidate->package() != selection.package() ||
      candidate->release().identity() != selection.release().identity() ||
      candidate->source().identity() != selection.source_snapshot())
    throw error(error_code::inconsistent_authority,
                "check goal selection differs from catalog authority: " +
                selection.package().name());
  if (!candidate->source().recipe().check_program())
    throw error(error_code::missing_check_program,
                "check goal has no check program: " +
                selection.package().name());
}

struct graph_state {
  transaction_request request;
  std::vector<transaction_node> nodes;
  std::map<std::string, node_id> node_by_key;
  std::map<std::string, const pkgresolve::selected_package*> selections;
};

node_id add_selected_node(graph_state& state,
                          const pkgresolve::selected_package& selected,
                          transaction_action_kind action,
                          std::optional<pkgsource::lifecycle_action> lifecycle = std::nullopt)
{
  const std::string node_key = key(selected.identity(), action, lifecycle);
  if (const auto found = state.node_by_key.find(node_key);
      found != state.node_by_key.end()) return found->second;
  transaction_authority authority = selected;
  const node_id identity = node_identity(state.request, action,
      selected.environment(), authority, lifecycle);
  state.nodes.push_back(detail::program_builder::node(
      action, selected.environment(), std::move(authority), selected.package(),
      lifecycle, state.request.resolution().reasons_for(selected.identity()),
      identity));
  state.node_by_key.emplace(node_key, identity);
  return identity;
}
node_id add_installed_node(graph_state& state,
                           const pkgstate::installed_package& installed,
                           transaction_action_kind action)
{
  const std::string node_key = installed_key(installed, action);
  if (const auto found = state.node_by_key.find(node_key);
      found != state.node_by_key.end()) return found->second;
  transaction_authority authority = installed;
  const node_id identity = node_identity(state.request, action,
      pkgresolve::resolution_environment::target, authority, std::nullopt);
  state.nodes.push_back(detail::program_builder::node(
      action, pkgresolve::resolution_environment::target, std::move(authority),
      pkgsource::package_reference(installed.release().name()), std::nullopt,
      std::vector<pkgresolve::selection_reason>{}, identity));
  state.node_by_key.emplace(node_key, identity);
  return identity;
}
std::optional<node_id> lookup(const graph_state& state,
                              const selection_id& selection,
                              transaction_action_kind action,
                              std::optional<pkgsource::lifecycle_action> lifecycle = std::nullopt)
{
  const auto found = state.node_by_key.find(key(selection, action, lifecycle));
  if (found == state.node_by_key.end()) return std::nullopt;
  return found->second;
}
std::optional<node_id> completion_node(const graph_state& state,
                                       const selection_id& selection)
{
  for (const auto action : {transaction_action_kind::install,
                            transaction_action_kind::upgrade,
                            transaction_action_kind::retain,
                            transaction_action_kind::build})
    if (const auto value = lookup(state, selection, action)) return value;
  return std::nullopt;
}

std::optional<node_id> target_upgrade_node(
    const graph_state& state,
    const pkgsource::package_reference& package)
{
  std::optional<node_id> result;
  for (const auto& node : state.nodes) {
    if (node.environment() != pkgresolve::resolution_environment::target ||
        node.action() != transaction_action_kind::upgrade ||
        node.package().name() != package.name())
      continue;
    if (result && *result != node.identity())
      throw error(error_code::inconsistent_authority,
                  "multiple target upgrades for package: " + package.name());
    result = node.identity();
  }
  return result;
}

std::optional<node_id> removal_lifecycle_target(
    const graph_state& state,
    const pkgstate::installed_package& installed)
{
  std::optional<node_id> remove;
  if (const auto found = state.node_by_key.find(
          installed_key(installed, transaction_action_kind::remove));
      found != state.node_by_key.end())
    remove = found->second;

  const auto upgrade = target_upgrade_node(
      state, pkgsource::package_reference(installed.release().name()));
  if (remove && upgrade)
    throw error(error_code::inconsistent_authority,
                "package has both remove and upgrade actions: " +
                installed.release().name());
  return remove ? remove : upgrade;
}

std::vector<std::vector<std::string>> strongly_connected(
    const std::map<std::string, std::vector<std::string>>& graph)
{
  std::map<std::string, int> index;
  std::map<std::string, int> low;
  std::set<std::string> on_stack;
  std::vector<std::string> stack;
  std::vector<std::vector<std::string>> result;
  int next = 0;
  std::function<void(const std::string&)> visit = [&](const std::string& node) {
    index[node] = low[node] = next++;
    stack.push_back(node);
    on_stack.insert(node);
    const auto found = graph.find(node);
    if (found != graph.end()) for (const auto& target : found->second) {
      if (!index.count(target)) { visit(target); low[node] = std::min(low[node], low[target]); }
      else if (on_stack.count(target)) low[node] = std::min(low[node], index[target]);
    }
    if (low[node] == index[node]) {
      std::vector<std::string> component;
      while (true) {
        const std::string value = stack.back(); stack.pop_back(); on_stack.erase(value);
        component.push_back(value);
        if (value == node) break;
      }
      std::sort(component.begin(), component.end());
      result.push_back(std::move(component));
    }
  };
  for (const auto& [node, targets] : graph) {
    (void)targets;
    if (!index.count(node)) visit(node);
  }
  return result;
}
bool cyclic_component(const std::vector<std::string>& component,
                      const std::map<std::string, std::vector<std::string>>& graph)
{
  if (component.size() > 1) return true;
  const auto found = graph.find(component.front());
  return found != graph.end() &&
      std::find(found->second.begin(), found->second.end(), component.front()) != found->second.end();
}
} // namespace

transaction_program compose(transaction_request request)
{
  graph_state state{std::move(request), {}, {}, {}};
  const auto& result = state.request.resolution();
  for (const auto& selection : result.selections())
    state.selections.emplace(selection.identity().hex(), &selection);

  for (const auto& selection : result.selections()) {
    const auto reasons = result.reasons_for(selection.identity());
    if (selection.authority_kind() ==
        pkgresolve::selection_authority_kind::catalog_candidate) {
      std::optional<transaction_action_kind> target_action;
      if (selection.environment() == pkgresolve::resolution_environment::target &&
          target_operation_required(reasons)) {
        const auto* installed = result.request().installed().find_package(
            selection.package().name());
        if (!installed)
          target_action = transaction_action_kind::install;
        else if (exact_installed(selection, *installed))
          target_action = transaction_action_kind::retain;
        else
          target_action = transaction_action_kind::upgrade;
      }
      const bool explicit_construction = std::any_of(
          reasons.begin(), reasons.end(), [](const auto& reason) {
            return reason.scope().kind() ==
                       pkgsource::requirement_scope_kind::build ||
                   reason.scope().kind() ==
                       pkgsource::requirement_scope_kind::check;
          });
      if (selection.environment() == pkgresolve::resolution_environment::build ||
          explicit_construction ||
          (target_action && *target_action != transaction_action_kind::retain))
        add_selected_node(state, selection, transaction_action_kind::build);
      if (target_action)
        add_selected_node(state, selection, *target_action);
    } else if (selection.environment() ==
                   pkgresolve::resolution_environment::build ||
               target_operation_required(reasons)) {
      if (!removal_only(reasons))
        add_selected_node(state, selection, transaction_action_kind::retain);
    }
  }

  for (const auto& goal : result.goals()) {
    for (const auto& member : goal.members()) {
      const auto found = state.selections.find(member.selection().hex());
      if (found == state.selections.end())
        throw error(error_code::inconsistent_authority,
                    "goal references an unknown selection");
      const auto& selection = *found->second;
      if (goal.goal().scope().kind() ==
          pkgsource::requirement_scope_kind::check) {
        require_check_authority(selection);
        add_selected_node(state, selection, transaction_action_kind::check);
      }
      if (goal.goal().scope().kind() == pkgsource::requirement_scope_kind::lifecycle)
        add_selected_node(state, selection, transaction_action_kind::lifecycle,
                          goal.goal().scope().action());
    }
  }

  std::set<std::string> desired_target;
  for (const auto& selection : result.selections()) {
    const auto reasons = result.reasons_for(selection.identity());
    if (selection.environment() == pkgresolve::resolution_environment::target &&
        target_operation_required(reasons) && !removal_only(reasons))
      desired_target.insert(selection.package().name());
  }
  if (state.request.policy().mode() == convergence_mode::remove_explicit) {
    for (const auto& package : state.request.policy().removals())
      add_installed_node(state, *result.request().installed().find_package(package.name()),
                         transaction_action_kind::remove);
  } else if (state.request.policy().mode() == convergence_mode::converge_exact) {
    for (const auto& installed : result.request().installed().packages())
      if (!desired_target.count(installed.release().name()))
        add_installed_node(state, installed, transaction_action_kind::remove);
  }

  for (const auto& node : state.nodes) {
    if (node.action() != transaction_action_kind::lifecycle)
      continue;
    const auto* selection = node.selection();
    if (!selection || !node.lifecycle())
      throw error(error_code::inconsistent_authority,
                  "lifecycle node lacks selected authority");
    const auto action = *node.lifecycle();
    if (action == pkgsource::lifecycle_action::pre_install ||
        action == pkgsource::lifecycle_action::post_install) {
      if (!lookup(state, selection->identity(), transaction_action_kind::install) &&
          !lookup(state, selection->identity(), transaction_action_kind::upgrade))
        throw error(error_code::unbound_lifecycle,
                    "install lifecycle has no install or upgrade action: " +
                    selection->package().name());
    } else {
      const auto* installed = selection->installed();
      if (!installed || !removal_lifecycle_target(state, *installed))
        throw error(error_code::unbound_lifecycle,
                    "removal lifecycle has no remove or upgrade action: " +
                    selection->package().name());
    }
  }

  std::map<std::string, std::vector<std::string>> runtime_graph;
  std::map<std::string, std::vector<std::string>> construction_graph;
  std::map<std::string, std::vector<std::string>> lifecycle_graph;
  for (const auto& edge : result.edges()) {
    if (edge.scope().kind() == pkgsource::requirement_scope_kind::run) {
      runtime_graph[edge.issuer().hex()].push_back(edge.required().hex());
      runtime_graph.try_emplace(edge.required().hex());
      continue;
    }
    if (edge.scope().kind() == pkgsource::requirement_scope_kind::lifecycle) {
      lifecycle_graph[edge.issuer().hex()].push_back(edge.required().hex());
      lifecycle_graph.try_emplace(edge.required().hex());
      continue;
    }
    const auto issuer = state.selections.find(edge.issuer().hex());
    const auto required = state.selections.find(edge.required().hex());
    if (issuer != state.selections.end() && required != state.selections.end() &&
        issuer->second->authority_kind() ==
            pkgresolve::selection_authority_kind::catalog_candidate &&
        required->second->authority_kind() ==
            pkgresolve::selection_authority_kind::catalog_candidate) {
      construction_graph[edge.issuer().hex()].push_back(edge.required().hex());
      construction_graph.try_emplace(edge.required().hex());
    }
  }
  for (const auto& component : strongly_connected(construction_graph))
    if (cyclic_component(component, construction_graph))
      throw error(error_code::construction_cycle,
                  "build/check requirement cycle cannot be ordered");
  for (const auto& component : strongly_connected(lifecycle_graph))
    if (cyclic_component(component, lifecycle_graph))
      throw error(error_code::lifecycle_cycle,
                  "lifecycle requirement cycle cannot be ordered");

  std::map<std::string, std::size_t> runtime_component;
  std::vector<runtime_cohort> cohorts;
  const auto runtime_components = strongly_connected(runtime_graph);
  for (std::size_t index = 0; index < runtime_components.size(); ++index) {
    const auto& component = runtime_components[index];
    if (!cyclic_component(component, runtime_graph)) continue;
    std::vector<node_id> members;
    std::vector<pkgresolve::requirement_edge_identity> witnesses;
    std::set<std::string> component_set(component.begin(), component.end());
    for (const auto& value : component) {
      runtime_component[value] = cohorts.size();
      const auto selection = state.selections.find(value);
      if (selection != state.selections.end())
        if (const auto node = completion_node(state, selection->second->identity()))
          members.push_back(*node);
    }
    for (const auto& edge : result.edges())
      if (edge.scope().kind() == pkgsource::requirement_scope_kind::run &&
          component_set.count(edge.issuer().hex()) &&
          component_set.count(edge.required().hex()))
        witnesses.push_back(edge.identity());
    std::sort(members.begin(), members.end());
    members.erase(std::unique(members.begin(), members.end()), members.end());
    std::sort(witnesses.begin(), witnesses.end());
    cohorts.push_back(detail::program_builder::cohort(
        members, witnesses, cohort_identity(state.request, members, witnesses)));
  }

  std::vector<transaction_edge> edges;
  auto add_requirement = [&](const node_id& before, const node_id& after,
                             const pkgresolve::requirement_edge& witness) {
    if (before == after) return;
    const auto identity = edge_identity(state.request,
        transaction_edge_kind::requirement, before, after, witness.scope(),
        witness.identity(), std::nullopt);
    edges.push_back(detail::program_builder::requirement_edge(
        before, after, witness.scope(), witness.identity(), identity));
  };
  auto add_phase = [&](const node_id& before, const node_id& after,
                       phase_order_kind order) {
    if (before == after) return;
    const auto identity = edge_identity(state.request,
        transaction_edge_kind::phase, before, after, std::nullopt,
        std::nullopt, order);
    edges.push_back(detail::program_builder::phase_edge(
        before, after, order, identity));
  };

  for (const auto& witness : result.edges()) {
    if (witness.scope().kind() == pkgsource::requirement_scope_kind::run) {
      const auto left = runtime_component.find(witness.issuer().hex());
      const auto right = runtime_component.find(witness.required().hex());
      if (left != runtime_component.end() && right != runtime_component.end() &&
          left->second == right->second) continue;
    }
    const auto before = completion_node(state, witness.required());
    std::optional<node_id> after;
    if (witness.scope().kind() == pkgsource::requirement_scope_kind::build ||
        witness.scope().kind() == pkgsource::requirement_scope_kind::check)
      after = lookup(state, witness.issuer(), transaction_action_kind::build);
    else if (witness.scope().kind() == pkgsource::requirement_scope_kind::lifecycle)
      after = lookup(state, witness.issuer(), transaction_action_kind::lifecycle,
                     witness.scope().action());
    else
      after = completion_node(state, witness.issuer());
    if (before && after) add_requirement(*before, *after, witness);
  }

  for (const auto& selection : result.selections()) {
    const auto build = lookup(state, selection.identity(), transaction_action_kind::build);
    const auto check = lookup(state, selection.identity(), transaction_action_kind::check);
    std::optional<node_id> target;
    for (const auto action : {transaction_action_kind::install,
                              transaction_action_kind::upgrade})
      if (const auto found = lookup(state, selection.identity(), action)) target = found;
    if (build && check) add_phase(*build, *check, phase_order_kind::build_before_check);
    if (build && target) add_phase(*build, *target, phase_order_kind::build_before_target);
    if (check && target) add_phase(*check, *target, phase_order_kind::check_before_target);
    const auto pre_install = lookup(state, selection.identity(),
        transaction_action_kind::lifecycle, pkgsource::lifecycle_action::pre_install);
    const auto post_install = lookup(state, selection.identity(),
        transaction_action_kind::lifecycle, pkgsource::lifecycle_action::post_install);
    if (pre_install && target)
      add_phase(*pre_install, *target,
                phase_order_kind::pre_lifecycle_before_action);
    if (post_install && target)
      add_phase(*target, *post_install,
                phase_order_kind::action_before_post_lifecycle);
  }
  for (const auto& installed : result.request().installed().packages()) {
    const auto target = removal_lifecycle_target(state, installed);
    if (!target) continue;
    for (const auto& selection : result.selections()) {
      const auto* selected_installed = selection.installed();
      if (!selected_installed ||
          selected_installed->identity() != installed.identity())
        continue;
      const auto pre = lookup(state, selection.identity(),
          transaction_action_kind::lifecycle, pkgsource::lifecycle_action::pre_remove);
      const auto post = lookup(state, selection.identity(),
          transaction_action_kind::lifecycle, pkgsource::lifecycle_action::post_remove);
      if (pre) add_phase(*pre, *target,
                         phase_order_kind::pre_lifecycle_before_action);
      if (post) add_phase(*target, *post,
                          phase_order_kind::action_before_post_lifecycle);
    }
  }

  std::sort(state.nodes.begin(), state.nodes.end(),
      [](const auto& lhs, const auto& rhs) { return lhs.identity() < rhs.identity(); });
  std::sort(edges.begin(), edges.end(),
      [](const auto& lhs, const auto& rhs) { return lhs.identity() < rhs.identity(); });
  edges.erase(std::unique(edges.begin(), edges.end(),
      [](const auto& lhs, const auto& rhs) { return lhs.identity() == rhs.identity(); }),
      edges.end());
  std::sort(cohorts.begin(), cohorts.end(),
      [](const auto& lhs, const auto& rhs) { return lhs.identity() < rhs.identity(); });
  const auto identity = program_identity(state.request, state.nodes, edges, cohorts);
  return detail::program_builder::program(
      std::move(state.request), std::move(state.nodes), std::move(edges),
      std::move(cohorts), identity);
}
} // namespace pkgtransaction

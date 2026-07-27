// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#include "fixture_transaction.h"
#include "test.h"

#include <algorithm>

int main()
{
  using namespace pkgtransaction;
  auto profiles = fixture::profiles();
  auto app = fixture::source(profiles, "app", {
      fixture::requirement(pkgsource::requirement_scope::run(),
                           "lib", "requirements.run[0]"),
      fixture::requirement(pkgsource::requirement_scope::check(),
                           "tester", "requirements.check[0]"),
      fixture::requirement(
          pkgsource::requirement_scope::lifecycle(
              pkgsource::lifecycle_action::post_install),
          "cache", "requirements.lifecycle.post-install[0]"),
      fixture::requirement(
          pkgsource::requirement_scope::lifecycle(
              pkgsource::lifecycle_action::pre_remove),
          "cleanup", "requirements.lifecycle.pre-remove[0]"),
  });
  auto lib = fixture::source(profiles, "lib", {
      fixture::requirement(pkgsource::requirement_scope::run(),
                           "runtime", "requirements.run[0]"),
  });
  auto runtime = fixture::source(profiles, "runtime");
  auto tester = fixture::source(profiles, "tester");
  auto cache = fixture::source(profiles, "cache");
  auto cleanup = fixture::source(profiles, "cleanup");
  auto stale = fixture::source(profiles, "stale");
  auto catalog = fixture::catalog(profiles,
      {app, lib, runtime, tester, cache, cleanup, stale});

  const auto run_result = fixture::resolution(
      catalog, fixture::empty_state(),
      {fixture::package_goal(pkgsource::requirement_scope::run(), "app")});
  const auto run_program = compose(transaction_request::seal(run_result));
  TEST_CHECK(fixture::count_action(run_program,
      transaction_action_kind::build) == 3);
  TEST_CHECK(fixture::count_action(run_program,
      transaction_action_kind::install) == 3);
  TEST_CHECK(run_program.runtime_cohorts().empty());
  TEST_CHECK(std::count_if(run_program.edges().begin(), run_program.edges().end(),
      [](const auto& edge) {
        return edge.kind() == transaction_edge_kind::requirement;
      }) == 2);

  const auto check_result = fixture::resolution(
      catalog, fixture::empty_state(),
      {fixture::package_goal(pkgsource::requirement_scope::check(), "app")});
  const auto check_program = compose(transaction_request::seal(check_result));
  TEST_CHECK(fixture::count_action(check_program,
      transaction_action_kind::check) == 1);
  TEST_CHECK(fixture::count_action(check_program,
      transaction_action_kind::install) == 0);

  const auto post_install = pkgsource::requirement_scope::lifecycle(
      pkgsource::lifecycle_action::post_install);
  const auto lifecycle_result = fixture::resolution(
      catalog, fixture::empty_state(),
      {fixture::package_goal(post_install, "app")});
  const auto lifecycle_program = compose(
      transaction_request::seal(lifecycle_result));
  TEST_CHECK(fixture::count_action(lifecycle_program,
      transaction_action_kind::lifecycle) == 1);

  const auto binding = fixture::target();
  const auto installed_app = fixture::installed_package(app, binding, 60);
  const auto installed_stale = fixture::installed_package(stale, binding, 90);
  const auto installed_state = fixture::state(
      {installed_app, installed_stale}, binding);
  const auto retained_result = fixture::resolution(
      catalog, installed_state,
      {fixture::package_goal(pkgsource::requirement_scope::run(), "app")});
  const auto retained_program = compose(
      transaction_request::seal(retained_result,
          convergence_policy::converge_exact()));
  TEST_CHECK(fixture::count_action(retained_program,
      transaction_action_kind::retain) >= 1);
  TEST_CHECK(fixture::count_action(retained_program,
      transaction_action_kind::remove) == 1);
  const auto preserved_program = compose(
      transaction_request::seal(retained_result));
  TEST_CHECK(fixture::count_action(preserved_program,
      transaction_action_kind::remove) == 0);

  const auto pre_remove = pkgsource::requirement_scope::lifecycle(
      pkgsource::lifecycle_action::pre_remove);
  const auto removal_result = fixture::resolution(
      catalog, installed_state,
      {fixture::package_goal(pre_remove, "app")});
  TEST_TRANSACTION_THROWS(error_code::unbound_lifecycle,
      compose(transaction_request::seal(removal_result)));
  const auto removal_program = compose(transaction_request::seal(
      removal_result, convergence_policy::remove_explicit(
          {pkgsource::package_reference("app")})));
  TEST_CHECK(fixture::count_action(removal_program,
      transaction_action_kind::remove) == 1);
  TEST_CHECK(fixture::count_action(removal_program,
      transaction_action_kind::lifecycle) == 1);
  TEST_CHECK(std::any_of(removal_program.edges().begin(),
                         removal_program.edges().end(), [](const auto& edge) {
    return edge.kind() == transaction_edge_kind::phase &&
           edge.phase_order() == phase_order_kind::pre_lifecycle_before_action;
  }));

  const auto installed_cleanup = fixture::installed_package(cleanup, binding, 110);
  const auto removal_dependency_state = fixture::state(
      {installed_app, installed_cleanup}, binding);
  const auto removal_dependency_result = fixture::resolution(
      catalog, removal_dependency_state,
      {fixture::package_goal(pre_remove, "app")});
  TEST_TRANSACTION_THROWS(error_code::selected_for_removal,
      transaction_request::seal(removal_dependency_result,
          convergence_policy::remove_explicit(
              {pkgsource::package_reference("cleanup")})));

  auto old_app = fixture::source(profiles, "app", {}, {"x86_64"}, {"x86_64"},
                                 "0.9.0", 1);
  const auto old_installed = fixture::installed_package(old_app, binding, 100);
  const auto upgrade_state = fixture::state({old_installed}, binding);
  const auto upgrade_result = fixture::resolution(
      catalog, upgrade_state,
      {fixture::package_goal(pkgsource::requirement_scope::run(), "app")},
      pkgresolve::installed_preference::prefer_catalog);
  const auto upgrade_program = compose(
      transaction_request::seal(upgrade_result));
  TEST_CHECK(fixture::count_action(upgrade_program,
      transaction_action_kind::upgrade) == 1);

  auto cycle_a = fixture::source(profiles, "cycle-a", {
      fixture::requirement(pkgsource::requirement_scope::run(),
                           "cycle-b", "requirements.run[0]"),
  });
  auto cycle_b = fixture::source(profiles, "cycle-b", {
      fixture::requirement(pkgsource::requirement_scope::run(),
                           "cycle-a", "requirements.run[0]"),
  });
  auto cycle_catalog = fixture::catalog(profiles, {cycle_a, cycle_b});
  const auto cycle_result = fixture::resolution(
      cycle_catalog, fixture::empty_state(),
      {fixture::package_goal(pkgsource::requirement_scope::run(), "cycle-a")});
  const auto cycle_program = compose(transaction_request::seal(cycle_result));
  TEST_CHECK(cycle_program.runtime_cohorts().size() == 1);
  TEST_CHECK(cycle_program.runtime_cohorts().front().members().size() == 2);
  TEST_CHECK(std::none_of(cycle_program.edges().begin(), cycle_program.edges().end(),
      [](const auto& edge) {
        return edge.kind() == transaction_edge_kind::requirement &&
               edge.scope() && edge.scope()->kind() ==
                   pkgsource::requirement_scope_kind::run;
      }));

  std::vector<pkgresolve::requirement_edge> construction_edges;
  unsigned int edge_seed = 1;
  for (const auto& edge : cycle_result.edges()) {
    std::string hex(64, '0');
    hex[63] = static_cast<char>('0' + edge_seed++);
    construction_edges.emplace_back(
        edge.issuer(), edge.required(), pkgsource::requirement_scope::build(),
        pkgresolve::resolution_environment::build, edge.witness(),
        pkgresolve::requirement_edge_identity::from_sha256(std::move(hex)));
  }
  const auto forged_construction_cycle = pkgresolve::resolution_result(
      cycle_result.request(), cycle_result.selections(),
      std::move(construction_edges), cycle_result.goals(),
      cycle_result.reasons(), pkgresolve::resolution_result_identity::from_sha256(
          std::string(63, 'f') + "0"));
  TEST_TRANSACTION_THROWS(error_code::construction_cycle,
      compose(transaction_request::seal(forged_construction_cycle)));
  std::vector<pkgresolve::requirement_edge> lifecycle_edges;
  edge_seed = 3;
  for (const auto& edge : cycle_result.edges()) {
    std::string hex(64, '0');
    hex[63] = static_cast<char>('0' + edge_seed++);
    lifecycle_edges.emplace_back(
        edge.issuer(), edge.required(),
        pkgsource::requirement_scope::lifecycle(
            pkgsource::lifecycle_action::pre_install),
        pkgresolve::resolution_environment::target, edge.witness(),
        pkgresolve::requirement_edge_identity::from_sha256(std::move(hex)));
  }
  const auto forged_lifecycle_cycle = pkgresolve::resolution_result(
      cycle_result.request(), cycle_result.selections(),
      std::move(lifecycle_edges), cycle_result.goals(),
      cycle_result.reasons(), pkgresolve::resolution_result_identity::from_sha256(
          std::string(63, 'e') + "0"));
  TEST_TRANSACTION_THROWS(error_code::lifecycle_cycle,
      compose(transaction_request::seal(forged_lifecycle_cycle)));
  return 0;
}

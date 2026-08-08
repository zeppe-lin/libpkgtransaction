// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#include "../fixtures/transaction.h"
#include "../support/test.h"

#include <algorithm>
#include <string>
#include <vector>

int main()
{
  using namespace pkgtransaction;

  auto profiles = fixture::profiles();
  auto cycle_a = fixture::source(profiles, "cycle-a", {
      fixture::requirement(pkgsource::requirement_scope::run(),
                           "cycle-b", "requirements.run[0]"),
  });
  auto cycle_b = fixture::source(profiles, "cycle-b", {
      fixture::requirement(pkgsource::requirement_scope::run(),
                           "cycle-a", "requirements.run[0]"),
  });
  auto catalog = fixture::catalog(profiles, {cycle_a, cycle_b});
  const auto result = fixture::resolution(
      catalog, fixture::empty_state(),
      {fixture::package_goal(pkgsource::requirement_scope::run(), "cycle-a")});
  const auto program = compose(transaction_request::seal(result));

  TEST_CHECK(program.runtime_cohorts().size() == 1);
  const auto& cohort = program.runtime_cohorts().front();
  TEST_CHECK(cohort.members().size() == 2);
  TEST_CHECK(cohort.witnesses().size() == 2);
  TEST_CHECK(std::is_sorted(cohort.members().begin(), cohort.members().end()));
  TEST_CHECK(std::is_sorted(cohort.witnesses().begin(), cohort.witnesses().end()));
  TEST_CHECK(std::none_of(program.edges().begin(), program.edges().end(),
      [](const auto& edge) {
        return edge.kind() == transaction_edge_kind::requirement &&
               edge.scope() && edge.scope()->kind() ==
                   pkgsource::requirement_scope_kind::run;
      }));

  std::vector<pkgresolve::requirement_edge> construction_edges;
  unsigned int edge_seed = 1;
  for (const auto& edge : result.edges()) {
    std::string hex(64, '0');
    hex[63] = static_cast<char>('0' + edge_seed++);
    construction_edges.emplace_back(
        edge.issuer(), edge.required(), pkgsource::requirement_scope::build(),
        pkgresolve::resolution_environment::build, edge.witness(),
        pkgresolve::requirement_edge_identity::from_sha256(std::move(hex)));
  }
  const auto construction_cycle = pkgresolve::resolution_result(
      result.request(), result.selections(), std::move(construction_edges),
      result.goals(), result.reasons(),
      pkgresolve::resolution_result_identity::from_sha256(
          std::string(63, 'f') + "0"));
  TEST_TRANSACTION_THROWS(error_code::construction_cycle,
      compose(transaction_request::seal(construction_cycle)));

  std::vector<pkgresolve::requirement_edge> lifecycle_edges;
  edge_seed = 3;
  for (const auto& edge : result.edges()) {
    std::string hex(64, '0');
    hex[63] = static_cast<char>('0' + edge_seed++);
    lifecycle_edges.emplace_back(
        edge.issuer(), edge.required(),
        pkgsource::requirement_scope::lifecycle(
            pkgsource::lifecycle_action::pre_install),
        pkgresolve::resolution_environment::target, edge.witness(),
        pkgresolve::requirement_edge_identity::from_sha256(std::move(hex)));
  }
  const auto lifecycle_cycle = pkgresolve::resolution_result(
      result.request(), result.selections(), std::move(lifecycle_edges),
      result.goals(), result.reasons(),
      pkgresolve::resolution_result_identity::from_sha256(
          std::string(63, 'e') + "0"));
  TEST_TRANSACTION_THROWS(error_code::lifecycle_cycle,
      compose(transaction_request::seal(lifecycle_cycle)));
  return 0;
}

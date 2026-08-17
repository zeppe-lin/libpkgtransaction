// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#include "../fixtures/transaction.h"
#include "../support/program_query.h"
#include "../support/test.h"

#include <algorithm>

namespace {

const pkgresolve::selected_package& selection(
    const pkgresolve::resolution_result& result,
    const char* package)
{
  const auto* value = result.find(
      pkgsource::package_reference(package),
      pkgresolve::resolution_environment::target,
      pkgresolve::selection_authority_kind::catalog_candidate);
  TEST_CHECK(value != nullptr);
  return *value;
}

const pkgresolve::requirement_edge& run_edge(
    const pkgresolve::resolution_result& result,
    const pkgresolve::selected_package& issuer,
    const pkgresolve::selected_package& required)
{
  const auto found = std::find_if(
      result.edges().begin(), result.edges().end(), [&](const auto& edge) {
        return edge.issuer() == issuer.identity() &&
               edge.required() == required.identity() &&
               edge.scope().kind() == pkgsource::requirement_scope_kind::run;
      });
  TEST_CHECK(found != result.edges().end());
  return *found;
}

bool has_witnessed_requirement(
    const pkgtransaction::transaction_program& program,
    const pkgtransaction::transaction_node& before,
    const pkgtransaction::transaction_node& after,
    const pkgresolve::requirement_edge_identity& witness)
{
  return std::any_of(
      program.edges().begin(), program.edges().end(), [&](const auto& edge) {
        return edge.kind() == pkgtransaction::transaction_edge_kind::requirement &&
               edge.before() == before.identity() &&
               edge.after() == after.identity() && edge.scope() &&
               edge.scope()->kind() == pkgsource::requirement_scope_kind::run &&
               edge.requirement_witness() == witness;
      });
}

} // namespace

int main()
{
  using namespace pkgtransaction;

  auto profiles = fixture::profiles();
  auto base = fixture::source(profiles, "base");
  auto runtime_a = fixture::source(profiles, "runtime-a", {
      fixture::requirement(pkgsource::requirement_scope::run(),
                           "runtime-b", "requirements.run[0]"),
      fixture::requirement(pkgsource::requirement_scope::run(),
                           "base", "requirements.run[1]"),
  });
  auto runtime_b = fixture::source(profiles, "runtime-b", {
      fixture::requirement(pkgsource::requirement_scope::run(),
                           "runtime-a", "requirements.run[0]"),
  });
  auto consumer = fixture::source(profiles, "consumer", {
      fixture::requirement(pkgsource::requirement_scope::run(),
                           "runtime-a", "requirements.run[0]"),
  });

  auto catalog = fixture::catalog(
      profiles, {base, runtime_a, runtime_b, consumer});
  const auto result = fixture::resolution(
      catalog, fixture::empty_state(),
      {fixture::package_goal(pkgsource::requirement_scope::run(), "consumer")});
  const auto program = compose(transaction_request::seal(result));

  const auto* base_install = test_support::find_node(
      program, "base", transaction_action_kind::install);
  const auto* a_install = test_support::find_node(
      program, "runtime-a", transaction_action_kind::install);
  const auto* b_install = test_support::find_node(
      program, "runtime-b", transaction_action_kind::install);
  const auto* consumer_install = test_support::find_node(
      program, "consumer", transaction_action_kind::install);
  TEST_CHECK(base_install && a_install && b_install && consumer_install);

  TEST_CHECK(program.runtime_cohorts().size() == 1);
  const auto& cohort = program.runtime_cohorts().front();
  TEST_CHECK(cohort.members().size() == 2);
  TEST_CHECK(std::find(cohort.members().begin(), cohort.members().end(),
                       a_install->identity()) != cohort.members().end());
  TEST_CHECK(std::find(cohort.members().begin(), cohort.members().end(),
                       b_install->identity()) != cohort.members().end());

  const auto& base_witness = run_edge(
      result, selection(result, "runtime-a"), selection(result, "base"));
  const auto& consumer_witness = run_edge(
      result, selection(result, "consumer"), selection(result, "runtime-a"));

  // A prerequisite of one SCC member gates the complete runtime cohort.
  TEST_CHECK(has_witnessed_requirement(
      program, *base_install, *a_install, base_witness.identity()));
  TEST_CHECK(has_witnessed_requirement(
      program, *base_install, *b_install, base_witness.identity()));

  // A consumer of one SCC member waits for the complete required cohort.
  TEST_CHECK(has_witnessed_requirement(
      program, *a_install, *consumer_install, consumer_witness.identity()));
  TEST_CHECK(has_witnessed_requirement(
      program, *b_install, *consumer_install, consumer_witness.identity()));

  // Internal reciprocal runtime witnesses remain cohort authority, never
  // fabricated member-to-member execution precedence.
  TEST_CHECK(!test_support::has_requirement(
      program, *a_install, *b_install,
      pkgsource::requirement_scope_kind::run));
  TEST_CHECK(!test_support::has_requirement(
      program, *b_install, *a_install,
      pkgsource::requirement_scope_kind::run));

  return 0;
}

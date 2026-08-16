// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#include "../fixtures/transaction.h"
#include "../support/program_query.h"
#include "../support/test.h"

#include <algorithm>

int main()
{
  using namespace pkgtransaction;

  auto profiles = fixture::profiles();
  auto headers = fixture::source(profiles, "cohort-headers");
  auto libc_bootstrap = fixture::source(profiles, "cohort-libc-bootstrap", {
      fixture::requirement(pkgsource::requirement_scope::build(),
                           "cohort-headers", "requirements.build[0]"),
  });
  auto libc = fixture::source(profiles, "cohort-libc", {
      fixture::requirement(pkgsource::requirement_scope::build(),
                           "cohort-headers", "requirements.build[0]"),
      fixture::requirement(pkgsource::requirement_scope::run(),
                           "cohort-libgcc", "requirements.run[0]"),
  });
  auto libgcc = fixture::source(profiles, "cohort-libgcc", {
      fixture::requirement(pkgsource::requirement_scope::build(),
                           "cohort-libc-bootstrap", "requirements.build[0]"),
      fixture::requirement(pkgsource::requirement_scope::run(),
                           "cohort-libc", "requirements.run[0]"),
  });
  auto filesystem = fixture::source(profiles, "cohort-filesystem");
  auto checker = fixture::source(profiles, "cohort-checker", {
      fixture::requirement(pkgsource::requirement_scope::build(),
                           "cohort-headers", "requirements.build[0]"),
  });
  auto probe = fixture::source(
      profiles, "cohort-probe", {
          fixture::requirement(pkgsource::requirement_scope::build(),
                               "cohort-filesystem", "requirements.build[0]"),
          fixture::requirement(pkgsource::requirement_scope::build(),
                               "cohort-libc", "requirements.build[1]"),
          fixture::requirement(pkgsource::requirement_scope::build(),
                               "cohort-libgcc", "requirements.build[2]"),
          fixture::requirement(pkgsource::requirement_scope::check(),
                               "cohort-filesystem", "requirements.check[0]"),
          fixture::requirement(pkgsource::requirement_scope::check(),
                               "cohort-libc", "requirements.check[1]"),
          fixture::requirement(pkgsource::requirement_scope::check(),
                               "cohort-libgcc", "requirements.check[2]"),
          fixture::requirement(pkgsource::requirement_scope::check(),
                               "cohort-checker", "requirements.check[3]"),
      }, {"x86_64"}, {"x86_64"}, "1.0.0", 1, {}, "true\n");

  auto catalog = fixture::catalog(
      profiles,
      {headers, libc_bootstrap, libc, libgcc, filesystem, checker, probe});
  const auto result = fixture::resolution(
      catalog, fixture::empty_state(),
      {fixture::package_goal(pkgsource::requirement_scope::check(),
                             "cohort-probe")});
  const auto program = compose(transaction_request::seal(result));

  TEST_CHECK(test_support::count_action(program, transaction_action_kind::build)
             == 7);
  TEST_CHECK(test_support::count_action(program, transaction_action_kind::check)
             == 1);
  TEST_CHECK(test_support::count_action(program, transaction_action_kind::install)
             == 0);

  const auto* headers_build = test_support::find_node(
      program, "cohort-headers", transaction_action_kind::build);
  const auto* bootstrap_build = test_support::find_node(
      program, "cohort-libc-bootstrap", transaction_action_kind::build);
  const auto* libc_build = test_support::find_node(
      program, "cohort-libc", transaction_action_kind::build);
  const auto* libgcc_build = test_support::find_node(
      program, "cohort-libgcc", transaction_action_kind::build);
  const auto* filesystem_build = test_support::find_node(
      program, "cohort-filesystem", transaction_action_kind::build);
  const auto* checker_build = test_support::find_node(
      program, "cohort-checker", transaction_action_kind::build);
  const auto* probe_build = test_support::find_node(
      program, "cohort-probe", transaction_action_kind::build);
  const auto* probe_check = test_support::find_node(
      program, "cohort-probe", transaction_action_kind::check);
  TEST_CHECK(headers_build && bootstrap_build && libc_build && libgcc_build &&
             filesystem_build && checker_build && probe_build && probe_check);

  TEST_CHECK(test_support::has_requirement(
      program, *headers_build, *bootstrap_build,
      pkgsource::requirement_scope_kind::build));
  TEST_CHECK(test_support::has_requirement(
      program, *headers_build, *libc_build,
      pkgsource::requirement_scope_kind::build));
  TEST_CHECK(test_support::has_requirement(
      program, *bootstrap_build, *libgcc_build,
      pkgsource::requirement_scope_kind::build));
  TEST_CHECK(test_support::has_requirement(
      program, *headers_build, *checker_build,
      pkgsource::requirement_scope_kind::build));

  TEST_CHECK(test_support::has_requirement(
      program, *filesystem_build, *probe_build,
      pkgsource::requirement_scope_kind::build));
  TEST_CHECK(test_support::has_requirement(
      program, *libc_build, *probe_build,
      pkgsource::requirement_scope_kind::build));
  TEST_CHECK(test_support::has_requirement(
      program, *libgcc_build, *probe_build,
      pkgsource::requirement_scope_kind::build));

  TEST_CHECK(test_support::has_requirement(
      program, *filesystem_build, *probe_check,
      pkgsource::requirement_scope_kind::check));
  TEST_CHECK(test_support::has_requirement(
      program, *libc_build, *probe_check,
      pkgsource::requirement_scope_kind::check));
  TEST_CHECK(test_support::has_requirement(
      program, *libgcc_build, *probe_check,
      pkgsource::requirement_scope_kind::check));
  TEST_CHECK(test_support::has_requirement(
      program, *checker_build, *probe_check,
      pkgsource::requirement_scope_kind::check));
  TEST_CHECK(test_support::has_phase(
      program, *probe_build, *probe_check,
      phase_order_kind::build_before_check));

  TEST_CHECK(program.runtime_cohorts().size() == 1);
  const auto& cohort = program.runtime_cohorts().front();
  TEST_CHECK(cohort.members().size() == 2);
  TEST_CHECK(std::find(cohort.members().begin(), cohort.members().end(),
                       libc_build->identity()) != cohort.members().end());
  TEST_CHECK(std::find(cohort.members().begin(), cohort.members().end(),
                       libgcc_build->identity()) != cohort.members().end());

  // Reciprocal run authority is represented by the cohort, never by
  // executable precedence. Otherwise the build graph would become cyclic.
  TEST_CHECK(std::none_of(
      program.edges().begin(), program.edges().end(), [&](const auto& edge) {
        if (edge.kind() != transaction_edge_kind::requirement || !edge.scope() ||
            edge.scope()->kind() != pkgsource::requirement_scope_kind::run)
          return false;
        return (edge.before() == libc_build->identity() &&
                edge.after() == libgcc_build->identity()) ||
               (edge.before() == libgcc_build->identity() &&
                edge.after() == libc_build->identity());
      }));

  // Check authority must never be pulled backward onto the probe build.
  TEST_CHECK(std::none_of(
      program.edges().begin(), program.edges().end(), [&](const auto& edge) {
        return edge.kind() == transaction_edge_kind::requirement &&
               edge.after() == probe_build->identity() && edge.scope() &&
               edge.scope()->kind() == pkgsource::requirement_scope_kind::check;
      }));

  return 0;
}

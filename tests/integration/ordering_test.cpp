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
  auto app = fixture::source(profiles, "app", {
      fixture::requirement(pkgsource::requirement_scope::run(),
                           "runtime", "requirements.run[0]"),
      fixture::requirement(pkgsource::requirement_scope::build(),
                           "tester", "requirements.build[0]"),
      fixture::requirement(pkgsource::requirement_scope::check(),
                           "tester", "requirements.check[0]"),
  }, {"x86_64"}, {"x86_64"}, "1.0.0", 1, {}, "printf 'checked\\n'\n");
  auto runtime = fixture::source(profiles, "runtime");
  auto tester = fixture::source(profiles, "tester");
  auto catalog = fixture::catalog(profiles, {app, runtime, tester});

  const auto result = fixture::resolution(
      catalog, fixture::empty_state(), {
          fixture::package_goal(pkgsource::requirement_scope::run(), "app"),
          fixture::package_goal(pkgsource::requirement_scope::check(), "app"),
      });
  const auto program = compose(transaction_request::seal(result));

  const auto* app_build = test_support::find_node(
      program, "app", transaction_action_kind::build);
  const auto* app_check = test_support::find_node(
      program, "app", transaction_action_kind::check);
  const auto* app_install = test_support::find_node(
      program, "app", transaction_action_kind::install);
  const auto* runtime_install = test_support::find_node(
      program, "runtime", transaction_action_kind::install);
  const auto* tester_build = test_support::find_node(
      program, "tester", transaction_action_kind::build);
  TEST_CHECK(app_build && app_check && app_install && runtime_install &&
             tester_build);

  TEST_CHECK(test_support::has_phase(
      program, *app_build, *app_check, phase_order_kind::build_before_check));
  TEST_CHECK(test_support::has_phase(
      program, *app_build, *app_install, phase_order_kind::build_before_target));
  TEST_CHECK(test_support::has_phase(
      program, *app_check, *app_install, phase_order_kind::check_before_target));

  TEST_CHECK(test_support::has_requirement(
      program, *tester_build, *app_build,
      pkgsource::requirement_scope_kind::build));
  TEST_CHECK(test_support::has_requirement(
      program, *tester_build, *app_check,
      pkgsource::requirement_scope_kind::check));
  TEST_CHECK(test_support::has_requirement(
      program, *runtime_install, *app_install,
      pkgsource::requirement_scope_kind::run));

  TEST_CHECK(std::none_of(
      program.edges().begin(), program.edges().end(), [&](const auto& edge) {
        return edge.kind() == transaction_edge_kind::requirement &&
               edge.after() == app_build->identity() && edge.scope() &&
               edge.scope()->kind() == pkgsource::requirement_scope_kind::check;
      }));
  return 0;
}

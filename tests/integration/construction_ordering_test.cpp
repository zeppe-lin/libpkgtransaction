// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#include "../fixtures/transaction.h"
#include "../support/program_query.h"
#include "../support/test.h"

int main()
{
  using namespace pkgtransaction;

  auto profiles = fixture::profiles();
  auto app = fixture::source(profiles, "app", {
      fixture::requirement(pkgsource::requirement_scope::build(),
                           "builder", "requirements.build[0]"),
  });
  auto builder = fixture::source(profiles, "builder");
  auto catalog = fixture::catalog(profiles, {app, builder});
  const auto build_goal = fixture::package_goal(
      pkgsource::requirement_scope::build(), "app");

  const auto fresh_result = fixture::resolution(
      catalog, fixture::empty_state(), {build_goal});
  const auto fresh_program = compose(transaction_request::seal(fresh_result));
  const auto* fresh_app_build = test_support::find_node(
      fresh_program, "app", transaction_action_kind::build);
  const auto* fresh_builder_build = test_support::find_node(
      fresh_program, "builder", transaction_action_kind::build);
  TEST_CHECK(fresh_app_build && fresh_builder_build);
  TEST_CHECK(fresh_builder_build->environment() ==
             pkgresolve::resolution_environment::build);
  TEST_CHECK(test_support::has_requirement(
      fresh_program, *fresh_builder_build, *fresh_app_build,
      pkgsource::requirement_scope_kind::build));
  TEST_CHECK(test_support::count_action(
      fresh_program, transaction_action_kind::install) == 0);

  const auto binding = fixture::target();
  const auto installed_builder = fixture::installed_package(builder, binding, 70);
  const auto installed_result = fixture::resolution(
      catalog, fixture::state({installed_builder}, binding), {build_goal});
  const auto installed_program = compose(
      transaction_request::seal(installed_result));
  const auto* installed_app_build = test_support::find_node(
      installed_program, "app", transaction_action_kind::build);
  const auto* retained_builder = test_support::find_node(
      installed_program, "builder", transaction_action_kind::retain);
  TEST_CHECK(installed_app_build && retained_builder);
  TEST_CHECK(retained_builder->environment() ==
             pkgresolve::resolution_environment::build);
  TEST_CHECK(retained_builder->selection() &&
             retained_builder->selection()->installed());
  TEST_CHECK(test_support::find_node(
      installed_program, "builder", transaction_action_kind::build) == nullptr);
  TEST_CHECK(test_support::has_requirement(
      installed_program, *retained_builder, *installed_app_build,
      pkgsource::requirement_scope_kind::build));
  return 0;
}

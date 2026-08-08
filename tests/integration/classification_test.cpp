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
      fixture::requirement(pkgsource::requirement_scope::run(),
                           "lib", "requirements.run[0]"),
  });
  auto lib = fixture::source(profiles, "lib", {
      fixture::requirement(pkgsource::requirement_scope::run(),
                           "runtime", "requirements.run[0]"),
  });
  auto runtime = fixture::source(profiles, "runtime");
  auto stale = fixture::source(profiles, "stale");
  auto catalog = fixture::catalog(profiles, {app, lib, runtime, stale});

  const auto run_result = fixture::resolution(
      catalog, fixture::empty_state(),
      {fixture::package_goal(pkgsource::requirement_scope::run(), "app")});
  const auto run_program = compose(transaction_request::seal(run_result));
  TEST_CHECK(test_support::count_action(run_program,
      transaction_action_kind::build) == 3);
  TEST_CHECK(test_support::count_action(run_program,
      transaction_action_kind::install) == 3);
  TEST_CHECK(test_support::count_action(run_program,
      transaction_action_kind::retain) == 0);
  TEST_CHECK(test_support::count_action(run_program,
      transaction_action_kind::remove) == 0);

  const auto build_result = fixture::resolution(
      catalog, fixture::empty_state(),
      {fixture::package_goal(pkgsource::requirement_scope::build(), "app")});
  const auto build_program = compose(transaction_request::seal(build_result));
  TEST_CHECK(test_support::count_action(build_program,
      transaction_action_kind::build) >= 1);
  TEST_CHECK(test_support::count_action(build_program,
      transaction_action_kind::install) == 0);
  TEST_CHECK(test_support::count_action(build_program,
      transaction_action_kind::upgrade) == 0);

  const auto binding = fixture::target();
  const auto installed_app = fixture::installed_package(app, binding, 60);
  const auto installed_stale = fixture::installed_package(stale, binding, 90);
  const auto installed_state = fixture::state(
      {installed_app, installed_stale}, binding);
  const auto retained_result = fixture::resolution(
      catalog, installed_state,
      {fixture::package_goal(pkgsource::requirement_scope::run(), "app")});

  const auto preserved_program = compose(transaction_request::seal(retained_result));
  TEST_CHECK(test_support::count_action(preserved_program,
      transaction_action_kind::retain) >= 1);
  TEST_CHECK(test_support::find_node(
      preserved_program, "app", transaction_action_kind::build) == nullptr);
  TEST_CHECK(test_support::count_action(preserved_program,
      transaction_action_kind::remove) == 0);

  const auto exact_program = compose(transaction_request::seal(
      retained_result, convergence_policy::converge_exact()));
  TEST_CHECK(test_support::count_action(exact_program,
      transaction_action_kind::retain) >= 1);
  TEST_CHECK(test_support::count_action(exact_program,
      transaction_action_kind::remove) == 1);
  const auto* removed = test_support::find_node(
      exact_program, "stale", transaction_action_kind::remove);
  TEST_CHECK(removed != nullptr);
  TEST_CHECK(removed->installed() != nullptr);
  TEST_CHECK(removed->selection() == nullptr);

  auto changed_app = fixture::source(
      profiles, "app", {}, {"x86_64"}, {"x86_64"}, "1.0.1");
  auto changed_catalog = fixture::catalog(profiles, {changed_app});
  const auto upgrade_result = fixture::resolution(
      changed_catalog, fixture::state({installed_app}, binding),
      {fixture::package_goal(pkgsource::requirement_scope::run(), "app")},
      pkgresolve::installed_preference::prefer_catalog);
  const auto upgrade_program = compose(transaction_request::seal(upgrade_result));
  TEST_CHECK(test_support::count_action(upgrade_program,
      transaction_action_kind::upgrade) == 1);
  TEST_CHECK(test_support::count_action(upgrade_program,
      transaction_action_kind::build) == 1);
  TEST_CHECK(test_support::count_action(upgrade_program,
      transaction_action_kind::remove) == 0);

  auto source_changed_app = fixture::source(
      profiles, "app", {}, {"x86_64"}, {"x86_64"}, "1.0.0", 1, {},
      "printf 'new source\\n'\n");
  auto source_changed_catalog = fixture::catalog(profiles, {source_changed_app});
  const auto source_changed_result = fixture::resolution(
      source_changed_catalog, fixture::state({installed_app}, binding),
      {fixture::package_goal(pkgsource::requirement_scope::run(), "app")},
      pkgresolve::installed_preference::prefer_catalog);
  const auto source_changed_program = compose(
      transaction_request::seal(source_changed_result));
  TEST_CHECK(test_support::count_action(source_changed_program,
      transaction_action_kind::upgrade) == 1);
  TEST_CHECK(test_support::find_node(
      source_changed_program, "app", transaction_action_kind::retain) == nullptr);
  return 0;
}

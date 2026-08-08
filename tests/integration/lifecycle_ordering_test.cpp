// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#include "../fixtures/transaction.h"
#include "../support/program_query.h"
#include "../support/test.h"

int main()
{
  using namespace pkgtransaction;

  auto profiles = fixture::profiles();
  const auto pre_install = pkgsource::requirement_scope::lifecycle(
      pkgsource::lifecycle_action::pre_install);
  auto app = fixture::source(profiles, "app", {
      fixture::requirement(pre_install, "helper",
                           "requirements.lifecycle.pre-install[0]"),
  }, {"x86_64"}, {"x86_64"}, "1.0.0", 1,
      {pkgsource::lifecycle_action::pre_install});
  auto helper = fixture::source(profiles, "helper");
  auto catalog = fixture::catalog(profiles, {app, helper});

  const auto result = fixture::resolution(
      catalog, fixture::empty_state(), {
          fixture::package_goal(pkgsource::requirement_scope::run(), "app"),
          fixture::package_goal(pre_install, "app"),
      });
  const auto program = compose(transaction_request::seal(result));
  const auto* app_install = test_support::find_node(
      program, "app", transaction_action_kind::install);
  const auto* app_pre = test_support::find_node(
      program, "app", transaction_action_kind::lifecycle,
      pkgsource::lifecycle_action::pre_install);
  const auto* helper_install = test_support::find_node(
      program, "helper", transaction_action_kind::install);
  TEST_CHECK(app_install && app_pre && helper_install);
  TEST_CHECK(test_support::has_requirement(
      program, *helper_install, *app_pre,
      pkgsource::requirement_scope_kind::lifecycle));
  TEST_CHECK(test_support::has_phase(
      program, *app_pre, *app_install,
      phase_order_kind::pre_lifecycle_before_action));
  return 0;
}

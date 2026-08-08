// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#include "../fixtures/transaction.h"
#include "../support/program_query.h"
#include "../support/test.h"

int main()
{
  using namespace pkgtransaction;

  auto profiles = fixture::profiles();
  auto app = fixture::source(
      profiles, "app", {}, {"x86_64"}, {"x86_64"}, "1.0.0", 1,
      {pkgsource::lifecycle_action::pre_install,
       pkgsource::lifecycle_action::post_install,
       pkgsource::lifecycle_action::pre_remove,
       pkgsource::lifecycle_action::post_remove});
  auto catalog = fixture::catalog(profiles, {app});
  const auto binding = fixture::target();
  const auto installed_app = fixture::installed_package(app, binding, 60);
  const auto installed_state = fixture::state({installed_app}, binding);

  const auto pre_install = pkgsource::requirement_scope::lifecycle(
      pkgsource::lifecycle_action::pre_install);
  const auto post_install = pkgsource::requirement_scope::lifecycle(
      pkgsource::lifecycle_action::post_install);
  const auto install_lifecycle_result = fixture::resolution(
      catalog, fixture::empty_state(binding), {
          fixture::package_goal(pkgsource::requirement_scope::run(), "app"),
          fixture::package_goal(pre_install, "app"),
          fixture::package_goal(post_install, "app"),
      });
  const auto install_lifecycle_program = compose(
      transaction_request::seal(install_lifecycle_result));
  const auto* install = test_support::find_node(
      install_lifecycle_program, "app", transaction_action_kind::install);
  const auto* install_pre = test_support::find_node(
      install_lifecycle_program, "app", transaction_action_kind::lifecycle,
      pkgsource::lifecycle_action::pre_install);
  const auto* install_post = test_support::find_node(
      install_lifecycle_program, "app", transaction_action_kind::lifecycle,
      pkgsource::lifecycle_action::post_install);
  TEST_CHECK(install && install_pre && install_post);
  TEST_CHECK(test_support::has_phase(
      install_lifecycle_program, *install_pre, *install,
      phase_order_kind::pre_lifecycle_before_action));
  TEST_CHECK(test_support::has_phase(
      install_lifecycle_program, *install, *install_post,
      phase_order_kind::action_before_post_lifecycle));

  const auto pre_remove = pkgsource::requirement_scope::lifecycle(
      pkgsource::lifecycle_action::pre_remove);
  const auto post_remove = pkgsource::requirement_scope::lifecycle(
      pkgsource::lifecycle_action::post_remove);
  const auto removal_result = fixture::resolution(
      catalog, installed_state, {
          fixture::package_goal(pre_remove, "app"),
          fixture::package_goal(post_remove, "app"),
      });
  TEST_TRANSACTION_THROWS(error_code::unbound_lifecycle,
      compose(transaction_request::seal(removal_result)));

  const auto removal_program = compose(transaction_request::seal(
      removal_result, convergence_policy::remove_explicit(
          {pkgsource::package_reference("app")})));
  const auto* remove = test_support::find_node(
      removal_program, "app", transaction_action_kind::remove);
  const auto* pre = test_support::find_node(
      removal_program, "app", transaction_action_kind::lifecycle,
      pkgsource::lifecycle_action::pre_remove);
  const auto* post = test_support::find_node(
      removal_program, "app", transaction_action_kind::lifecycle,
      pkgsource::lifecycle_action::post_remove);
  TEST_CHECK(remove && pre && post);
  TEST_CHECK(remove->installed() != nullptr);
  TEST_CHECK(pre->selection() && pre->selection()->installed());
  TEST_CHECK(post->selection() && post->selection()->installed());
  TEST_CHECK(test_support::has_phase(
      removal_program, *pre, *remove,
      phase_order_kind::pre_lifecycle_before_action));
  TEST_CHECK(test_support::has_phase(
      removal_program, *remove, *post,
      phase_order_kind::action_before_post_lifecycle));

  auto incoming = fixture::source(
      profiles, "upgrade-app", {}, {"x86_64"}, {"x86_64"}, "1.0.0", 1,
      {pkgsource::lifecycle_action::pre_install,
       pkgsource::lifecycle_action::post_install});
  auto historical = fixture::source(
      profiles, "upgrade-app", {}, {"x86_64"}, {"x86_64"}, "0.9.0", 1,
      {pkgsource::lifecycle_action::pre_remove,
       pkgsource::lifecycle_action::post_remove});
  const auto upgrade_catalog = fixture::catalog(profiles, {incoming});
  const auto historical_installed = fixture::installed_package(
      historical, binding, 100);
  const auto upgrade_state = fixture::state({historical_installed}, binding);
  const auto upgrade_result = fixture::resolution(
      upgrade_catalog, upgrade_state, {
          fixture::package_goal(pkgsource::requirement_scope::run(), "upgrade-app"),
          fixture::package_goal(pre_remove, "upgrade-app"),
          fixture::package_goal(post_remove, "upgrade-app"),
          fixture::package_goal(pre_install, "upgrade-app"),
          fixture::package_goal(post_install, "upgrade-app"),
      }, pkgresolve::installed_preference::prefer_catalog);
  const auto upgrade_program = compose(transaction_request::seal(upgrade_result));

  const auto* upgrade = test_support::find_node(
      upgrade_program, "upgrade-app", transaction_action_kind::upgrade);
  const auto* old_pre = test_support::find_node(
      upgrade_program, "upgrade-app", transaction_action_kind::lifecycle,
      pkgsource::lifecycle_action::pre_remove);
  const auto* old_post = test_support::find_node(
      upgrade_program, "upgrade-app", transaction_action_kind::lifecycle,
      pkgsource::lifecycle_action::post_remove);
  const auto* new_pre = test_support::find_node(
      upgrade_program, "upgrade-app", transaction_action_kind::lifecycle,
      pkgsource::lifecycle_action::pre_install);
  const auto* new_post = test_support::find_node(
      upgrade_program, "upgrade-app", transaction_action_kind::lifecycle,
      pkgsource::lifecycle_action::post_install);
  TEST_CHECK(upgrade && old_pre && old_post && new_pre && new_post);
  TEST_CHECK(upgrade->selection() && upgrade->selection()->candidate());
  TEST_CHECK(old_pre->selection() && old_pre->selection()->installed());
  TEST_CHECK(old_post->selection() && old_post->selection()->installed());
  TEST_CHECK(new_pre->selection() && new_pre->selection()->candidate());
  TEST_CHECK(new_post->selection() && new_post->selection()->candidate());
  TEST_CHECK(test_support::has_phase(
      upgrade_program, *old_pre, *upgrade,
      phase_order_kind::pre_lifecycle_before_action));
  TEST_CHECK(test_support::has_phase(
      upgrade_program, *new_pre, *upgrade,
      phase_order_kind::pre_lifecycle_before_action));
  TEST_CHECK(test_support::has_phase(
      upgrade_program, *upgrade, *old_post,
      phase_order_kind::action_before_post_lifecycle));
  TEST_CHECK(test_support::has_phase(
      upgrade_program, *upgrade, *new_post,
      phase_order_kind::action_before_post_lifecycle));
  TEST_CHECK(test_support::count_action(
      upgrade_program, transaction_action_kind::remove) == 0);
  return 0;
}

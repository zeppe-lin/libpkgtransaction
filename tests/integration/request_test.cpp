// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#include "../fixtures/transaction.h"
#include "../support/test.h"

int main()
{
  using namespace pkgtransaction;
  auto profiles = fixture::profiles();
  auto app = fixture::source(profiles, "app");
  auto data = fixture::source(profiles, "data");
  auto catalog = fixture::catalog(profiles, {app, data});
  const auto binding = fixture::target();
  const auto installed_data = fixture::installed_package(data, binding, 50);
  const auto installed_app = fixture::installed_package(app, binding, 80);
  const auto state = fixture::state({installed_app, installed_data}, binding);
  const auto result = fixture::resolution(
      catalog, state,
      {fixture::package_goal(pkgsource::requirement_scope::run(), "app")});

  const auto preserve = transaction_request::seal(result);
  TEST_CHECK(preserve.policy().mode() == convergence_mode::preserve_unselected);
  TEST_CHECK(preserve.resolution().identity() == result.identity());
  const auto exact = transaction_request::seal(
      result, convergence_policy::converge_exact());
  TEST_CHECK(exact.identity() != preserve.identity());

  const auto remove_a = transaction_request::seal(
      result, convergence_policy::remove_explicit({
          pkgsource::package_reference("data"),
      }));
  const auto remove_b = transaction_request::seal(
      result, convergence_policy::remove_explicit({
          pkgsource::package_reference("data"),
      }));
  TEST_CHECK(remove_a.identity() == remove_b.identity());
  TEST_CHECK(remove_a.policy().removals().size() == 1);

  TEST_TRANSACTION_THROWS(error_code::unknown_installed_package,
      transaction_request::seal(result,
          convergence_policy::remove_explicit(
              {pkgsource::package_reference("missing")})));
  TEST_TRANSACTION_THROWS(error_code::selected_for_removal,
      transaction_request::seal(result,
          convergence_policy::remove_explicit(
              {pkgsource::package_reference("app")})));

  const auto pre_remove = pkgsource::requirement_scope::lifecycle(
      pkgsource::lifecycle_action::pre_remove);
  const auto removal_only = fixture::resolution(
      catalog, state, {fixture::package_goal(pre_remove, "app")});
  const auto admitted_removal = transaction_request::seal(
      removal_only, convergence_policy::remove_explicit(
          {pkgsource::package_reference("app")}));
  TEST_CHECK(admitted_removal.policy().mode() ==
             convergence_mode::remove_explicit);

  auto removal_app = fixture::source(profiles, "removal-app", {
      fixture::requirement(
          pkgsource::requirement_scope::lifecycle(
              pkgsource::lifecycle_action::pre_remove),
          "cleanup", "requirements.lifecycle.pre-remove[0]"),
  });
  auto cleanup = fixture::source(profiles, "cleanup");
  auto removal_catalog = fixture::catalog(profiles, {removal_app, cleanup});
  const auto installed_removal_app = fixture::installed_package(
      removal_app, binding, 100);
  const auto installed_cleanup = fixture::installed_package(cleanup, binding, 120);
  const auto removal_dependency_result = fixture::resolution(
      removal_catalog,
      fixture::state({installed_removal_app, installed_cleanup}, binding),
      {fixture::package_goal(pre_remove, "removal-app")});
  TEST_TRANSACTION_THROWS(error_code::selected_for_removal,
      transaction_request::seal(
          removal_dependency_result, convergence_policy::remove_explicit(
              {pkgsource::package_reference("cleanup")})));
  return 0;
}

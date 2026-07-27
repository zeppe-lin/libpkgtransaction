// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#include "fixture_transaction.h"
#include "test.h"

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
  const auto exact = transaction_request::seal(
      result, convergence_policy::converge_exact());
  TEST_CHECK(exact.identity() != preserve.identity());
  const auto remove = transaction_request::seal(
      result, convergence_policy::remove_explicit(
          {pkgsource::package_reference("data")}));
  TEST_CHECK(remove.policy().removals().size() == 1);

  TEST_TRANSACTION_THROWS(error_code::duplicate_removal,
      convergence_policy::remove_explicit({
          pkgsource::package_reference("data"),
          pkgsource::package_reference("data")}));
  TEST_TRANSACTION_THROWS(error_code::unknown_installed_package,
      transaction_request::seal(result,
          convergence_policy::remove_explicit(
              {pkgsource::package_reference("missing")})));
  TEST_TRANSACTION_THROWS(error_code::selected_for_removal,
      transaction_request::seal(result,
          convergence_policy::remove_explicit(
              {pkgsource::package_reference("app")})));
  return 0;
}

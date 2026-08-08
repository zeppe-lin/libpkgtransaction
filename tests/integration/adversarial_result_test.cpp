// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#include "../fixtures/transaction.h"
#include "../support/test.h"

#include <string>
#include <vector>

int main()
{
  using namespace pkgtransaction;

  auto profiles = fixture::profiles();
  auto app = fixture::source(profiles, "app", {
      fixture::requirement(pkgsource::requirement_scope::run(),
                           "dep", "requirements.run[0]"),
  });
  auto dep = fixture::source(profiles, "dep");
  auto catalog = fixture::catalog(profiles, {app, dep});
  const auto result = fixture::resolution(
      catalog, fixture::empty_state(),
      {fixture::package_goal(pkgsource::requirement_scope::run(), "app")});

  auto forged_goals = result.goals();
  TEST_CHECK(!forged_goals.empty());
  TEST_CHECK(!forged_goals.front().members().empty());
  auto forged_members = forged_goals.front().members();
  forged_members.front() = pkgresolve::goal_member(
      forged_members.front().package(),
      pkgresolve::package_selection_identity::from_sha256(std::string(64, 'f')),
      forged_members.front().profile(), forged_members.front().expansion());
  forged_goals.front() = pkgresolve::resolved_goal(
      forged_goals.front().goal(), std::move(forged_members),
      forged_goals.front().selections(), forged_goals.front().edges(),
      pkgresolve::goal_closure_identity::from_sha256(std::string(64, 'e')));
  const auto unknown_goal_selection = pkgresolve::resolution_result(
      result.request(), result.selections(), result.edges(),
      std::move(forged_goals), result.reasons(),
      pkgresolve::resolution_result_identity::from_sha256(std::string(64, 'd')));
  TEST_TRANSACTION_THROWS(error_code::inconsistent_authority,
      compose(transaction_request::seal(unknown_goal_selection)));

  return 0;
}

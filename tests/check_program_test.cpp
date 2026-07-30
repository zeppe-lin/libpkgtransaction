// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#include "fixture_transaction.h"
#include "test.h"

#include <algorithm>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

const pkgtransaction::transaction_node& build_node(
    const pkgtransaction::transaction_program& program,
    std::string_view package)
{
  const auto found = std::find_if(
      program.nodes().begin(), program.nodes().end(), [&](const auto& node) {
        return node.action() ==
                   pkgtransaction::transaction_action_kind::build &&
               node.package().name() == package;
      });
  TEST_CHECK(found != program.nodes().end());
  return *found;
}

const pkgtransaction::transaction_node& check_node(
    const pkgtransaction::transaction_program& program)
{
  const auto found = std::find_if(
      program.nodes().begin(), program.nodes().end(), [](const auto& node) {
        return node.action() ==
            pkgtransaction::transaction_action_kind::check;
      });
  TEST_CHECK(found != program.nodes().end());
  return *found;
}

} // namespace

int main()
{
  using namespace pkgtransaction;

  auto profiles = fixture::profiles();
  const auto check_requirement = fixture::requirement(
      pkgsource::requirement_scope::check(), "tester",
      "requirements.check[0]");
  auto checked = fixture::source(
      profiles, "checked", {check_requirement}, {"x86_64"}, {"x86_64"},
      "1.0.0", 1, {}, "printf 'checked\\n'\n");
  auto tester = fixture::source(profiles, "tester");
  auto catalog = fixture::catalog(profiles, {checked, tester});

  const auto result = fixture::resolution(
      catalog, fixture::empty_state(),
      {fixture::package_goal(pkgsource::requirement_scope::check(),
                             "checked")});
  const auto program = compose(transaction_request::seal(result));
  TEST_CHECK(fixture::count_action(program, transaction_action_kind::build) == 2);
  TEST_CHECK(fixture::count_action(program, transaction_action_kind::check) == 1);
  TEST_CHECK(fixture::count_action(program, transaction_action_kind::install) == 0);

  const auto& check = check_node(program);
  TEST_CHECK(check.selection());
  TEST_CHECK(check.selection()->candidate());
  TEST_CHECK(check.check_program());
  TEST_CHECK(check.check_program()->language() ==
             pkgsource::program_language::posix_shell);
  TEST_CHECK(check.check_program()->material() == "printf 'checked\\n'\n");
  TEST_CHECK(check.selection()->candidate()->source().identity() ==
             check.selection()->source_snapshot());
  TEST_CHECK(std::all_of(
      program.nodes().begin(), program.nodes().end(), [&](const auto& node) {
        return node.action() == transaction_action_kind::check ||
               node.check_program() == nullptr;
      }));

  const auto build_before_check = std::any_of(
      program.edges().begin(), program.edges().end(), [&](const auto& edge) {
        return edge.kind() == transaction_edge_kind::phase &&
               edge.after() == check.identity() &&
               edge.phase_order() == phase_order_kind::build_before_check;
      });
  TEST_CHECK(build_before_check);
  const auto& checked_build = build_node(program, "checked");
  const auto& tester_build = build_node(program, "tester");
  TEST_CHECK(std::any_of(
      program.edges().begin(), program.edges().end(), [&](const auto& edge) {
        return edge.kind() == transaction_edge_kind::requirement &&
               edge.before() == tester_build.identity() &&
               edge.after() == checked_build.identity() && edge.scope() &&
               edge.scope()->kind() ==
                   pkgsource::requirement_scope_kind::check;
      }));
  TEST_CHECK(std::none_of(
      program.edges().begin(), program.edges().end(), [&](const auto& edge) {
        return edge.kind() == transaction_edge_kind::requirement &&
               edge.after() == check.identity() && edge.scope() &&
               edge.scope()->kind() ==
                   pkgsource::requirement_scope_kind::check;
      }));

  auto unchecked = fixture::source(
      profiles, "unchecked", {fixture::requirement(
          pkgsource::requirement_scope::check(), "tester",
          "requirements.check[0]")});
  auto unchecked_catalog = fixture::catalog(profiles, {unchecked, tester});
  const auto unchecked_result = fixture::resolution(
      unchecked_catalog, fixture::empty_state(),
      {fixture::package_goal(pkgsource::requirement_scope::check(),
                             "unchecked")});
  TEST_TRANSACTION_THROWS(error_code::missing_check_program,
      compose(transaction_request::seal(unchecked_result)));

  auto changed = fixture::source(
      profiles, "checked", {check_requirement}, {"x86_64"}, {"x86_64"},
      "1.0.0", 1, {}, "printf 'changed\\n'\n");
  auto changed_catalog = fixture::catalog(profiles, {changed, tester});
  const auto changed_result = fixture::resolution(
      changed_catalog, fixture::empty_state(),
      {fixture::package_goal(pkgsource::requirement_scope::check(),
                             "checked")});
  const auto changed_program = compose(
      transaction_request::seal(changed_result));
  TEST_CHECK(program.request().identity() != changed_program.request().identity());
  TEST_CHECK(program.identity() != changed_program.identity());
  TEST_CHECK(check.identity() != check_node(changed_program).identity());

  std::vector<pkgresolve::selected_package> forged_selections =
      result.selections();
  const auto selected = std::find_if(
      forged_selections.begin(), forged_selections.end(), [](const auto& value) {
        return value.package().name() == "checked" &&
               value.environment() ==
                   pkgresolve::resolution_environment::target;
      });
  TEST_CHECK(selected != forged_selections.end());
  *selected = pkgresolve::selected_package(
      selected->environment(), selected->architectures(), selected->authority(),
      selected->release(),
      pkgsource::source_snapshot_identity::from_sha256(std::string(64, 'f')),
      selected->identity());
  const auto forged = pkgresolve::resolution_result(
      result.request(), std::move(forged_selections), result.edges(),
      result.goals(), result.reasons(),
      pkgresolve::resolution_result_identity::from_sha256(
          std::string(63, 'f') + "0"));
  TEST_TRANSACTION_THROWS(error_code::inconsistent_authority,
      compose(transaction_request::seal(forged)));

  return 0;
}
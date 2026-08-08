// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#include "../fixtures/transaction.h"
#include "../support/program_query.h"
#include "../support/test.h"

#include <algorithm>
#include <string>

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
  const auto request = transaction_request::seal(result);
  const auto left = compose(request);
  const auto right = compose(request);

  TEST_CHECK(left.identity() == right.identity());
  TEST_CHECK(left.request().identity() == request.identity());
  TEST_CHECK(std::is_sorted(
      left.nodes().begin(), left.nodes().end(), [](const auto& lhs, const auto& rhs) {
        return lhs.identity() < rhs.identity();
      }));
  TEST_CHECK(std::is_sorted(
      left.edges().begin(), left.edges().end(), [](const auto& lhs, const auto& rhs) {
        return lhs.identity() < rhs.identity();
      }));

  for (const auto& node : left.nodes()) {
    const auto* found = left.find(node.identity());
    TEST_CHECK(found != nullptr);
    TEST_CHECK(found->identity() == node.identity());
  }
  TEST_CHECK(left.find(transaction_node_identity::from_sha256(
      std::string(64, 'f'))) == nullptr);

  const auto app_nodes = left.nodes_for(pkgsource::package_reference("app"));
  TEST_CHECK(app_nodes.size() == 2);
  TEST_CHECK(std::all_of(app_nodes.begin(), app_nodes.end(), [](const auto* node) {
    return node != nullptr && node->package().name() == "app";
  }));
  TEST_CHECK(left.nodes_for(pkgsource::package_reference("missing")).empty());

  for (const auto& edge : left.edges()) {
    TEST_CHECK(edge.before() != edge.after());
    TEST_CHECK(left.find(edge.before()) != nullptr);
    TEST_CHECK(left.find(edge.after()) != nullptr);
    if (edge.kind() == transaction_edge_kind::requirement) {
      TEST_CHECK(edge.scope().has_value());
      TEST_CHECK(edge.requirement_witness().has_value());
      TEST_CHECK(!edge.phase_order().has_value());
    } else {
      TEST_CHECK(!edge.scope().has_value());
      TEST_CHECK(!edge.requirement_witness().has_value());
      TEST_CHECK(edge.phase_order().has_value());
    }
  }
  return 0;
}

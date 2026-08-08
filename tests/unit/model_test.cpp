// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#include "../support/test.h"

#include <libpkgtransaction/libpkgtransaction.h>

#include <vector>

int main()
{
  using namespace pkgtransaction;

  TEST_CHECK(to_string(convergence_mode::preserve_unselected) ==
             "preserve-unselected");
  TEST_CHECK(to_string(convergence_mode::remove_explicit) == "remove-explicit");
  TEST_CHECK(to_string(convergence_mode::converge_exact) == "converge-exact");

  TEST_CHECK(to_string(transaction_action_kind::build) == "build");
  TEST_CHECK(to_string(transaction_action_kind::check) == "check");
  TEST_CHECK(to_string(transaction_action_kind::install) == "install");
  TEST_CHECK(to_string(transaction_action_kind::upgrade) == "upgrade");
  TEST_CHECK(to_string(transaction_action_kind::retain) == "retain");
  TEST_CHECK(to_string(transaction_action_kind::remove) == "remove");
  TEST_CHECK(to_string(transaction_action_kind::lifecycle) == "lifecycle");
  TEST_CHECK(to_string(transaction_edge_kind::requirement) == "requirement");
  TEST_CHECK(to_string(transaction_edge_kind::phase) == "phase");
  TEST_CHECK(to_string(phase_order_kind::build_before_check) ==
             "build-before-check");
  TEST_CHECK(to_string(phase_order_kind::build_before_target) ==
             "build-before-target");
  TEST_CHECK(to_string(phase_order_kind::check_before_target) ==
             "check-before-target");
  TEST_CHECK(to_string(phase_order_kind::pre_lifecycle_before_action) ==
             "pre-lifecycle-before-action");
  TEST_CHECK(to_string(phase_order_kind::action_before_post_lifecycle) ==
             "action-before-post-lifecycle");

  const auto preserve = convergence_policy::preserve_unselected();
  const auto exact = convergence_policy::converge_exact();
  TEST_CHECK(preserve.mode() == convergence_mode::preserve_unselected);
  TEST_CHECK(preserve.removals().empty());
  TEST_CHECK(exact.mode() == convergence_mode::converge_exact);
  TEST_CHECK(exact.removals().empty());
  TEST_CHECK(preserve != exact);

  const auto removals = convergence_policy::remove_explicit({
      pkgsource::package_reference("zeta"),
      pkgsource::package_reference("alpha"),
  });
  TEST_CHECK(removals.mode() == convergence_mode::remove_explicit);
  TEST_CHECK(removals.removals().size() == 2);
  TEST_CHECK(removals.removals()[0].name() == "alpha");
  TEST_CHECK(removals.removals()[1].name() == "zeta");
  TEST_CHECK(removals == convergence_policy::remove_explicit({
      pkgsource::package_reference("alpha"),
      pkgsource::package_reference("zeta"),
  }));

  TEST_TRANSACTION_THROWS(error_code::invalid_request,
      convergence_policy::remove_explicit({}));
  TEST_TRANSACTION_THROWS(error_code::duplicate_removal,
      convergence_policy::remove_explicit({
          pkgsource::package_reference("alpha"),
          pkgsource::package_reference("alpha"),
      }));
  return 0;
}

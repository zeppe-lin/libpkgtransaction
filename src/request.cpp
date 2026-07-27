// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#include <libpkgtransaction/request.h>

#include <libpkgtransaction/error.h>

#include "identity_support.h"

#include <algorithm>
#include <set>
#include <utility>

namespace pkgtransaction {
namespace {
transaction_request_identity make_identity(
    const pkgresolve::resolution_result& resolution,
    const convergence_policy& policy)
{
  detail::identity_writer writer;
  writer.text("libpkgtransaction/request/1");
  writer.text(resolution.identity().hex());
  writer.text(to_string(policy.mode()));
  writer.number(policy.removals().size());
  for (const auto& package : policy.removals())
    writer.text(package.name());
  return transaction_request_identity::from_sha256(writer.finish());
}

bool selected_for_nonremoval_target(
    const pkgresolve::resolution_result& resolution,
    const pkgsource::package_reference& package)
{
  for (const auto& selection : resolution.selections()) {
    if (selection.environment() != pkgresolve::resolution_environment::target ||
        selection.package() != package)
      continue;
    const auto reasons = resolution.reasons_for(selection.identity());
    const bool removal_only = !reasons.empty() &&
        std::all_of(reasons.begin(), reasons.end(), [](const auto& reason) {
          if (reason.kind() != pkgresolve::selection_reason_kind::direct_goal &&
              reason.kind() != pkgresolve::selection_reason_kind::profile_goal)
            return false;
          if (reason.scope().kind() !=
                  pkgsource::requirement_scope_kind::lifecycle ||
              !reason.scope().action())
            return false;
          return *reason.scope().action() ==
                     pkgsource::lifecycle_action::pre_remove ||
                 *reason.scope().action() ==
                     pkgsource::lifecycle_action::post_remove;
        });
    if (!removal_only)
      return true;
  }
  return false;
}
} // namespace

transaction_request::transaction_request(
    pkgresolve::resolution_result resolution,
    convergence_policy policy,
    transaction_request_identity identity)
    : resolution_(std::move(resolution)), policy_(std::move(policy)),
      identity_(std::move(identity)) {}
transaction_request transaction_request::seal(
    pkgresolve::resolution_result resolution,
    convergence_policy policy)
{
  const auto& installed = resolution.request().installed();
  if (policy.mode() == convergence_mode::remove_explicit) {
    for (const auto& package : policy.removals()) {
      if (!installed.find_package(package.name()))
        throw error(error_code::unknown_installed_package,
                    "explicit removal is not installed: " + package.name());
      if (selected_for_nonremoval_target(resolution, package))
        throw error(error_code::selected_for_removal,
                    "package is both selected and explicitly removed: " +
                    package.name());
    }
  }
  const auto identity = make_identity(resolution, policy);
  return transaction_request(std::move(resolution), std::move(policy), identity);
}
const pkgresolve::resolution_result& transaction_request::resolution() const noexcept
{ return resolution_; }
const convergence_policy& transaction_request::policy() const noexcept
{ return policy_; }
const transaction_request_identity& transaction_request::identity() const noexcept
{ return identity_; }
} // namespace pkgtransaction

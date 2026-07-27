// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#include <libpkgtransaction/model.h>

#include <libpkgtransaction/error.h>

#include <algorithm>
#include <tuple>
#include <utility>

namespace pkgtransaction {
std::string_view to_string(convergence_mode value) noexcept
{
  switch (value) {
  case convergence_mode::preserve_unselected: return "preserve-unselected";
  case convergence_mode::remove_explicit: return "remove-explicit";
  case convergence_mode::converge_exact: return "converge-exact";
  }
  return "unknown";
}
std::string_view to_string(transaction_action_kind value) noexcept
{
  switch (value) {
  case transaction_action_kind::build: return "build";
  case transaction_action_kind::check: return "check";
  case transaction_action_kind::install: return "install";
  case transaction_action_kind::upgrade: return "upgrade";
  case transaction_action_kind::retain: return "retain";
  case transaction_action_kind::remove: return "remove";
  case transaction_action_kind::lifecycle: return "lifecycle";
  }
  return "unknown";
}
std::string_view to_string(transaction_edge_kind value) noexcept
{
  switch (value) {
  case transaction_edge_kind::requirement: return "requirement";
  case transaction_edge_kind::phase: return "phase";
  }
  return "unknown";
}

convergence_policy::convergence_policy(
    convergence_mode mode,
    std::vector<pkgsource::package_reference> removals)
    : mode_(mode), removals_(std::move(removals)) {}
convergence_policy convergence_policy::preserve_unselected()
{ return convergence_policy(convergence_mode::preserve_unselected, {}); }
convergence_policy convergence_policy::remove_explicit(
    std::vector<pkgsource::package_reference> packages)
{
  if (packages.empty())
    throw error(error_code::invalid_request,
                "explicit-removal policy has no packages");
  std::sort(packages.begin(), packages.end());
  for (std::size_t index = 1; index < packages.size(); ++index)
    if (packages[index - 1] == packages[index])
      throw error(error_code::duplicate_removal,
                  "duplicate explicit removal: " + packages[index].name());
  return convergence_policy(convergence_mode::remove_explicit,
                            std::move(packages));
}
convergence_policy convergence_policy::converge_exact()
{ return convergence_policy(convergence_mode::converge_exact, {}); }
convergence_mode convergence_policy::mode() const noexcept { return mode_; }
const std::vector<pkgsource::package_reference>&
convergence_policy::removals() const noexcept { return removals_; }
bool operator==(const convergence_policy& lhs,
                const convergence_policy& rhs) noexcept
{ return std::tie(lhs.mode_, lhs.removals_) ==
         std::tie(rhs.mode_, rhs.removals_); }
bool operator!=(const convergence_policy& lhs,
                const convergence_policy& rhs) noexcept { return !(lhs == rhs); }
bool operator<(const convergence_policy& lhs,
               const convergence_policy& rhs) noexcept
{ return std::tie(lhs.mode_, lhs.removals_) <
         std::tie(rhs.mode_, rhs.removals_); }
} // namespace pkgtransaction

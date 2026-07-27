// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <string_view>
#include <vector>

#include <libpkgsource/model.h>

namespace pkgtransaction {

enum class convergence_mode {
  preserve_unselected,
  remove_explicit,
  converge_exact,
};

enum class transaction_action_kind {
  build,
  check,
  install,
  upgrade,
  retain,
  remove,
  lifecycle,
};

enum class transaction_edge_kind {
  requirement,
  phase,
};

[[nodiscard]] std::string_view to_string(convergence_mode value) noexcept;
[[nodiscard]] std::string_view to_string(transaction_action_kind value) noexcept;
[[nodiscard]] std::string_view to_string(transaction_edge_kind value) noexcept;

class convergence_policy final {
public:
  [[nodiscard]] static convergence_policy preserve_unselected();
  [[nodiscard]] static convergence_policy remove_explicit(
      std::vector<pkgsource::package_reference> packages);
  [[nodiscard]] static convergence_policy converge_exact();

  [[nodiscard]] convergence_mode mode() const noexcept;
  [[nodiscard]] const std::vector<pkgsource::package_reference>&
  removals() const noexcept;

  friend bool operator==(const convergence_policy& lhs,
                         const convergence_policy& rhs) noexcept;
  friend bool operator!=(const convergence_policy& lhs,
                         const convergence_policy& rhs) noexcept;
  friend bool operator<(const convergence_policy& lhs,
                        const convergence_policy& rhs) noexcept;
private:
  convergence_policy(convergence_mode mode,
                     std::vector<pkgsource::package_reference> removals);
  convergence_mode mode_;
  std::vector<pkgsource::package_reference> removals_;
};

} // namespace pkgtransaction

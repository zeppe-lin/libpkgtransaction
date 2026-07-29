// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <stdexcept>
#include <string>

namespace pkgtransaction {

enum class error_code {
  invalid_request,
  invalid_identity,
  duplicate_removal,
  unknown_installed_package,
  selected_for_removal,
  missing_check_program,
  unbound_lifecycle,
  inconsistent_authority,
  construction_cycle,
  lifecycle_cycle,
  identity_failed,
};

class error : public std::runtime_error {
public:
  error(error_code code, std::string message);
  [[nodiscard]] error_code code() const noexcept;
private:
  error_code code_;
};

} // namespace pkgtransaction

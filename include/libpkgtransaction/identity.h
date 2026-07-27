// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <string>

namespace pkgtransaction {

#define PKGTRANSACTION_DECLARE_IDENTITY(type_name)                             \
class type_name final {                                                        \
public:                                                                        \
  [[nodiscard]] static type_name from_sha256(std::string hex);                 \
  [[nodiscard]] const std::string& hex() const noexcept;                       \
  friend bool operator==(const type_name& lhs, const type_name& rhs) noexcept; \
  friend bool operator!=(const type_name& lhs, const type_name& rhs) noexcept; \
  friend bool operator<(const type_name& lhs, const type_name& rhs) noexcept;  \
private:                                                                       \
  explicit type_name(std::string hex);                                         \
  std::string hex_;                                                            \
}

PKGTRANSACTION_DECLARE_IDENTITY(transaction_request_identity);
PKGTRANSACTION_DECLARE_IDENTITY(transaction_node_identity);
PKGTRANSACTION_DECLARE_IDENTITY(transaction_edge_identity);
PKGTRANSACTION_DECLARE_IDENTITY(runtime_cohort_identity);
PKGTRANSACTION_DECLARE_IDENTITY(transaction_program_identity);

#undef PKGTRANSACTION_DECLARE_IDENTITY

} // namespace pkgtransaction

// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#include <libpkgtransaction/identity.h>

#include "identity_support.h"

#include <utility>

namespace pkgtransaction {
#define PKGTRANSACTION_DEFINE_IDENTITY(type_name)                              \
type_name::type_name(std::string hex) : hex_(std::move(hex)) {}                \
type_name type_name::from_sha256(std::string hex)                              \
{                                                                              \
  detail::require_sha256_hex(hex);                                             \
  return type_name(std::move(hex));                                            \
}                                                                              \
const std::string& type_name::hex() const noexcept { return hex_; }             \
bool operator==(const type_name& lhs, const type_name& rhs) noexcept           \
{ return lhs.hex_ == rhs.hex_; }                                               \
bool operator!=(const type_name& lhs, const type_name& rhs) noexcept           \
{ return !(lhs == rhs); }                                                      \
bool operator<(const type_name& lhs, const type_name& rhs) noexcept            \
{ return lhs.hex_ < rhs.hex_; }

PKGTRANSACTION_DEFINE_IDENTITY(transaction_request_identity)
PKGTRANSACTION_DEFINE_IDENTITY(transaction_node_identity)
PKGTRANSACTION_DEFINE_IDENTITY(transaction_edge_identity)
PKGTRANSACTION_DEFINE_IDENTITY(runtime_cohort_identity)
PKGTRANSACTION_DEFINE_IDENTITY(transaction_program_identity)
#undef PKGTRANSACTION_DEFINE_IDENTITY
} // namespace pkgtransaction

// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#include "../support/test.h"

#include <libpkgtransaction/libpkgtransaction.h>

#include <string>

namespace {

template<typename Identity>
void qualify_identity()
{
  const std::string zero(64, '0');
  const std::string one(63, '0');
  const std::string next = one + "1";
  const auto left = Identity::from_sha256(zero);
  const auto right = Identity::from_sha256(next);
  TEST_CHECK(left.hex() == zero);
  TEST_CHECK(left == Identity::from_sha256(zero));
  TEST_CHECK(left != right);
  TEST_CHECK(left < right);
  TEST_TRANSACTION_THROWS(pkgtransaction::error_code::invalid_identity,
      Identity::from_sha256("bad"));
  TEST_TRANSACTION_THROWS(pkgtransaction::error_code::invalid_identity,
      Identity::from_sha256(std::string(64, 'A')));
  TEST_TRANSACTION_THROWS(pkgtransaction::error_code::invalid_identity,
      Identity::from_sha256(std::string(63, '0') + "g"));
}

} // namespace

int main()
{
  qualify_identity<pkgtransaction::transaction_request_identity>();
  qualify_identity<pkgtransaction::transaction_node_identity>();
  qualify_identity<pkgtransaction::transaction_edge_identity>();
  qualify_identity<pkgtransaction::runtime_cohort_identity>();
  qualify_identity<pkgtransaction::transaction_program_identity>();
  return 0;
}

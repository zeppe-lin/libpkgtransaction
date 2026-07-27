// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#include "test.h"

#include <libpkgtransaction/libpkgtransaction.h>

int main()
{
  using namespace pkgtransaction;
  const std::string zero(64, '0');
  TEST_CHECK(transaction_request_identity::from_sha256(zero).hex() == zero);
  TEST_CHECK(transaction_node_identity::from_sha256(zero).hex() == zero);
  TEST_TRANSACTION_THROWS(error_code::invalid_identity,
      transaction_program_identity::from_sha256("bad"));
  return 0;
}

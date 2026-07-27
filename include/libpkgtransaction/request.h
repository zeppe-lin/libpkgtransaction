// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <libpkgresolve/result.h>

#include <libpkgtransaction/identity.h>
#include <libpkgtransaction/model.h>

namespace pkgtransaction {

class transaction_request final {
public:
  [[nodiscard]] static transaction_request seal(
      pkgresolve::resolution_result resolution,
      convergence_policy policy = convergence_policy::preserve_unselected());

  [[nodiscard]] const pkgresolve::resolution_result& resolution() const noexcept;
  [[nodiscard]] const convergence_policy& policy() const noexcept;
  [[nodiscard]] const transaction_request_identity& identity() const noexcept;
private:
  transaction_request(pkgresolve::resolution_result resolution,
                      convergence_policy policy,
                      transaction_request_identity identity);
  pkgresolve::resolution_result resolution_;
  convergence_policy policy_;
  transaction_request_identity identity_;
};

} // namespace pkgtransaction

// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <libpkgtransaction/program.h>

namespace pkgtransaction {

/*! \brief Compose an immutable cross-package operation graph. */
[[nodiscard]] transaction_program compose(transaction_request request);

} // namespace pkgtransaction

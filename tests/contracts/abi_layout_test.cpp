// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#include <libpkgtransaction/libpkgtransaction.h>

static_assert(sizeof(void*) == 8,
              "libpkgtransaction 3 ABI layout contract requires 64-bit pointers");
static_assert(sizeof(pkgsource::source_snapshot) == 712);
static_assert(alignof(pkgsource::source_snapshot) == 8);
static_assert(sizeof(pkgcatalog::catalog_candidate) == 896);
static_assert(sizeof(pkgstate::installed_package) == 1544);
static_assert(sizeof(pkgresolve::selection_authority) == 1552);
static_assert(sizeof(pkgresolve::selected_package) == 1792);
static_assert(sizeof(pkgresolve::resolution_result) == 704);
static_assert(sizeof(pkgtransaction::convergence_policy) == 32);
static_assert(sizeof(pkgtransaction::transaction_authority) == 1800);
static_assert(sizeof(pkgtransaction::transaction_request) == 768);
static_assert(sizeof(pkgtransaction::transaction_node) == 1904);
static_assert(sizeof(pkgtransaction::transaction_edge) == 168);
static_assert(sizeof(pkgtransaction::runtime_cohort) == 80);
static_assert(sizeof(pkgtransaction::transaction_program) == 872);

int main() { return 0; }

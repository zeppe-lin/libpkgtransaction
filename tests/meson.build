# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later
if get_option('tests').enabled()
  foreach case : [
    ['identity', 'identity_test.cpp'],
    ['request', 'request_test.cpp'],
    ['public-headers', 'public_headers.cpp'],
  ]
    executable_value = executable(
      case[0] + '-test', case[1],
      dependencies: libpkgtransaction_dep,
      install: false,
    )
    test(case[0], executable_value)
  endforeach
endif

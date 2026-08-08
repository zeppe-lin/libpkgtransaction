# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later

if get_option('tests').allowed()
  foreach case : [
    ['identity', 'unit/identity_test.cpp'],
    ['model', 'unit/model_test.cpp'],
  ]
    executable_value = executable(
      case[0] + '-test',
      case[1],
      dependencies: libpkgtransaction_dep,
      install: false,
    )
    test(case[0], executable_value, suite: 'unit')
  endforeach

  foreach case : [
    ['request', 'integration/request_test.cpp'],
    ['classification', 'integration/classification_test.cpp'],
    ['ordering', 'integration/ordering_test.cpp'],
    ['construction-ordering', 'integration/construction_ordering_test.cpp'],
    ['lifecycle', 'integration/lifecycle_test.cpp'],
    ['lifecycle-ordering', 'integration/lifecycle_ordering_test.cpp'],
    ['cycle', 'integration/cycle_test.cpp'],
    ['check-authority', 'integration/check_authority_test.cpp'],
    ['program-view', 'integration/program_view_test.cpp'],
    ['adversarial-result', 'integration/adversarial_result_test.cpp'],
  ]
    executable_value = executable(
      case[0] + '-test',
      case[1],
      dependencies: libpkgtransaction_dep,
      install: false,
    )
    test(case[0], executable_value, suite: 'integration')
  endforeach

  transaction_headers = [
    'composer.h',
    'error.h',
    'identity.h',
    'libpkgtransaction.h',
    'model.h',
    'program.h',
    'request.h',
  ]
  foreach header : transaction_headers
    header_test = executable(
      'public-header-' + header.underscorify(),
      'unit/public_header_test.cpp',
      cpp_args: [
        '-DPKGTRANSACTION_TEST_HEADER="libpkgtransaction/' + header + '"',
      ],
      dependencies: libpkgtransaction_dep,
      install: false,
    )
    test('public-header-' + header, header_test, suite: 'header')
  endforeach

  pkgconfig_metadata = find_program('contracts/check_pkgconfig_metadata.sh')
  test(
    'pkgconfig-metadata',
    pkgconfig_metadata,
    args: [meson.project_build_root()],
    suite: 'contract',
  )

  release_metadata = find_program('contracts/check_release_metadata.sh')
  test(
    'release-metadata',
    release_metadata,
    args: [meson.project_source_root()],
    suite: 'contract',
  )

  authority_contract = find_program('contracts/check_authority_contract.sh')
  test(
    'authority-contract',
    authority_contract,
    args: [meson.project_source_root()],
    suite: 'contract',
  )

  test_layout = find_program('contracts/check_test_layout.sh')
  test(
    'test-layout',
    test_layout,
    args: [meson.project_source_root()],
    suite: 'contract',
  )
endif

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
    ['runtime-cohort-ordering', 'integration/runtime_cohort_ordering_test.cpp'],
    ['runtime-cohort-boundary-ordering', 'integration/runtime_cohort_boundary_ordering_test.cpp'],
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

  sh = find_program('sh')

  pkgconfig_metadata = find_program('contracts/check_pkgconfig_metadata.sh')
  test(
    'pkgconfig-metadata',
    pkgconfig_metadata,
    args: [meson.project_build_root()],
    suite: 'contract',
  )

  abi_layout = executable(
    'abi-layout-test',
    'contracts/abi_layout_test.cpp',
    dependencies: libpkgtransaction_dep,
    install: false,
  )
  test('abi-layout', abi_layout, suite: 'contract')

  if library_kind == 'shared'
    nm = find_program('nm')
    test(
      'abi-surface',
      sh,
      args: [
        meson.current_source_dir() / 'contracts/check_abi_surface.sh',
        nm.full_path(),
        libpkgtransaction.full_path(),
        meson.project_source_root() / 'abi/libpkgtransaction.exports',
      ],
      suite: 'contract',
    )
    dependency_abi = find_program('contracts/check_dependency_abi.sh')
    test(
      'dependency-abi',
      dependency_abi,
      args: [libpkgtransaction.full_path()],
      suite: 'contract',
    )
  endif

  contract_tests = {
    'release-metadata': 'contracts/check_release_metadata.sh',
    'authority-contract': 'contracts/check_authority_contract.sh',
    'test-layout': 'contracts/check_test_layout.sh',
    'abi-contract': 'contracts/check_abi_contract.sh',
    'ci-contract': 'contracts/check_ci_contract.sh',
  }
  foreach name, program : contract_tests
    test(
      name,
      sh,
      args: [meson.current_source_dir() / program, meson.project_source_root()],
      suite: 'contract',
    )
  endforeach
endif

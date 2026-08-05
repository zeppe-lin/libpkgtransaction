# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later
if get_option('tests').allowed()
  foreach case : [
    ['identity', 'identity_test.cpp'],
    ['request', 'request_test.cpp'],
    ['composer', 'composer_test.cpp'],
    ['check-program', 'check_program_test.cpp'],
    ['public-headers', 'public_headers.cpp'],
  ]
    executable_value = executable(
      case[0] + '-test',
      case[1],
      dependencies: libpkgtransaction_dep,
      install: false,
    )
    test(case[0], executable_value)
  endforeach
  metadata = find_program('metadata_test.sh')
  test('metadata', metadata, args: [meson.project_build_root()])
  release_metadata = find_program('check_release_metadata.sh')
  test('release-metadata', release_metadata,
       args: [meson.project_source_root()])
  authority_contract = find_program('check_authority_contract.sh')
  test('authority-contract', authority_contract,
       args: [meson.project_source_root()])
endif

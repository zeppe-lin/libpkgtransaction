# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later
project(
  'libpkgtransaction',
  'cpp',
  version: '1.1.0',
  license: 'GPL-3.0-or-later',
  meson_version: '>=1.2.0',
  default_options: [
    'cpp_std=c++17',
    'warning_level=3',
    'default_library=shared',
    'b_ndebug=if-release',
  ],
)

library_kind = get_option('default_library')
if library_kind == 'both'
  error('libpkgtransaction requires separate shared and static builds')
endif
want_static = get_option('link_mode') == 'static'
if (library_kind == 'static') != want_static
  error('default_library and link_mode must both select shared or static')
endif

libcrypto_dep = dependency('libcrypto', required: true, static: want_static)
libpkgstate_dep = dependency(
  'libpkgstate',
  version: '>=2.1.0',
  required: true,
  static: want_static,
  fallback: ['libpkgstate', 'libpkgstate_dep'],
  default_options: [
    'default_library=' + library_kind,
    'link_mode=' + get_option('link_mode'),
    'source_adapter=disabled',
    'build_adapter=disabled',
    'planner_adapter=disabled',
    'application_adapter=disabled',
  ],
)
libpkgresolve_dep = dependency(
  'libpkgresolve',
  version: '>=1.0.0',
  required: true,
  static: want_static,
  fallback: ['libpkgresolve', 'libpkgresolve_dep'],
  default_options: [
    'default_library=' + library_kind,
    'link_mode=' + get_option('link_mode'),
  ],
)
pkgconf = import('pkgconfig')

subdir('src')
subdir('man')
subdir('tests')

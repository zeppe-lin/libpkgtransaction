# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later
public_inc = include_directories('../include')
private_inc = include_directories('.')

libpkgtransaction = library(
  'pkgtransaction',
  [
    'error.cpp',
    'identity.cpp',
    'identity_support.cpp',
    'model.cpp',
    'request.cpp',
    'program.cpp',
    'composer.cpp',
  ],
  include_directories: [public_inc, private_inc],
  dependencies: [libcrypto_dep, libpkgresolve_dep, libpkgstate_dep],
  install: true,
  version: meson.project_version(),
  soversion: '0',
)

libpkgtransaction_dep = declare_dependency(
  link_with: libpkgtransaction,
  include_directories: public_inc,
  dependencies: [libcrypto_dep, libpkgresolve_dep, libpkgstate_dep],
)
meson.override_dependency('libpkgtransaction', libpkgtransaction_dep)

install_headers(
  '../include/libpkgtransaction/libpkgtransaction.h',
  '../include/libpkgtransaction/error.h',
  '../include/libpkgtransaction/identity.h',
  '../include/libpkgtransaction/model.h',
  '../include/libpkgtransaction/request.h',
  '../include/libpkgtransaction/program.h',
  '../include/libpkgtransaction/composer.h',
  subdir: 'libpkgtransaction',
)

pkgconf.generate(
  libpkgtransaction,
  name: 'libpkgtransaction',
  filebase: 'libpkgtransaction',
  description: 'Native deterministic package-transaction authority',
  subdirs: '.',
  requires: [
    'libpkgresolve >= 1.0.0',
    'libpkgstate >= 2.1.0',
  ],
  requires_private: ['libcrypto'],
)

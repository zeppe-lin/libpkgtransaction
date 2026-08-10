# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later
public_inc = include_directories('../include')
private_inc = include_directories('.')

elf_export_generator = find_program(
  meson.project_source_root() / 'tools/generate-elf-export-script.sh',
)
elf_export_script_name = 'libpkgtransaction.exports.map'
elf_export_script_path = meson.current_build_dir() / elf_export_script_name
elf_export_script = configure_file(
  input: '../abi/libpkgtransaction.exports',
  output: elf_export_script_name,
  command: [elf_export_generator, '@INPUT@'],
  capture: true,
)

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
  dependencies: [
    libcrypto_dep,
    libpkgsource_dep,
    libpkgresolve_dep,
    libpkgstate_dep,
  ],
  link_args: ['-Wl,--version-script=' + elf_export_script_path],
  link_depends: [elf_export_script],
  install: true,
  version: meson.project_version(),
  soversion: '3',
)

libpkgtransaction_consumer_deps = [
  libpkgsource_dep,
  libpkgresolve_dep,
  libpkgstate_dep,
]
if want_static
  libpkgtransaction_consumer_deps += [libcrypto_dep]
endif

libpkgtransaction_dep = declare_dependency(
  link_with: libpkgtransaction,
  include_directories: public_inc,
  dependencies: libpkgtransaction_consumer_deps,
  version: meson.project_version(),
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
    libpkgsource_dep,
    libpkgresolve_dep,
    libpkgstate_dep,
  ],
  requires_private: [libcrypto_dep],
)

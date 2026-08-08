// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <libpkgcatalog/libpkgcatalog.h>
#include <libpkgresolve/libpkgresolve.h>
#include <libpkgstate/libpkgstate.h>

namespace fixture {

inline pkgsource::declaration_provenance at(
    std::string path, std::uint32_t line = 1)
{
  return pkgsource::declaration_provenance(
      "recipe.yml", std::move(path), line, 3);
}

inline pkgsource::requirement_declaration requirement(
    pkgsource::requirement_scope scope,
    std::string package,
    std::string path)
{
  return pkgsource::requirement_declaration(
      std::move(scope),
      pkgsource::requirement_subject(
          pkgsource::package_reference(std::move(package))),
      at(std::move(path)));
}

inline pkgsource::requirement_declaration profile_requirement(
    pkgsource::requirement_scope scope,
    std::string profile,
    std::string path)
{
  return pkgsource::requirement_declaration(
      std::move(scope),
      pkgsource::requirement_subject(
          pkgsource::profile_reference(std::move(profile))),
      at(std::move(path)));
}

inline pkgsource::profile_catalog profiles()
{
  pkgsource::profile_declaration toolchain(
      pkgsource::profile_reference("@toolchain"), at("profiles.toolchain"),
      {
        pkgsource::profile_member_declaration(
            pkgsource::requirement_subject(
                pkgsource::package_reference("compiler")),
            at("profiles.toolchain.members[0]")),
        pkgsource::profile_member_declaration(
            pkgsource::requirement_subject(
                pkgsource::package_reference("make")),
            at("profiles.toolchain.members[1]")),
      });
  pkgsource::profile_declaration desktop(
      pkgsource::profile_reference("@desktop"), at("profiles.desktop"),
      {
        pkgsource::profile_member_declaration(
            pkgsource::requirement_subject(
                pkgsource::package_reference("app")),
            at("profiles.desktop.members[0]")),
        pkgsource::profile_member_declaration(
            pkgsource::requirement_subject(
                pkgsource::package_reference("data")),
            at("profiles.desktop.members[1]")),
      });
  return pkgsource::profile_catalog::seal(
      {std::move(toolchain), std::move(desktop)});
}

inline pkgsource::source_snapshot source(
    const pkgsource::profile_catalog& profile_catalog,
    std::string name,
    std::vector<pkgsource::requirement_declaration> requirements = {},
    std::vector<std::string> build_architectures = {"x86_64"},
    std::vector<std::string> target_architectures = {"x86_64"},
    std::string version = "1.0.0",
    std::uint32_t release = 1,
    std::vector<pkgsource::lifecycle_action> lifecycle_actions = {},
    std::optional<std::string> check_program = std::nullopt)
{
  std::vector<pkgsource::architecture_reference> build;
  for (auto& value : build_architectures)
    build.emplace_back(std::move(value));
  std::vector<pkgsource::architecture_reference> target;
  for (auto& value : target_architectures)
    target.emplace_back(std::move(value));

  std::vector<pkgsource::lifecycle_program> lifecycle_programs;
  std::vector<pkgsource::lifecycle_action> actions =
      std::move(lifecycle_actions);
  for (const auto& value : requirements) {
    if (value.scope().kind() == pkgsource::requirement_scope_kind::lifecycle)
      actions.push_back(*value.scope().action());
  }
  std::sort(actions.begin(), actions.end());
  actions.erase(std::unique(actions.begin(), actions.end()), actions.end());
  for (const auto action : actions)
    lifecycle_programs.emplace_back(
        action,
        pkgsource::program(pkgsource::program_language::posix_shell,
                           "true\n"));

  std::optional<pkgsource::program> check;
  if (check_program)
    check.emplace(pkgsource::program_language::posix_shell,
                  std::move(*check_program));

  pkgsource::recipe_declaration declaration(
      pkgsource::package_release(
          pkgsource::package_reference(name), std::move(version), release),
      pkgsource::package_metadata(
          name + " package", std::nullopt, std::nullopt,
          {"GPL-3.0-or-later"}),
      {},
      pkgsource::program(pkgsource::program_language::posix_shell,
                         "true\n"),
      std::move(requirements), std::move(lifecycle_programs),
      pkgsource::architecture_requirements(
          std::move(build), std::move(target)),
      at("recipe", 1), check);
  return pkgsource::seal_source(
      pkgsource::source_origin(name + "/recipe.yml"),
      std::move(declaration), profile_catalog);
}

inline pkgcatalog::catalog_snapshot catalog(
    pkgsource::profile_catalog profile_catalog,
    std::vector<pkgsource::source_snapshot> sources)
{
  pkgcatalog::collection_declaration declaration(
      pkgcatalog::collection_reference("core"),
      pkgcatalog::collection_provenance(
          "/collections/core", std::nullopt,
          pkgsource::declaration_provenance(
              "catalog", "collections[0]", 1, 1)),
      std::move(sources));
  std::vector<pkgcatalog::catalog_collection> collections;
  collections.emplace_back(
      0, pkgcatalog::seal_collection(std::move(declaration)));
  return pkgcatalog::catalog_snapshot::seal(
      std::move(profile_catalog), std::move(collections));
}

inline pkgstate::sha256_digest_bytes bytes_from_hex(const std::string& hex)
{
  pkgstate::sha256_digest_bytes result{};
  for (std::size_t index = 0; index < result.size(); ++index) {
    const auto digit = [](char value) -> std::uint8_t {
      if (value >= '0' && value <= '9')
        return static_cast<std::uint8_t>(value - '0');
      return static_cast<std::uint8_t>(value - 'a' + 10);
    };
    result[index] = static_cast<std::uint8_t>(
        (digit(hex[index * 2]) << 4) | digit(hex[index * 2 + 1]));
  }
  return result;
}

template<typename Identity>
Identity state_identity(std::uint8_t seed)
{
  pkgstate::sha256_digest_bytes bytes{};
  for (std::size_t index = 0; index < bytes.size(); ++index)
    bytes[index] = static_cast<std::uint8_t>(seed + index);
  return Identity::from_sha256(bytes);
}

template<typename Identity>
Identity imported_identity(const std::string& hex)
{
  return Identity::from_sha256(bytes_from_hex(hex));
}

inline pkgstate::state_target_binding target(std::uint8_t seed = 1)
{
  return pkgstate::state_target_binding::make(
      state_identity<pkgstate::managed_target_identity>(seed),
      state_identity<pkgstate::state_store_identity>(seed + 1),
      state_identity<pkgstate::root_view_identity>(seed + 2),
      state_identity<pkgstate::state_backend_identity>(seed + 3),
      state_identity<pkgstate::publication_domain_identity>(seed + 4));
}

inline pkgstate::declaration_provenance state_provenance(
    const pkgsource::declaration_provenance& value)
{
  return pkgstate::declaration_provenance(
      value.document(), value.path(), value.line(), value.column());
}

inline pkgstate::profile_expansion_step state_step(
    const pkgsource::profile_expansion_step& value)
{
  return pkgstate::profile_expansion_step(
      pkgstate::profile_reference(value.profile().name()),
      value.member().kind() == pkgsource::requirement_subject_kind::package
          ? pkgstate::requirement_member_kind::package
          : pkgstate::requirement_member_kind::profile,
      value.member().text(), state_provenance(value.provenance()));
}

inline pkgstate::package_requirement state_requirement(
    const pkgsource::resolved_requirement& value)
{
  std::vector<pkgstate::requirement_origin> origins;
  for (const auto& origin : value.origins()) {
    std::vector<pkgstate::profile_expansion_step> expansion;
    for (const auto& step : origin.expansion())
      expansion.push_back(state_step(step));
    origins.emplace_back(state_provenance(origin.declaration()),
                         std::move(expansion));
  }
  return pkgstate::package_requirement(
      pkgstate::package_reference(value.package().name()),
      std::move(origins));
}

inline pkgstate::lifecycle_action state_action(pkgsource::lifecycle_action action)
{
  switch (action) {
  case pkgsource::lifecycle_action::pre_install:
    return pkgstate::lifecycle_action::pre_install;
  case pkgsource::lifecycle_action::post_install:
    return pkgstate::lifecycle_action::post_install;
  case pkgsource::lifecycle_action::pre_remove:
    return pkgstate::lifecycle_action::pre_remove;
  case pkgsource::lifecycle_action::post_remove:
    return pkgstate::lifecycle_action::post_remove;
  }
  return pkgstate::lifecycle_action::pre_install;
}

inline pkgstate::installed_package installed_package(
    const pkgsource::source_snapshot& source,
    pkgstate::state_target_binding binding,
    std::uint8_t seed = 30,
    pkgsource::architecture_reference selected_build =
        pkgsource::architecture_reference("x86_64"),
    pkgsource::architecture_reference selected_target =
        pkgsource::architecture_reference("x86_64"))
{
  const auto& recipe = source.recipe();
  std::vector<pkgstate::package_requirement> runtime;
  for (const auto& value : recipe.run_requirements())
    runtime.push_back(state_requirement(value));
  std::vector<pkgstate::lifecycle_requirement> lifecycle;
  for (const auto action : {
      pkgsource::lifecycle_action::pre_install,
      pkgsource::lifecycle_action::post_install,
      pkgsource::lifecycle_action::pre_remove,
      pkgsource::lifecycle_action::post_remove})
    for (const auto& value : recipe.lifecycle_requirements(action))
      lifecycle.emplace_back(state_action(action), state_requirement(value));

  std::vector<pkgstate::architecture_reference> declared_build;
  for (const auto& value : recipe.architectures().build())
    declared_build.emplace_back(value.name());
  std::vector<pkgstate::architecture_reference> declared_target;
  for (const auto& value : recipe.architectures().target())
    declared_target.emplace_back(value.name());

  pkgstate::package_release release(
      imported_identity<pkgstate::package_release_identity>(
          recipe.release().identity().hex()),
      pkgstate::package_reference(recipe.release().package().name()),
      recipe.release().version(), recipe.release().release());
  pkgstate::package_source_record source_record =
      pkgstate::package_source_record::make(
          std::move(release),
          pkgstate::package_metadata(
              recipe.metadata().summary(), recipe.metadata().description(),
              recipe.metadata().homepage(), recipe.metadata().licenses()),
          std::move(runtime), {}, std::move(lifecycle),
          pkgstate::architecture_binding::make(
              std::move(declared_build), std::move(declared_target),
              pkgstate::architecture_reference(selected_build.name()),
              pkgstate::architecture_reference(selected_target.name())),
          {},
          imported_identity<pkgstate::source_snapshot_identity>(
              source.identity().hex()));

  pkgstate::installed_control control = pkgstate::installed_control::make(
      source_record, pkgstate::installation_reason::explicit_request(),
      pkgstate::build_provenance(
          source_record.identity(),
          state_identity<pkgstate::build_request_identity>(seed),
          state_identity<pkgstate::build_input_set_identity>(seed + 2),
          state_identity<pkgstate::environment_policy_identity>(seed + 3),
          state_identity<pkgstate::build_policy_identity>(seed + 4),
          state_identity<pkgstate::build_result_identity>(seed + 5),
          state_identity<pkgstate::payload_manifest_identity>(seed + 6),
          state_identity<pkgstate::build_artifact_identity>(seed + 7),
          state_identity<pkgstate::artifact_content_identity>(seed + 8),
          state_identity<pkgstate::artifact_binding_identity>(seed + 9),
          state_identity<pkgstate::execution_evidence_identity>(seed + 10),
          state_identity<pkgstate::build_image_identity>(seed + 11),
          state_identity<pkgstate::artifact_image_identity>(seed + 12),
          state_identity<pkgstate::artifact_inspection_identity>(seed + 13)));
  return pkgstate::installed_package::make(
      pkgstate::installation_receipt::make(
          std::move(control), std::move(binding), {},
          state_identity<pkgstate::operation_plan_identity>(seed + 13),
          state_identity<pkgstate::application_evidence_identity>(seed + 14)));
}

inline pkgstate::snapshot empty_state(pkgstate::state_target_binding binding = target())
{
  return pkgstate::snapshot::make(std::move(binding));
}

inline pkgstate::snapshot state(
    std::vector<pkgstate::installed_package> packages,
    pkgstate::state_target_binding binding = target())
{
  return pkgstate::snapshot::make(std::move(binding), std::move(packages));
}

inline pkgresolve::resolution_goal package_goal(
    pkgsource::requirement_scope scope, std::string package,
    std::string origin = "test")
{
  return pkgresolve::resolution_goal(
      std::move(scope),
      pkgsource::requirement_subject(
          pkgsource::package_reference(std::move(package))),
      std::move(origin));
}

inline pkgresolve::resolution_goal profile_goal(
    pkgsource::requirement_scope scope, std::string profile,
    std::string origin = "test")
{
  return pkgresolve::resolution_goal(
      std::move(scope),
      pkgsource::requirement_subject(
          pkgsource::profile_reference(std::move(profile))),
      std::move(origin));
}

} // namespace fixture

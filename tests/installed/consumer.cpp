// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

#include <libpkgcatalog/libpkgcatalog.h>
#include <libpkgresolve/libpkgresolve.h>
#include <libpkgsource/libpkgsource.h>
#include <libpkgstate/libpkgstate.h>
#include <libpkgtransaction/libpkgtransaction.h>

namespace {

template<typename Identity>
Identity state_identity(std::uint8_t seed)
{
  pkgstate::sha256_digest_bytes bytes{};
  for (std::size_t index = 0; index < bytes.size(); ++index)
    bytes[index] = static_cast<std::uint8_t>(seed + index);
  return Identity::from_sha256(bytes);
}

pkgstate::state_target_binding target()
{
  return pkgstate::state_target_binding::make(
      state_identity<pkgstate::managed_target_identity>(1),
      state_identity<pkgstate::state_store_identity>(2),
      state_identity<pkgstate::root_view_identity>(3),
      state_identity<pkgstate::state_backend_identity>(4),
      state_identity<pkgstate::publication_domain_identity>(5));
}

pkgsource::declaration_provenance provenance(const char* path)
{
  return pkgsource::declaration_provenance("recipe.yml", path, 1, 1);
}

} // namespace

int main()
{
  auto profiles = pkgsource::profile_catalog::seal({});
  pkgsource::recipe_declaration recipe(
      pkgsource::package_release(pkgsource::package_reference("installed-consumer"),
                                 "1.0.0", 1),
      pkgsource::package_metadata("installed consumer", std::nullopt,
                                  std::nullopt, {"GPL-3.0-or-later"}),
      {},
      pkgsource::program(pkgsource::program_language::posix_shell, "true\n"),
      {}, {},
      pkgsource::architecture_requirements(
          {pkgsource::architecture_reference("x86_64")},
          {pkgsource::architecture_reference("x86_64")}),
      provenance("recipe"), std::nullopt);
  auto source = pkgsource::seal_source(
      pkgsource::source_origin("installed-consumer/recipe.yml"),
      std::move(recipe), profiles);

  pkgcatalog::collection_declaration collection(
      pkgcatalog::collection_reference("core"),
      pkgcatalog::collection_provenance(
          "/collections/core", std::nullopt,
          pkgsource::declaration_provenance("catalog", "collections[0]", 1, 1)),
      {source});
  std::vector<pkgcatalog::catalog_collection> collections;
  collections.emplace_back(0, pkgcatalog::seal_collection(std::move(collection)));
  auto catalog = pkgcatalog::catalog_snapshot::seal(profiles, std::move(collections));
  auto installed = pkgstate::snapshot::make(target());

  auto resolution_request = pkgresolve::resolution_request::seal(
      std::move(catalog), std::move(installed),
      pkgresolve::architecture_context(
          pkgsource::architecture_reference("x86_64"),
          pkgsource::architecture_reference("x86_64")),
      {pkgresolve::resolution_goal(
          pkgsource::requirement_scope::run(),
          pkgsource::requirement_subject(
              pkgsource::package_reference("installed-consumer")),
          "installed-consumer")});
  const auto resolution = pkgresolve::resolve(std::move(resolution_request));
  const auto request = pkgtransaction::transaction_request::seal(resolution);
  const auto program = pkgtransaction::compose(request);
  if (program.request().identity() != request.identity())
    return 1;
  if (program.nodes_for(pkgsource::package_reference("installed-consumer")).empty())
    return 2;

  try {
    (void)pkgtransaction::convergence_policy::remove_explicit({});
    return 3;
  } catch (const pkgtransaction::error& value) {
    if (value.code() != pkgtransaction::error_code::invalid_request)
      return 4;
  }
  return 0;
}

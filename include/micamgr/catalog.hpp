#ifndef MICAMGR_CATALOG_HPP
#define MICAMGR_CATALOG_HPP

#include <micamgr/types.hpp>
#include <vector>
#include <string>
#include <string_view>
#include <optional>
#include <mutex>

namespace micamgr {

class Catalog {
public:
    Catalog();

    /// Seeds default sovereign, WinGet, and AOSP package catalogs.
    void seedDefaults();

    /// Adds or updates a package manifest.
    void addOrUpdatePackage(const PackageManifest& pkg);

    /// Searches packages matching query across name, vendor, description, or id.
    [[nodiscard]] std::vector<PackageManifest> search(std::string_view query) const;

    /// Filters packages by category.
    [[nodiscard]] std::vector<PackageManifest> filterByCategory(PackageCategory cat) const;

    /// Filters packages by source.
    [[nodiscard]] std::vector<PackageManifest> filterBySource(PackageSource src) const;

    /// Finds a package by unique package ID.
    [[nodiscard]] std::optional<PackageManifest> findById(std::string_view id) const;

    /// Returns all registered packages.
    [[nodiscard]] std::vector<PackageManifest> allPackages() const;

    /// Total count of packages.
    [[nodiscard]] size_t size() const;

    /// Sets the pinned status of a package by ID.
    bool pinPackage(std::string_view id, bool pin = true);

    /// Checks if a package is pinned to prevent automatic upgrades.
    [[nodiscard]] bool isPinned(std::string_view id) const;

    /// Returns all pinned packages.
    [[nodiscard]] std::vector<PackageManifest> pinnedPackages() const;

    /// Loads external catalog JSON data.
    bool loadFromJson(std::string_view json);

    /// Exports entire catalog to JSON string.
    [[nodiscard]] std::string exportToJson() const;

private:
    mutable std::mutex mutex_;
    std::vector<PackageManifest> packages_;
};

} // namespace micamgr

#endif // MICAMGR_CATALOG_HPP

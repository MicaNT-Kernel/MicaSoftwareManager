#ifndef MICAMGR_PACKAGE_HPP
#define MICAMGR_PACKAGE_HPP

#include <micamgr/types.hpp>
#include <string>
#include <vector>
#include <optional>

namespace micamgr {

class PackageHelper {
public:
    /// Compares two version strings (e.g., "1.2.3" vs "1.2.4").
    /// Returns: -1 if v1 < v2, 0 if v1 == v2, 1 if v1 > v2.
    static int compareVersions(std::string_view v1, std::string_view v2);

    /// Serializes a package manifest into a clean JSON object string.
    static std::string toJson(const PackageManifest& pkg);

    /// Parses a package manifest from a JSON string block.
    static std::optional<PackageManifest> fromJson(std::string_view json);

    /// Serializes a collection of package manifests into a JSON array string.
    static std::string toManifestListJson(const std::vector<PackageManifest>& packages);

    /// Parses a collection of package manifests from a JSON array string.
    static std::vector<PackageManifest> fromManifestListJson(std::string_view json);

    /// Serializes an offline bundle to a JSON string.
    static std::string toBundleJson(const OfflineBundle& bundle);

    /// Parses an offline bundle from a JSON string.
    static std::optional<OfflineBundle> fromBundleJson(std::string_view json);
};

} // namespace micamgr

#endif // MICAMGR_PACKAGE_HPP

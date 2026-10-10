#ifndef MICAMGR_UPDATE_ENGINE_HPP
#define MICAMGR_UPDATE_ENGINE_HPP

#include <micamgr/types.hpp>
#include <micamgr/catalog.hpp>
#include <micamgr/registry_scanner.hpp>
#include <vector>
#include <string>
#include <functional>

namespace micamgr {

struct UpdateReport {
    std::string packageId;
    std::string name;
    std::string currentVersion;
    std::string newVersion;
    PackageSource source;
    bool success{false};
    std::string message;
};

class UpdateEngine {
public:
    using ProgressCallback = std::function<void(const std::string& packageId, float progress, const std::string& status)>;

    /// Cross-references installed software with catalog packages to find updates.
    static std::vector<PackageManifest> checkForUpdates(Catalog& catalog, const std::vector<InstalledAppInfo>& installedApps);

    /// Performs an update on a specific package.
    static UpdateReport updatePackage(const PackageManifest& pkg, ProgressCallback onProgress = nullptr);

    /// Performs a batch update on all available packages with pending updates.
    static std::vector<UpdateReport> updateAll(const std::vector<PackageManifest>& pendingUpdates, ProgressCallback onProgress = nullptr);
};

} // namespace micamgr

#endif // MICAMGR_UPDATE_ENGINE_HPP

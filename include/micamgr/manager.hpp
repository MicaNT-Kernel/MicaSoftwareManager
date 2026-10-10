#ifndef MICAMGR_MANAGER_HPP
#define MICAMGR_MANAGER_HPP

#include <micamgr/types.hpp>
#include <micamgr/catalog.hpp>
#include <micamgr/registry_scanner.hpp>
#include <micamgr/update_engine.hpp>
#include <micamgr/sideload.hpp>
#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace micamgr {

class MicaManager {
public:
    static MicaManager& instance();

    /// Initializes catalogs and performs initial system inventory.
    void initialize();

    /// Refreshes the local installed applications inventory.
    void refreshInstalledApps();

    /// Returns the main software catalog.
    [[nodiscard]] Catalog& catalog() noexcept { return catalog_; }
    [[nodiscard]] const Catalog& catalog() const noexcept { return catalog_; }

    /// Returns the list of currently installed applications discovered.
    [[nodiscard]] const std::vector<InstalledAppInfo>& installedApps() const noexcept { return installedApps_; }

    /// Returns packages with available updates.
    [[nodiscard]] std::vector<PackageManifest> checkUpdates();

    /// Installs a package by ID.
    bool installPackage(std::string_view packageId, std::string& outMessage);

    /// Uninstalls an application by ID or display name.
    bool uninstallPackage(std::string_view packageId, std::string& outMessage);

    /// Upgrades all outdated packages.
    std::vector<UpdateReport> upgradeAll(UpdateEngine::ProgressCallback onProgress = nullptr);

    /// Inspects and prepares a local package for sideloading.
    SideloadInspectResult inspectLocalFile(const std::filesystem::path& path);

    /// Installs a local inspected package file.
    bool sideloadFile(const std::filesystem::path& path, std::string& outMessage);

    /// Pins or unpins a package version to prevent accidental upgrades.
    bool pinPackage(std::string_view packageId, bool pin = true);

    /// Checks if a package is pinned.
    [[nodiscard]] bool isPackagePinned(std::string_view packageId) const;

    /// Returns list of all pinned packages.
    [[nodiscard]] std::vector<PackageManifest> pinnedPackages() const;

    /// Retrieves full manifest inspection details for a package.
    [[nodiscard]] std::optional<PackageManifest> getPackageDetails(std::string_view packageId) const;

    /// Returns curated sovereign software stacks.
    [[nodiscard]] std::vector<CuratedStack> curatedStacks() const;

    /// Installs all packages in a curated stack.
    std::vector<std::string> installCuratedStack(std::string_view stackId, std::string& outMessage);

    /// Exports a self-contained offline bundle pack.
    bool exportBundle(const std::filesystem::path& targetDir, const std::vector<std::string>& packageIds, std::string& outMessage);

    /// Imports and installs packages from an offline bundle pack.
    bool importBundle(const std::filesystem::path& bundleDir, std::string& outMessage);

    /// Configures dry-run / simulation mode (useful for testing and preflight checks)
    void setDryRun(bool dryRun) noexcept { dryRun_ = dryRun; }
    [[nodiscard]] bool isDryRun() const noexcept { return dryRun_; }

private:
    MicaManager();
    ~MicaManager() = default;

    Catalog catalog_;
    std::vector<InstalledAppInfo> installedApps_;
    bool initialized_{false};
    bool dryRun_{false};
};

} // namespace micamgr

#endif // MICAMGR_MANAGER_HPP

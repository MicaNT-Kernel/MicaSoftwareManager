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

private:
    MicaManager();
    ~MicaManager() = default;

    Catalog catalog_;
    std::vector<InstalledAppInfo> installedApps_;
    bool initialized_{false};
};

} // namespace micamgr

#endif // MICAMGR_MANAGER_HPP

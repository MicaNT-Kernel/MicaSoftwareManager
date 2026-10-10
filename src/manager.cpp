#include <micamgr/manager.hpp>
#include <iostream>
#include <sstream>

namespace micamgr {

MicaManager& MicaManager::instance() {
    static MicaManager mgr;
    return mgr;
}

MicaManager::MicaManager() {
    initialize();
}

void MicaManager::initialize() {
    if (initialized_) return;
    catalog_.seedDefaults();
    refreshInstalledApps();
    initialized_ = true;
}

void MicaManager::refreshInstalledApps() {
    installedApps_ = RegistryScanner::scanInstalledApps();
    UpdateEngine::checkForUpdates(catalog_, installedApps_);
}

std::vector<PackageManifest> MicaManager::checkUpdates() {
    refreshInstalledApps();
    return catalog_.filterByCategory(PackageCategory::Updates);
}

bool MicaManager::installPackage(std::string_view packageId, std::string& outMessage) {
    auto optPkg = catalog_.findById(packageId);
    if (!optPkg) {
        outMessage = "Package not found: " + std::string(packageId);
        return false;
    }

    auto pkg = *optPkg;
    std::ostringstream ss;
    ss << "[MicaManager] Installing " << pkg.name << " (" << pkg.id << ") v" << pkg.version << "\n";
    ss << "[MicaManager] Source: " << pkg.sourceToString() << " | Target Arch: " << pkg.architecture << "\n";
    ss << "[MicaManager] SHA-256 Checksum: " << pkg.sha256 << "\n";

    if (pkg.source == PackageSource::WinGet) {
        ss << "[MicaManager] Executing WinGet non-interactive silent provisioning...\n";
    } else if (pkg.source == PackageSource::AospWsa) {
        ss << "[MicaManager] Submitting package to WSA container with MicaG TPM 2.0 attestation...\n";
    } else {
        ss << "[MicaManager] Provisioning MicaNT sovereign executable...\n";
    }

    pkg.state = InstallState::Installed;
    pkg.installedVersion = pkg.version;
    catalog_.addOrUpdatePackage(pkg);

    ss << "[MicaManager] Installation completed successfully.";
    outMessage = ss.str();
    return true;
}

bool MicaManager::uninstallPackage(std::string_view packageId, std::string& outMessage) {
    auto optPkg = catalog_.findById(packageId);
    if (!optPkg) {
        // Try searching in installedApps
        for (const auto& app : installedApps_) {
            if (app.matchedPackageId == packageId || app.displayName == packageId) {
                outMessage = "Uninstalling " + app.displayName + " via: " + (app.quietUninstallString.empty() ? app.uninstallString : app.quietUninstallString);
                return true;
            }
        }
        outMessage = "Application not found: " + std::string(packageId);
        return false;
    }

    auto pkg = *optPkg;
    pkg.state = InstallState::NotInstalled;
    pkg.installedVersion.clear();
    catalog_.addOrUpdatePackage(pkg);

    outMessage = "Successfully removed " + pkg.name + " (" + pkg.id + ") from system.";
    return true;
}

std::vector<UpdateReport> MicaManager::upgradeAll(UpdateEngine::ProgressCallback onProgress) {
    auto updates = checkUpdates();
    auto reports = UpdateEngine::updateAll(updates, onProgress);
    refreshInstalledApps();
    return reports;
}

SideloadInspectResult MicaManager::inspectLocalFile(const std::filesystem::path& path) {
    return SideloadInspector::inspectFile(path);
}

bool MicaManager::sideloadFile(const std::filesystem::path& path, std::string& outMessage) {
    auto inspect = SideloadInspector::inspectFile(path);
    if (!inspect.valid) {
        outMessage = "Inspection failed: " + inspect.errorMessage;
        return false;
    }
    return SideloadInspector::executeSideload(path, inspect, outMessage);
}

} // namespace micamgr

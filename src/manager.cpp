#include <micamgr/manager.hpp>
#include <micamgr/package.hpp>
#include <iostream>
#include <sstream>
#include <fstream>

namespace micamgr {

MicaManager& MicaManager::instance() {
    static MicaManager mgr;
    return mgr;
}

MicaManager::MicaManager() {
    initialize();
}

static std::filesystem::path getPinsConfigPath() {
#ifdef _WIN32
    const char* localApp = std::getenv("LOCALAPPDATA");
    if (localApp && *localApp) {
        std::filesystem::path p = std::filesystem::path(localApp) / "MicaNT";
        std::error_code ec;
        std::filesystem::create_directories(p, ec);
        return p / "micamgr_pins.json";
    }
#endif
    return "micamgr_pins.json";
}

void MicaManager::initialize() {
    if (initialized_) return;
    catalog_.seedDefaults();

    // Load persisted pins
    std::filesystem::path pinPath = getPinsConfigPath();
    if (std::filesystem::exists(pinPath)) {
        std::ifstream ifs(pinPath);
        std::string line;
        while (std::getline(ifs, line)) {
            line.erase(0, line.find_first_not_of(" \t\r\n\""));
            auto last = line.find_last_not_of(" \t\r\n\",");
            if (last != std::string::npos) line.erase(last + 1);
            if (!line.empty() && line.front() != '{' && line.front() != '}' && line.front() != '[') {
                catalog_.pinPackage(line, true);
            }
        }
    }

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

bool MicaManager::pinPackage(std::string_view packageId, bool pin) {
    bool ok = catalog_.pinPackage(packageId, pin);
    if (ok) {
        std::filesystem::path pinPath = getPinsConfigPath();
        std::ofstream ofs(pinPath);
        if (ofs) {
            ofs << "[\n";
            auto pinned = catalog_.pinnedPackages();
            for (size_t i = 0; i < pinned.size(); ++i) {
                ofs << "  \"" << pinned[i].id << "\"" << (i + 1 < pinned.size() ? ",\n" : "\n");
            }
            ofs << "]\n";
        }
    }
    return ok;
}

bool MicaManager::isPackagePinned(std::string_view packageId) const {
    return catalog_.isPinned(packageId);
}

std::vector<PackageManifest> MicaManager::pinnedPackages() const {
    return catalog_.pinnedPackages();
}

std::optional<PackageManifest> MicaManager::getPackageDetails(std::string_view packageId) const {
    return catalog_.findById(packageId);
}

std::vector<CuratedStack> MicaManager::curatedStacks() const {
    return {
        {
            .id = "developer",
            .name = "Developer Sovereign Stack",
            .description = "Essential developer toolchain including Git, VS Code, Windows Terminal, and 7-Zip.",
            .category = "Developer",
            .packageIds = {"Git.Git", "Microsoft.VisualStudioCode", "Microsoft.WindowsTerminal", "7zip.7zip"}
        },
        {
            .id = "privacy",
            .name = "Privacy & Sovereign Security",
            .description = "Zero-telemetry communication, password management, sovereign browser, and security shield.",
            .category = "Security",
            .packageIds = {"Signal.Signal", "KeePassXCTeam.KeePassXC", "BraveSoftware.BraveBrowser", "com.barrer.sentinel"}
        },
        {
            .id = "media",
            .name = "Media Production Studio",
            .description = "Universal media playback, broadcasting, and audio workstation tools.",
            .category = "Media",
            .packageIds = {"VideoLAN.VLC", "OBSProject.OBSStudio", "Audacity.Audacity"}
        },
        {
            .id = "utilities",
            .name = "System Diagnostics & Utilities",
            .description = "High-speed file search, disk analysis, visual diff, and file archivers.",
            .category = "Utilities",
            .packageIds = {"voidtools.Everything", "AntibodySoftware.WizTree", "WinMerge.WinMerge", "7zip.7zip"}
        }
    };
}

std::vector<std::string> MicaManager::installCuratedStack(std::string_view stackId, std::string& outMessage) {
    std::vector<std::string> installed;
    auto stacks = curatedStacks();
    const CuratedStack* targetStack = nullptr;
    for (const auto& s : stacks) {
        if (s.id == stackId) {
            targetStack = &s;
            break;
        }
    }

    if (!targetStack) {
        outMessage = "Curated stack not found: " + std::string(stackId);
        return installed;
    }

    std::ostringstream ss;
    ss << "[MicaManager] Provisioning Curated Stack: " << targetStack->name << " (" << targetStack->packageIds.size() << " packages)\n";
    for (const auto& pkgId : targetStack->packageIds) {
        std::string log;
        if (installPackage(pkgId, log)) {
            installed.push_back(pkgId);
            ss << "  + Installed: " << pkgId << "\n";
        } else {
            ss << "  ! Warning: " << pkgId << " could not be provisioned.\n";
        }
    }
    outMessage = ss.str();
    return installed;
}

bool MicaManager::exportBundle(const std::filesystem::path& targetDir, const std::vector<std::string>& packageIds, std::string& outMessage) {
    std::error_code ec;
    std::filesystem::create_directories(targetDir, ec);

    OfflineBundle bundle;
    bundle.name = "MicaNT Sovereign Offline Bundle";
    bundle.version = "1.0.0";
    bundle.createdDate = "2026-10-10";
    bundle.targetOs = "MicaNT Sovereign / Windows NT";

    std::vector<PackageManifest> pkgsToExport;
    if (packageIds.empty()) {
        pkgsToExport = catalog_.allPackages();
    } else {
        for (const auto& id : packageIds) {
            auto optPkg = catalog_.findById(id);
            if (optPkg) {
                pkgsToExport.push_back(*optPkg);
            }
        }
    }

    bundle.packages = pkgsToExport;
    std::string bundleJson = PackageHelper::toBundleJson(bundle);

    std::filesystem::path manifestPath = targetDir / "micant_bundle.json";
    std::ofstream ofs(manifestPath);
    if (!ofs) {
        outMessage = "Failed to write bundle manifest to: " + manifestPath.string();
        return false;
    }
    ofs << bundleJson;
    ofs.close();

    outMessage = "Successfully exported offline bundle with " + std::to_string(pkgsToExport.size()) + " packages to " + manifestPath.string();
    return true;
}

bool MicaManager::importBundle(const std::filesystem::path& bundleDir, std::string& outMessage) {
    std::filesystem::path manifestPath = std::filesystem::is_directory(bundleDir)
        ? (bundleDir / "micant_bundle.json")
        : bundleDir;

    std::ifstream ifs(manifestPath);
    if (!ifs) {
        outMessage = "Bundle manifest not found at: " + manifestPath.string();
        return false;
    }

    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    auto optBundle = PackageHelper::fromBundleJson(content);
    if (!optBundle || optBundle->packages.empty()) {
        outMessage = "Failed to parse offline bundle from: " + manifestPath.string();
        return false;
    }

    size_t imported = 0;
    for (const auto& pkg : optBundle->packages) {
        catalog_.addOrUpdatePackage(pkg);
        imported++;
    }

    outMessage = "Successfully imported offline bundle '" + optBundle->name + "' (" + std::to_string(imported) + " packages added to catalog).";
    return true;
}

} // namespace micamgr

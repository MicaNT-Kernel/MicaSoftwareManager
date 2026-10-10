#ifndef MICAMGR_REGISTRY_SCANNER_HPP
#define MICAMGR_REGISTRY_SCANNER_HPP

#include <micamgr/types.hpp>
#include <vector>
#include <string>

namespace micamgr {

struct InstalledAppInfo {
    std::string displayName;
    std::string displayVersion;
    std::string publisher;
    std::string installLocation;
    std::string uninstallString;
    std::string quietUninstallString;
    uint64_t estimatedSizeBytes{0};
    bool isSystemComponent{false};
    std::string matchedPackageId;
};

class RegistryScanner {
public:
    /// Scans the system for installed Win32 software.
    /// On Windows, queries HKLM and HKCU Uninstall registry trees.
    static std::vector<InstalledAppInfo> scanInstalledApps();

    /// Attempts to correlate an installed application name with a package in the catalog.
    static std::string matchToCatalogId(const std::string& displayName, const std::vector<PackageManifest>& catalogPackages);
};

} // namespace micamgr

#endif // MICAMGR_REGISTRY_SCANNER_HPP

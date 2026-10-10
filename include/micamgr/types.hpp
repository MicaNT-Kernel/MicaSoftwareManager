#ifndef MICAMGR_TYPES_HPP
#define MICAMGR_TYPES_HPP

#include <string>
#include <string_view>
#include <vector>
#include <cstdint>

namespace micamgr {

enum class PackageSource {
    WinGet,
    AospWsa,
    MicaNative,
    PortableZip,
    Unknown
};

enum class PackageCategory {
    All,
    Featured,
    Productivity,
    Developer,
    Media,
    Utilities,
    Games,
    AndroidWsa,
    System,
    Updates
};

enum class InstallState {
    NotInstalled,
    Installed,
    UpdateAvailable,
    Installing,
    Error
};

enum class Architecture {
    Universal,
    X64,
    X86,
    Arm64
};

struct PackageManifest {
    std::string id;
    std::string name;
    std::string version;
    std::string installedVersion;
    std::string vendor;
    std::string license;
    std::string homepage;
    std::string downloadUrl;
    std::string sha256;
    std::string architecture{"x64"};
    PackageCategory category{PackageCategory::Utilities};
    PackageSource source{PackageSource::MicaNative};
    std::string description;
    std::string iconGlyph{"[PKG]"};
    std::string iconId{"Package"};
    InstallState state{InstallState::NotInstalled};
    std::string uninstallString;
    uint64_t installSizeBytes{0};

    [[nodiscard]] bool isUpdateAvailable() const noexcept {
        return state == InstallState::UpdateAvailable;
    }

    [[nodiscard]] std::string categoryToString() const {
        switch (category) {
            case PackageCategory::All: return "All";
            case PackageCategory::Featured: return "Featured";
            case PackageCategory::Productivity: return "Productivity";
            case PackageCategory::Developer: return "Developer";
            case PackageCategory::Media: return "Media";
            case PackageCategory::Utilities: return "Utilities";
            case PackageCategory::Games: return "Games";
            case PackageCategory::AndroidWsa: return "Android & WSA";
            case PackageCategory::System: return "System & Runtime";
            case PackageCategory::Updates: return "Updates";
            default: return "Other";
        }
    }

    [[nodiscard]] std::string sourceToString() const {
        switch (source) {
            case PackageSource::WinGet: return "WinGet";
            case PackageSource::AospWsa: return "Android AOSP (MicaG)";
            case PackageSource::MicaNative: return "MicaNT Sovereign";
            case PackageSource::PortableZip: return "Portable";
            default: return "Unknown";
        }
    }
};

struct SideloadInspectResult {
    bool valid{false};
    std::string format; // "PE/Win32", "APK/AOSP", "MSIX", "ZIP"
    std::string sha256;
    uint64_t fileSizeBytes{0};
    std::string architecture;
    std::string appName;
    std::string packageId;
    std::string version;
    std::string errorMessage;
};

} // namespace micamgr

#endif // MICAMGR_TYPES_HPP

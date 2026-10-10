#include <micamgr/catalog.hpp>
#include <micamgr/package.hpp>
#include <algorithm>

namespace micamgr {

static std::string toLower(std::string_view sv) {
    std::string s;
    s.reserve(sv.size());
    for (char c : sv) {
        s.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    return s;
}

Catalog::Catalog() {
    seedDefaults();
}

void Catalog::seedDefaults() {
    std::lock_guard lock(mutex_);
    packages_.clear();

    // --- 1. MicaNT Sovereign Software Suite (Barrer Software) ---
    packages_.push_back({
        .id = "com.barrer.surshell",
        .name = "SurShell Sovereign Desktop",
        .version = "1.0.0",
        .installedVersion = "1.0.0",
        .vendor = "Barrer Software",
        .license = "MIT",
        .homepage = "https://github.com/MicaNT-Kernel/SurShell",
        .downloadUrl = "https://github.com/MicaNT-Kernel/SurShell/releases/download/v1.0.0/surshell_app.exe",
        .sha256 = "c18f50438102377b21ecf4299b8e219736e053cb3971e428e9326e5d8a07c95a",
        .architecture = "x64",
        .category = PackageCategory::System,
        .source = PackageSource::MicaNative,
        .description = "Next-generation sovereign desktop shell for MicaNT and Windows NT, zero telemetry, pure ISO C++23.",
        .iconGlyph = "[SUR]",
        .iconId = "ShieldAdmin",
        .state = InstallState::Installed
    });

    packages_.push_back({
        .id = "com.barrer.micag",
        .name = "MicaG Sovereign Services & TPM Attestation",
        .version = "1.0.0",
        .installedVersion = "1.0.0",
        .vendor = "Barrer Software",
        .license = "Clean-Room Fair Use",
        .homepage = "https://github.com/MicaNT-Kernel/MicaG",
        .downloadUrl = "https://github.com/MicaNT-Kernel/MicaG/releases/download/v1.0.0/MicaG-Core.apk",
        .sha256 = "146927640728c7f217ec37ff5a22d46e7f21221b72a6ebbb4377bb46c6508931",
        .architecture = "universal",
        .category = PackageCategory::System,
        .source = PackageSource::MicaNative,
        .description = "Hardware-backed TPM 2.0 attestation broker, clean-room AIDL Parcel dispatcher, and hypervisor bridge.",
        .iconGlyph = "[MCG]",
        .iconId = "ShieldAdmin",
        .state = InstallState::Installed
    });

    packages_.push_back({
        .id = "com.barrer.sentinel",
        .name = "Clean-Room Sentinel Legal Auditor",
        .version = "1.2.0",
        .installedVersion = "1.1.0",
        .vendor = "Barrer Software",
        .license = "MIT",
        .homepage = "https://github.com/MicaNT-Kernel/clean-room-sentinel",
        .downloadUrl = "https://github.com/MicaNT-Kernel/clean-room-sentinel/releases/v1.2.0/sentinel.exe",
        .sha256 = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
        .architecture = "x64",
        .category = PackageCategory::Developer,
        .source = PackageSource::MicaNative,
        .description = "Automated AST and provenance scanner ensuring clean-room copyright compliance and IP boundaries.",
        .iconGlyph = "[SNT]",
        .iconId = "ShieldAdmin",
        .state = InstallState::UpdateAvailable
    });

    packages_.push_back({
        .id = "com.barrer.haven",
        .name = "Haven AI Companion & Server",
        .version = "2.1.0",
        .installedVersion = "2.1.0",
        .vendor = "Barrer Software",
        .license = "Apache-2.0",
        .homepage = "https://github.com/MicaNT-Kernel/haven-companions",
        .downloadUrl = "https://github.com/MicaNT-Kernel/haven-companions/releases/v2.1.0/haven_server.exe",
        .sha256 = "a591a6d40bf420404a011733cfb7b190d62c65bf0bcda32b57b277d9ad9f146e",
        .architecture = "x64",
        .category = PackageCategory::Productivity,
        .source = PackageSource::MicaNative,
        .description = "Local privacy-first multimodal AI companion stack with ASP.NET Core backend and GGUF acceleration.",
        .iconGlyph = "[HVN]",
        .iconId = "Bot",
        .state = InstallState::Installed
    });

    packages_.push_back({
        .id = "com.barrer.prismx",
        .name = "PrismX Sovereign IDE",
        .version = "1.5.0",
        .installedVersion = "",
        .vendor = "Barrer Software",
        .license = "MIT",
        .homepage = "https://github.com/MicaNT-Kernel/PrismX",
        .downloadUrl = "https://github.com/MicaNT-Kernel/PrismX/releases/v1.5.0/prismx.exe",
        .sha256 = "01ba4719c80b6fe911b091a7c05124b64eeece964e09c058ef8f9805daca546b",
        .architecture = "x64",
        .category = PackageCategory::Developer,
        .source = PackageSource::MicaNative,
        .description = "Ultra-fast low-latency code editing studio engineered in ISO C++23 with native language server protocol.",
        .iconGlyph = "[PRX]",
        .iconId = "Code",
        .state = InstallState::NotInstalled
    });

    packages_.push_back({
        .id = "com.barrer.avatarstudio",
        .name = "Barrer Avatar Studio",
        .version = "1.1.0",
        .installedVersion = "",
        .vendor = "Barrer Software",
        .license = "MIT",
        .homepage = "https://github.com/MicaNT-Kernel/BarrerAvatarStudio",
        .downloadUrl = "https://github.com/MicaNT-Kernel/BarrerAvatarStudio/releases/v1.1.0/avatar_studio.exe",
        .sha256 = "4355a46b19d348dc2f57c046f8ef63d4538ebb936000f3c9ee954a27460dd865",
        .architecture = "x64",
        .category = PackageCategory::Media,
        .source = PackageSource::MicaNative,
        .description = "DirectX 12 real-time 3D expressive character rigging and speech viseme synthesis pipeline.",
        .iconGlyph = "[AVT]",
        .iconId = "Image",
        .state = InstallState::NotInstalled
    });

    // --- 2. WinGet Curated Win32 Software Catalog ---
    packages_.push_back({
        .id = "Mozilla.Firefox",
        .name = "Mozilla Firefox",
        .version = "135.0.0",
        .installedVersion = "134.0.0",
        .vendor = "Mozilla Foundation",
        .license = "MPL-2.0",
        .homepage = "https://www.mozilla.org/firefox",
        .downloadUrl = "https://ftp.mozilla.org/pub/firefox/releases/135.0/win64/en-US/Firefox%20Setup%20135.0.exe",
        .sha256 = "5f4dcc3b5aa765d61d8327deb882cf992b959902cf691507f7634f199be35f41",
        .architecture = "x64",
        .category = PackageCategory::Featured,
        .source = PackageSource::WinGet,
        .description = "Fast, private, and open-source web browser that puts user agency and privacy first.",
        .iconGlyph = "[FFX]",
        .iconId = "Globe",
        .state = InstallState::UpdateAvailable
    });

    packages_.push_back({
        .id = "Microsoft.VisualStudioCode",
        .name = "Visual Studio Code",
        .version = "1.96.2",
        .installedVersion = "1.96.2",
        .vendor = "Microsoft",
        .license = "MIT",
        .homepage = "https://code.visualstudio.com",
        .downloadUrl = "https://update.code.visualstudio.com/1.96.2/win32-x64-user/stable",
        .sha256 = "8f3079b76e5d7990b79cfb76735c0529d4949176378a514d8641ed2927c8a14d",
        .architecture = "x64",
        .category = PackageCategory::Developer,
        .source = PackageSource::WinGet,
        .description = "Lightweight but powerful source code editor with built-in Git, debugging, and extensions.",
        .iconGlyph = "[VSC]",
        .iconId = "Code",
        .state = InstallState::Installed
    });

    packages_.push_back({
        .id = "Git.Git",
        .name = "Git for Windows",
        .version = "2.48.1",
        .installedVersion = "2.48.1",
        .vendor = "The Git Community",
        .license = "GPL-2.0",
        .homepage = "https://git-scm.com",
        .downloadUrl = "https://github.com/git-for-windows/git/releases/download/v2.48.1.windows.1/Git-2.48.1-64-bit.exe",
        .sha256 = "a5e9a4f6bbd57f9cb8944510b64d1f2b2c3a5e128148b5606d7d519b165d38f8",
        .architecture = "x64",
        .category = PackageCategory::Developer,
        .source = PackageSource::WinGet,
        .description = "Fast and distributed version control system designed for handling everything from small to large projects.",
        .iconGlyph = "[GIT]",
        .iconId = "Terminal",
        .state = InstallState::Installed
    });

    packages_.push_back({
        .id = "VideoLAN.VLC",
        .name = "VLC Media Player",
        .version = "3.0.21",
        .installedVersion = "",
        .vendor = "VideoLAN",
        .license = "GPL-2.0-or-later",
        .homepage = "https://www.videolan.org/vlc",
        .downloadUrl = "https://get.videolan.org/vlc/3.0.21/win64/vlc-3.0.21-win64.exe",
        .sha256 = "9b646c039fb2fc7cb5b5258db5465bb6d582bb83bfdf3bb0ecff31422791ef64",
        .architecture = "x64",
        .category = PackageCategory::Media,
        .source = PackageSource::WinGet,
        .description = "Free and open-source cross-platform multimedia player and framework playing most multimedia files.",
        .iconGlyph = "[VLC]",
        .iconId = "Video",
        .state = InstallState::NotInstalled
    });

    packages_.push_back({
        .id = "7zip.7zip",
        .name = "7-Zip Archiver",
        .version = "24.09",
        .installedVersion = "24.09",
        .vendor = "Igor Pavlov",
        .license = "LGPL-2.1-or-later",
        .homepage = "https://www.7-zip.org",
        .downloadUrl = "https://www.7-zip.org/a/7z2409-x64.exe",
        .sha256 = "294101e4a13b63294becc8ebfe1be44e23cf6347895cf358cf5f6bbd0d48f936",
        .architecture = "x64",
        .category = PackageCategory::Utilities,
        .source = PackageSource::WinGet,
        .description = "High compression ratio file archiver with support for 7z, ZIP, TAR, GZ, and RAR unpacking.",
        .iconGlyph = "[7ZP]",
        .iconId = "Archive",
        .state = InstallState::Installed
    });

    packages_.push_back({
        .id = "Notepad++.Notepad++",
        .name = "Notepad++",
        .version = "8.7.5",
        .installedVersion = "",
        .vendor = "Don Ho",
        .license = "GPL-3.0-or-later",
        .homepage = "https://notepad-plus-plus.org",
        .downloadUrl = "https://github.com/notepad-plus-plus/notepad-plus-plus/releases/download/v8.7.5/npp.8.7.5.Installer.x64.exe",
        .sha256 = "d225492acfe4e4c2741982b6b0631f4a4f89d3119c8f041ffef5f930e01c54ef",
        .architecture = "x64",
        .category = PackageCategory::Utilities,
        .source = PackageSource::WinGet,
        .description = "Free and lightweight text and source code editor supporting several programming languages.",
        .iconGlyph = "[NPP]",
        .iconId = "FileText",
        .state = InstallState::NotInstalled
    });

    packages_.push_back({
        .id = "OBSProject.OBSStudio",
        .name = "OBS Studio",
        .version = "31.0.1",
        .installedVersion = "",
        .vendor = "OBS Project",
        .license = "GPL-2.0-or-later",
        .homepage = "https://obsproject.com",
        .downloadUrl = "https://cdn-fastly.obsproject.com/downloads/OBS-Studio-31.0.1-Windows-Installer.exe",
        .sha256 = "e4d909c290d0fb1ca068ffaddf22cbd0add91ff292a8397a61d154ee4266fb8d",
        .architecture = "x64",
        .category = PackageCategory::Media,
        .source = PackageSource::WinGet,
        .description = "Free and open-source software for video recording and live streaming with hardware acceleration.",
        .iconGlyph = "[OBS]",
        .iconId = "Video",
        .state = InstallState::NotInstalled
    });

    packages_.push_back({
        .id = "BlenderFoundation.Blender",
        .name = "Blender 3D",
        .version = "4.3.2",
        .installedVersion = "",
        .vendor = "Blender Foundation",
        .license = "GPL-2.0-or-later",
        .homepage = "https://www.blender.org",
        .downloadUrl = "https://download.blender.org/release/Blender4.3/blender-4.3.2-windows-x64.msi",
        .sha256 = "91a8c3d9a10123985472098b6711902409827019823479012349817290123456",
        .architecture = "x64",
        .category = PackageCategory::Media,
        .source = PackageSource::WinGet,
        .description = "Free and open-source 3D creation suite supporting modeling, rigging, animation, and simulation.",
        .iconGlyph = "[BLN]",
        .iconId = "Image",
        .state = InstallState::NotInstalled
    });

    // --- 3. Clean-Room AOSP Android Catalog (WSA & MicaG Attestation) ---
    packages_.push_back({
        .id = "org.fdroid.fdroid",
        .name = "F-Droid AOSP Repository",
        .version = "1.20.0",
        .installedVersion = "1.20.0",
        .vendor = "F-Droid Limited",
        .license = "GPL-3.0",
        .homepage = "https://f-droid.org",
        .downloadUrl = "https://f-droid.org/F-Droid.apk",
        .sha256 = "41e86a07670984ee24ef0905e94b2f153a48e7e1a3d90615962bcfebe6cbe0eb",
        .architecture = "universal",
        .category = PackageCategory::AndroidWsa,
        .source = PackageSource::AospWsa,
        .description = "Installable catalog of FOSS (Free and Open Source Software) applications for the Android platform.",
        .iconGlyph = "[FDR]",
        .iconId = "Android",
        .state = InstallState::Installed
    });

    packages_.push_back({
        .id = "org.schabi.newpipe",
        .name = "NewPipe Streaming Player",
        .version = "0.27.2",
        .installedVersion = "0.27.0",
        .vendor = "Team NewPipe",
        .license = "GPL-3.0",
        .homepage = "https://newpipe.net",
        .downloadUrl = "https://github.com/TeamNewPipe/NewPipe/releases/download/v0.27.2/NewPipe_v0.27.2.apk",
        .sha256 = "8f43452b2c60822da5e2a089d4959f21f053593419a3b833350284822400e8de",
        .architecture = "universal",
        .category = PackageCategory::AndroidWsa,
        .source = PackageSource::AospWsa,
        .description = "Lightweight YouTube and media frontend with background playback and privacy protection.",
        .iconGlyph = "[NWP]",
        .iconId = "Video",
        .state = InstallState::UpdateAvailable
    });

    packages_.push_back({
        .id = "com.termux",
        .name = "Termux Terminal Emulator",
        .version = "0.118.0",
        .installedVersion = "",
        .vendor = "Termux Team",
        .license = "GPL-3.0",
        .homepage = "https://termux.dev",
        .downloadUrl = "https://github.com/termux/termux-app/releases/download/v0.118.0/termux-app_v0.118.0+github-debug_universal.apk",
        .sha256 = "07e324efc89d8924619b012487041a877be1c66710ae15b36449ddfa1c22ba10",
        .architecture = "universal",
        .category = PackageCategory::AndroidWsa,
        .source = PackageSource::AospWsa,
        .description = "Android terminal emulator and Linux environment app that works directly without rooting.",
        .iconGlyph = "[TMX]",
        .iconId = "Terminal",
        .state = InstallState::NotInstalled
    });
}

void Catalog::addOrUpdatePackage(const PackageManifest& pkg) {
    std::lock_guard lock(mutex_);
    for (auto& existing : packages_) {
        if (existing.id == pkg.id) {
            existing = pkg;
            return;
        }
    }
    packages_.push_back(pkg);
}

std::vector<PackageManifest> Catalog::search(std::string_view query, const std::function<bool(PackageSource)>& isSourceEnabled) const {
    std::lock_guard lock(mutex_);
    std::string lq = toLower(query);
    std::vector<PackageManifest> results;

    for (const auto& pkg : packages_) {
        if (isSourceEnabled && !isSourceEnabled(pkg.source)) {
            continue;
        }
        if (query.empty() ||
            toLower(pkg.name).find(lq) != std::string::npos ||
            toLower(pkg.id).find(lq) != std::string::npos ||
            toLower(pkg.vendor).find(lq) != std::string::npos ||
            toLower(pkg.description).find(lq) != std::string::npos) {
            results.push_back(pkg);
        }
    }
    return results;
}

std::vector<PackageManifest> Catalog::filterByCategory(PackageCategory cat, const std::function<bool(PackageSource)>& isSourceEnabled) const {
    std::lock_guard lock(mutex_);
    std::vector<PackageManifest> results;
    for (const auto& pkg : packages_) {
        if (isSourceEnabled && !isSourceEnabled(pkg.source)) {
            continue;
        }
        if (cat == PackageCategory::All) {
            results.push_back(pkg);
        } else if (cat == PackageCategory::Updates) {
            if (pkg.state == InstallState::UpdateAvailable) {
                results.push_back(pkg);
            }
        } else if (pkg.category == cat) {
            results.push_back(pkg);
        }
    }
    return results;
}

std::vector<PackageManifest> Catalog::filterBySource(PackageSource src) const {
    std::lock_guard lock(mutex_);
    std::vector<PackageManifest> results;
    for (const auto& pkg : packages_) {
        if (pkg.source == src) {
            results.push_back(pkg);
        }
    }
    return results;
}

std::optional<PackageManifest> Catalog::findById(std::string_view id) const {
    std::lock_guard lock(mutex_);
    for (const auto& pkg : packages_) {
        if (pkg.id == id) return pkg;
    }
    return std::nullopt;
}

std::vector<PackageManifest> Catalog::allPackages() const {
    std::lock_guard lock(mutex_);
    return packages_;
}

size_t Catalog::size() const {
    std::lock_guard lock(mutex_);
    return packages_.size();
}

bool Catalog::pinPackage(std::string_view id, bool pin) {
    std::lock_guard lock(mutex_);
    for (auto& pkg : packages_) {
        if (pkg.id == id) {
            pkg.isPinned = pin;
            return true;
        }
    }
    return false;
}

bool Catalog::isPinned(std::string_view id) const {
    std::lock_guard lock(mutex_);
    for (const auto& pkg : packages_) {
        if (pkg.id == id) return pkg.isPinned;
    }
    return false;
}

std::vector<PackageManifest> Catalog::pinnedPackages() const {
    std::lock_guard lock(mutex_);
    std::vector<PackageManifest> results;
    for (const auto& pkg : packages_) {
        if (pkg.isPinned) {
            results.push_back(pkg);
        }
    }
    return results;
}

bool Catalog::loadFromJson(std::string_view json) {
    auto parsed = PackageHelper::fromManifestListJson(json);
    if (parsed.empty()) return false;

    std::lock_guard lock(mutex_);
    for (const auto& pkg : parsed) {
        bool found = false;
        for (auto& existing : packages_) {
            if (existing.id == pkg.id) {
                existing = pkg;
                found = true;
                break;
            }
        }
        if (!found) {
            packages_.push_back(pkg);
        }
    }
    return true;
}

std::string Catalog::exportToJson() const {
    std::lock_guard lock(mutex_);
    return PackageHelper::toManifestListJson(packages_);
}

} // namespace micamgr

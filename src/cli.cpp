#include <micamgr/cli.hpp>
#include <micamgr/manager.hpp>
#include <iostream>
#include <iomanip>

namespace micamgr {

void CliHandler::printVersion() {
    std::cout << "Mica Software Manager (micamgr) v1.0.0\n";
    std::cout << "Target: MicaNT Sovereign Operating System & Windows NT\n";
    std::cout << "Copyright (c) 2026 Barrer Software. Clean-room provenance.\n";
}

void CliHandler::printUsage() {
    std::cout << "Usage: micamgr <command> [arguments] [options]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  search <query>          Search packages across all sources (WinGet, AOSP, Sovereign)\n";
    std::cout << "  list [options]          List software packages\n";
    std::cout << "         --installed      List currently installed applications on this machine\n";
    std::cout << "         --updates        List packages with pending upgrades available\n";
    std::cout << "         --pinned         List software packages with version locks\n";
    std::cout << "  info <package_id>       Inspect detailed package manifest, SHA-256, and origin\n";
    std::cout << "  install <package_id>    Install or provision a software package\n";
    std::cout << "  uninstall <package_id>  Uninstall an application\n";
    std::cout << "  update [--all]          Check for updates or upgrade all outdated software\n";
    std::cout << "  pin <package_id>        Lock a package to prevent automatic upgrades\n";
    std::cout << "  unpin <package_id>      Unlock package to allow updates\n";
    std::cout << "  stack [list|install]    Manage and deploy curated sovereign software stacks\n";
    std::cout << "  bundle [export|import]  Export or import air-gapped offline installer bundles\n";
    std::cout << "  inspect <file_path>     Inspect an installer file (.exe, .msix, .apk, .zip)\n";
    std::cout << "  gui                     Launch the modern sovereign desktop interface\n";
    std::cout << "  version, -v             Print version information\n";
    std::cout << "  help, -h                Display this help documentation\n";
}

int CliHandler::run(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 0;
    }

    std::string cmd = argv[1];

    if (cmd == "version" || cmd == "-v" || cmd == "--version") {
        printVersion();
        return 0;
    }

    if (cmd == "help" || cmd == "-h" || cmd == "--help") {
        printUsage();
        return 0;
    }

    auto& mgr = MicaManager::instance();

    if (cmd == "search") {
        if (argc < 3) {
            std::cerr << "Error: 'search' requires a search query.\n";
            return 1;
        }
        std::string query = argv[2];
        auto results = mgr.catalog().search(query);

        std::cout << "\nFound " << results.size() << " packages matching '" << query << "':\n";
        std::cout << "------------------------------------------------------------------------------------\n";
        std::cout << std::left << std::setw(30) << "Package ID"
                  << std::setw(28) << "Name"
                  << std::setw(12) << "Version"
                  << std::setw(15) << "Source"
                  << "Status\n";
        std::cout << "------------------------------------------------------------------------------------\n";

        for (const auto& pkg : results) {
            std::string st = (pkg.state == InstallState::Installed) ? "Installed" :
                             (pkg.state == InstallState::UpdateAvailable) ? "Update Available" : "Available";
            std::cout << std::left << std::setw(30) << pkg.id
                      << std::setw(28) << pkg.name
                      << std::setw(12) << pkg.version
                      << std::setw(15) << pkg.sourceToString()
                      << st << "\n";
        }
        std::cout << "\n";
        return 0;
    }

    if (cmd == "list") {
        bool showInstalled = false;
        bool showUpdates = false;
        bool showPinned = false;

        for (int i = 2; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--installed") showInstalled = true;
            else if (arg == "--updates") showUpdates = true;
            else if (arg == "--pinned") showPinned = true;
        }

        if (showPinned) {
            auto pinned = mgr.pinnedPackages();
            std::cout << "\nPinned Software Packages (" << pinned.size() << "):\n";
            std::cout << "------------------------------------------------------------------------------------\n";
            std::cout << std::left << std::setw(30) << "Package ID"
                      << std::setw(28) << "Name"
                      << std::setw(14) << "Pinned Ver"
                      << "Status\n";
            std::cout << "------------------------------------------------------------------------------------\n";
            for (const auto& p : pinned) {
                std::cout << std::left << std::setw(30) << p.id
                          << std::setw(28) << p.name
                          << std::setw(14) << (p.installedVersion.empty() ? p.version : p.installedVersion)
                          << "PINNED (Locked)\n";
            }
            std::cout << "\n";
            return 0;
        }

        if (showUpdates) {
            auto updates = mgr.checkUpdates();
            std::cout << "\nPackages with pending updates (" << updates.size() << "):\n";
            std::cout << "------------------------------------------------------------------------------------\n";
            std::cout << std::left << std::setw(30) << "Package ID"
                      << std::setw(25) << "Name"
                      << std::setw(12) << "Installed"
                      << std::setw(12) << "Available"
                      << "Source\n";
            std::cout << "------------------------------------------------------------------------------------\n";
            for (const auto& u : updates) {
                std::cout << std::left << std::setw(30) << u.id
                          << std::setw(25) << u.name
                          << std::setw(12) << (u.installedVersion.empty() ? "?" : u.installedVersion)
                          << std::setw(12) << u.version
                          << u.sourceToString() << "\n";
            }
            std::cout << "\nRun 'micamgr update --all' to apply all updates.\n\n";
            return 0;
        }

        if (showInstalled) {
            const auto& apps = mgr.installedApps();
            std::cout << "\nInstalled Applications Discovered (" << apps.size() << "):\n";
            std::cout << "------------------------------------------------------------------------------------\n";
            std::cout << std::left << std::setw(36) << "Display Name"
                      << std::setw(14) << "Version"
                      << std::setw(25) << "Publisher"
                      << "Catalog ID\n";
            std::cout << "------------------------------------------------------------------------------------\n";
            for (const auto& a : apps) {
                std::cout << std::left << std::setw(36) << a.displayName.substr(0, 34)
                          << std::setw(14) << a.displayVersion.substr(0, 12)
                          << std::setw(25) << a.publisher.substr(0, 23)
                          << (a.matchedPackageId.empty() ? "-" : a.matchedPackageId) << "\n";
            }
            std::cout << "\n";
            return 0;
        }

        // Show all catalog packages
        auto all = mgr.catalog().allPackages();
        std::cout << "\nMica Software Catalog (" << all.size() << " packages):\n";
        std::cout << "------------------------------------------------------------------------------------\n";
        std::cout << std::left << std::setw(30) << "Package ID"
                  << std::setw(28) << "Name"
                  << std::setw(12) << "Version"
                  << std::setw(15) << "Source"
                  << "Category\n";
        std::cout << "------------------------------------------------------------------------------------\n";
        for (const auto& pkg : all) {
            std::cout << std::left << std::setw(30) << pkg.id
                      << std::setw(28) << pkg.name
                      << std::setw(12) << pkg.version
                      << std::setw(15) << pkg.sourceToString()
                      << pkg.categoryToString() << "\n";
        }
        std::cout << "\n";
        return 0;
    }

    if (cmd == "install") {
        if (argc < 3) {
            std::cerr << "Error: 'install' requires a package ID.\n";
            return 1;
        }
        std::string pkgId = argv[2];
        std::string log;
        bool ok = mgr.installPackage(pkgId, log);
        std::cout << log << "\n";
        return ok ? 0 : 1;
    }

    if (cmd == "uninstall") {
        if (argc < 3) {
            std::cerr << "Error: 'uninstall' requires a package ID or application name.\n";
            return 1;
        }
        std::string pkgId = argv[2];
        std::string log;
        bool ok = mgr.uninstallPackage(pkgId, log);
        std::cout << log << "\n";
        return ok ? 0 : 1;
    }

    if (cmd == "update") {
        bool updateAll = false;
        for (int i = 2; i < argc; ++i) {
            if (std::string(argv[i]) == "--all") updateAll = true;
        }

        if (updateAll) {
            std::cout << "[MicaManager] Updating all packages with pending upgrades...\n";
            auto reports = mgr.upgradeAll([](const std::string& id, float prog, const std::string& status) {
                std::cout << "  [" << static_cast<int>(prog * 100.0f) << "%] " << id << ": " << status << "\n";
            });
            std::cout << "[MicaManager] Updated " << reports.size() << " packages successfully.\n";
            return 0;
        } else {
            auto updates = mgr.checkUpdates();
            std::cout << "[MicaManager] " << updates.size() << " update(s) available.\n";
            for (const auto& u : updates) {
                std::cout << "  - " << u.name << " (" << u.id << "): " << u.installedVersion << " -> " << u.version << "\n";
            }
            std::cout << "Run 'micamgr update --all' to apply.\n";
            return 0;
        }
    }

    if (cmd == "pin") {
        if (argc < 3) {
            std::cerr << "Error: 'pin' requires a package ID.\n";
            return 1;
        }
        std::string pkgId = argv[2];
        bool ok = mgr.pinPackage(pkgId, true);
        if (ok) {
            std::cout << "[MicaManager] Package '" << pkgId << "' successfully pinned to current version.\n";
            return 0;
        } else {
            std::cerr << "Error: Package '" << pkgId << "' not found in catalog.\n";
            return 1;
        }
    }

    if (cmd == "unpin") {
        if (argc < 3) {
            std::cerr << "Error: 'unpin' requires a package ID.\n";
            return 1;
        }
        std::string pkgId = argv[2];
        bool ok = mgr.pinPackage(pkgId, false);
        if (ok) {
            std::cout << "[MicaManager] Package '" << pkgId << "' unpinned. Updates re-enabled.\n";
            return 0;
        } else {
            std::cerr << "Error: Package '" << pkgId << "' not found in catalog.\n";
            return 1;
        }
    }

    if (cmd == "info") {
        if (argc < 3) {
            std::cerr << "Error: 'info' requires a package ID.\n";
            return 1;
        }
        std::string pkgId = argv[2];
        auto optPkg = mgr.getPackageDetails(pkgId);
        if (!optPkg) {
            std::cerr << "Error: Package '" << pkgId << "' not found.\n";
            return 1;
        }
        const auto& p = *optPkg;
        std::cout << "\nPackage Manifest Inspector:\n";
        std::cout << "  Identifier:        " << p.id << "\n";
        std::cout << "  Display Name:      " << p.name << "\n";
        std::cout << "  Catalog Version:   v" << p.version << "\n";
        std::cout << "  Installed Version: " << (p.installedVersion.empty() ? "(Not Installed)" : ("v" + p.installedVersion)) << "\n";
        std::cout << "  Pinned:            " << (p.isPinned ? "YES (Upgrades Locked)" : "No") << "\n";
        std::cout << "  Vendor/Publisher:  " << p.vendor << "\n";
        std::cout << "  License:           " << p.license << "\n";
        std::cout << "  Category:          " << p.categoryToString() << "\n";
        std::cout << "  Source Subsystem:  " << p.sourceToString() << "\n";
        std::cout << "  Architecture:      " << p.architecture << "\n";
        std::cout << "  Homepage:          " << p.homepage << "\n";
        std::cout << "  Download URL:      " << p.downloadUrl << "\n";
        std::cout << "  FIPS SHA-256:      " << p.sha256 << "\n";
        std::cout << "  Description:       " << p.description << "\n\n";
        return 0;
    }

    if (cmd == "stack") {
        if (argc < 3 || std::string(argv[2]) == "list") {
            auto stacks = mgr.curatedStacks();
            std::cout << "\nCurated Sovereign Software Stacks (" << stacks.size() << "):\n";
            std::cout << "------------------------------------------------------------------------------------\n";
            for (const auto& s : stacks) {
                std::cout << "  [" << s.id << "] " << s.name << " (" << s.category << ")\n";
                std::cout << "      " << s.description << "\n";
                std::cout << "      Packages: ";
                for (size_t i = 0; i < s.packageIds.size(); ++i) {
                    std::cout << s.packageIds[i] << (i + 1 < s.packageIds.size() ? ", " : "\n");
                }
                std::cout << "\n";
            }
            std::cout << "Run 'micamgr stack install <stack_id>' to provision an entire curated stack.\n\n";
            return 0;
        }

        if (std::string(argv[2]) == "install") {
            if (argc < 4) {
                std::cerr << "Error: 'stack install' requires a stack ID (e.g. developer, privacy, media, utilities).\n";
                return 1;
            }
            std::string stackId = argv[3];
            std::string log;
            auto installed = mgr.installCuratedStack(stackId, log);
            std::cout << log << "\n";
            return installed.empty() ? 1 : 0;
        }
    }

    if (cmd == "bundle") {
        if (argc < 3) {
            std::cerr << "Usage: micamgr bundle export <target_dir> [package_ids...]\n";
            std::cerr << "       micamgr bundle import <bundle_dir_or_json>\n";
            return 1;
        }
        std::string sub = argv[2];
        if (sub == "export") {
            if (argc < 4) {
                std::cerr << "Error: 'bundle export' requires a target directory path.\n";
                return 1;
            }
            std::filesystem::path targetDir = argv[3];
            std::vector<std::string> pkgIds;
            for (int i = 4; i < argc; ++i) {
                pkgIds.push_back(argv[i]);
            }
            std::string msg;
            bool ok = mgr.exportBundle(targetDir, pkgIds, msg);
            std::cout << msg << "\n";
            return ok ? 0 : 1;
        }
        if (sub == "import") {
            if (argc < 4) {
                std::cerr << "Error: 'bundle import' requires a bundle directory or micant_bundle.json file path.\n";
                return 1;
            }
            std::filesystem::path bundlePath = argv[3];
            std::string msg;
            bool ok = mgr.importBundle(bundlePath, msg);
            std::cout << msg << "\n";
            return ok ? 0 : 1;
        }
    }

    if (cmd == "inspect") {
        if (argc < 3) {
            std::cerr << "Error: 'inspect' requires a path to a file.\n";
            return 1;
        }
        std::filesystem::path p = argv[2];
        auto res = mgr.inspectLocalFile(p);
        if (!res.valid) {
            std::cerr << "Inspection failed: " << res.errorMessage << "\n";
            return 1;
        }

        std::cout << "\nFile Inspection Result:\n";
        std::cout << "  Path:         " << p.string() << "\n";
        std::cout << "  Format:       " << res.format << "\n";
        std::cout << "  Architecture: " << res.architecture << "\n";
        std::cout << "  Size:         " << res.fileSizeBytes << " bytes (" << (res.fileSizeBytes / 1024) << " KB)\n";
        std::cout << "  SHA-256:      " << res.sha256 << "\n";
        std::cout << "  Valid:        YES\n\n";
        return 0;
    }

    std::cerr << "Unknown command: " << cmd << ". Run 'micamgr help' for usage.\n";
    return 1;
}

} // namespace micamgr

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
    std::cout << "  install <package_id>    Install or provision a software package\n";
    std::cout << "  uninstall <package_id>  Uninstall an application\n";
    std::cout << "  update [--all]          Check for updates or upgrade all outdated software\n";
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

        for (int i = 2; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--installed") showInstalled = true;
            else if (arg == "--updates") showUpdates = true;
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

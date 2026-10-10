#include <micamgr/package.hpp>
#include <micamgr/catalog.hpp>
#include <micamgr/registry_scanner.hpp>
#include <micamgr/update_engine.hpp>
#include <micamgr/sideload.hpp>
#include <micamgr/manager.hpp>
#include <micamgr/ui.hpp>

#include <iostream>
#include <fstream>
#include <cassert>
#include <cstdlib>

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "Assertion FAILED: " << (msg) << " (" #cond ") at line " << __LINE__ << std::endl; \
            std::exit(1); \
        } \
    } while(0)

void Test_Package_Helpers_And_Json() {
    std::cout << "[TEST] Running Suite 1: Package Helpers, Version Comparison & JSON Serialization...\n";

    // 1. Version Comparison
    TEST_ASSERT(micamgr::PackageHelper::compareVersions("1.0.0", "1.0.0") == 0, "1.0.0 == 1.0.0");
    TEST_ASSERT(micamgr::PackageHelper::compareVersions("1.0.1", "1.0.0") == 1, "1.0.1 > 1.0.0");
    TEST_ASSERT(micamgr::PackageHelper::compareVersions("1.0.0", "1.1.0") == -1, "1.0.0 < 1.1.0");
    TEST_ASSERT(micamgr::PackageHelper::compareVersions("2.0.0", "1.99.99") == 1, "2.0.0 > 1.99.99");
    TEST_ASSERT(micamgr::PackageHelper::compareVersions("0.27.0", "0.27.2") == -1, "0.27.0 < 0.27.2");

    // 2. JSON Serialization & Deserialization
    micamgr::PackageManifest pkg{
        .id = "test.app",
        .name = "Test App",
        .version = "2.0.1",
        .installedVersion = "2.0.0",
        .vendor = "Barrer Test",
        .license = "MIT",
        .homepage = "https://example.com",
        .downloadUrl = "https://example.com/app.exe",
        .sha256 = "1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef",
        .architecture = "x64",
        .category = micamgr::PackageCategory::Developer,
        .source = micamgr::PackageSource::MicaNative,
        .description = "Test package description",
        .iconGlyph = "[TST]",
        .iconId = "Code",
        .state = micamgr::InstallState::UpdateAvailable
    };

    std::string json = micamgr::PackageHelper::toJson(pkg);
    TEST_ASSERT(!json.empty(), "Serialized JSON is not empty");
    TEST_ASSERT(json.find("\"test.app\"") != std::string::npos, "JSON contains test.app");

    auto parsed = micamgr::PackageHelper::fromJson(json);
    TEST_ASSERT(parsed.has_value(), "Parsed JSON successfully");
    TEST_ASSERT(parsed->id == "test.app", "Parsed ID matches");
    TEST_ASSERT(parsed->version == "2.0.1", "Parsed version matches");
    TEST_ASSERT(parsed->category == micamgr::PackageCategory::Developer, "Parsed category matches");
    TEST_ASSERT(parsed->source == micamgr::PackageSource::MicaNative, "Parsed source matches");

    std::cout << "[TEST] Suite 1: Package Helpers, Version Comparison & JSON Serialization PASSED.\n";
}

void Test_Catalog_Engine() {
    std::cout << "[TEST] Running Suite 2: Catalog Seeding, Search & Filtering...\n";

    micamgr::Catalog catalog;
    TEST_ASSERT(catalog.size() > 10, "Default catalog has over 10 packages");

    // Check Sovereign Suite presence
    auto surshell = catalog.findById("com.barrer.surshell");
    TEST_ASSERT(surshell.has_value(), "Catalog contains com.barrer.surshell");
    TEST_ASSERT(surshell->vendor == "Barrer Software", "SurShell vendor is Barrer Software");

    auto micag = catalog.findById("com.barrer.micag");
    TEST_ASSERT(micag.has_value(), "Catalog contains com.barrer.micag");

    // Search tests
    auto firefoxResults = catalog.search("Firefox");
    TEST_ASSERT(firefoxResults.size() >= 1, "Search for Firefox finds packages");

    auto barrenResults = catalog.search("NONEXISTENT_QUERY_XYZ");
    TEST_ASSERT(barrenResults.empty(), "Search for non-existent package returns empty");

    // Category filter tests
    auto devApps = catalog.filterByCategory(micamgr::PackageCategory::Developer);
    TEST_ASSERT(!devApps.empty(), "Developer category has packages");

    auto androidApps = catalog.filterByCategory(micamgr::PackageCategory::AndroidWsa);
    TEST_ASSERT(!androidApps.empty(), "Android category has packages");

    // Source filter tests
    auto wingetApps = catalog.filterBySource(micamgr::PackageSource::WinGet);
    TEST_ASSERT(!wingetApps.empty(), "WinGet source has packages");

    // Export & Reload JSON
    std::string exported = catalog.exportToJson();
    TEST_ASSERT(exported.find("Mica Software Manager") != std::string::npos, "Exported JSON contains hub header");

    micamgr::Catalog newCatalog;
    bool loaded = newCatalog.loadFromJson(exported);
    TEST_ASSERT(loaded, "loadFromJson succeeded");
    TEST_ASSERT(newCatalog.size() >= catalog.size(), "Reloaded catalog has matching size");

    std::cout << "[TEST] Suite 2: Catalog Seeding, Search & Filtering PASSED.\n";
}

void Test_Registry_Scanner_And_Matching() {
    std::cout << "[TEST] Running Suite 3: Installed Apps Scanner & Catalog Correlation...\n";

    auto apps = micamgr::RegistryScanner::scanInstalledApps();
    TEST_ASSERT(!apps.empty(), "Registry scanner returned installed applications");

    micamgr::Catalog catalog;
    auto pkgs = catalog.allPackages();

    // Match detection test
    std::string matched = micamgr::RegistryScanner::matchToCatalogId("Mozilla Firefox 134.0 (x64)", pkgs);
    TEST_ASSERT(matched == "Mozilla.Firefox", "Correlated Mozilla Firefox to Mozilla.Firefox");

    std::string matchedCode = micamgr::RegistryScanner::matchToCatalogId("Microsoft Visual Studio Code", pkgs);
    TEST_ASSERT(matchedCode == "Microsoft.VisualStudioCode", "Correlated Visual Studio Code");

    std::cout << "[TEST] Suite 3: Installed Apps Scanner & Catalog Correlation PASSED.\n";
}

void Test_Update_Engine() {
    std::cout << "[TEST] Running Suite 4: Update Engine & Batch Upgrade Pipeline...\n";

    micamgr::Catalog catalog;
    std::vector<micamgr::InstalledAppInfo> apps = {
        {
            .displayName = "Mozilla Firefox",
            .displayVersion = "134.0.0",
            .matchedPackageId = "Mozilla.Firefox"
        },
        {
            .displayName = "NewPipe",
            .displayVersion = "0.27.0",
            .matchedPackageId = "org.schabi.newpipe"
        }
    };

    auto updates = micamgr::UpdateEngine::checkForUpdates(catalog, apps);
    TEST_ASSERT(!updates.empty(), "Detected pending updates");

    bool foundFirefox = false;
    for (const auto& u : updates) {
        if (u.id == "Mozilla.Firefox") {
            foundFirefox = true;
            TEST_ASSERT(u.state == micamgr::InstallState::UpdateAvailable, "Firefox marked as UpdateAvailable");
            TEST_ASSERT(u.installedVersion == "134.0.0", "Installed version recorded");
        }
    }
    TEST_ASSERT(foundFirefox, "Firefox identified in updates list");

    // Single update execution test
    auto report = micamgr::UpdateEngine::updatePackage(updates[0]);
    TEST_ASSERT(report.success, "updatePackage returned success");
    TEST_ASSERT(report.newVersion == updates[0].version, "Report reflects updated target version");

    // Batch update execution test
    int progressCalls = 0;
    auto batchReports = micamgr::UpdateEngine::updateAll(updates, [&](const std::string&, float, const std::string&) {
        progressCalls++;
    });
    TEST_ASSERT(batchReports.size() == updates.size(), "Batch update processed all pending updates");
    TEST_ASSERT(progressCalls > 0, "Progress callback was invoked");

    std::cout << "[TEST] Suite 4: Update Engine & Batch Upgrade Pipeline PASSED.\n";
}

void Test_Sideload_Inspector() {
    std::cout << "[TEST] Running Suite 5: Sideload Inspector & Hash Verification...\n";

    std::filesystem::path tempDir = std::filesystem::temp_directory_path();
    std::filesystem::path testPe = tempDir / "micant_test_sample.exe";
    std::filesystem::path testApk = tempDir / "micant_test_sample.apk";

    // 1. Create dummy PE file
    {
        std::ofstream ofs(testPe, std::ios::binary);
        std::vector<uint8_t> dummyPe(1024, 0);
        dummyPe[0] = 0x4D; dummyPe[1] = 0x5A; // MZ
        *reinterpret_cast<uint32_t*>(&dummyPe[0x3C]) = 0x80; // PE offset
        dummyPe[0x80] = 'P'; dummyPe[0x81] = 'E'; dummyPe[0x82] = 0; dummyPe[0x83] = 0;
        *reinterpret_cast<uint16_t*>(&dummyPe[0x84]) = 0x8664; // x64
        ofs.write(reinterpret_cast<char*>(dummyPe.data()), dummyPe.size());
    }

    auto peResult = micamgr::SideloadInspector::inspectFile(testPe);
    TEST_ASSERT(peResult.valid, "PE inspection succeeded");
    TEST_ASSERT(peResult.format == "PE/Win32 Executable", "Format detected as PE/Win32");
    TEST_ASSERT(peResult.architecture == "x64", "Architecture detected as x64");
    TEST_ASSERT(!peResult.sha256.empty(), "SHA-256 computed");

    // 2. Create dummy APK file
    {
        std::ofstream ofs(testApk, std::ios::binary);
        const char zipHeader[4] = {'P', 'K', 0x03, 0x04};
        ofs.write(zipHeader, 4);
        ofs.write("AndroidManifest.xml_TEST_DATA", 29);
    }

    auto apkResult = micamgr::SideloadInspector::inspectFile(testApk);
    TEST_ASSERT(apkResult.valid, "APK inspection succeeded");
    TEST_ASSERT(apkResult.format == "APK/Android (AOSP)", "Format detected as APK/Android (AOSP)");
    TEST_ASSERT(apkResult.architecture == "universal", "APK architecture is universal");

    // 3. Clean up
    std::error_code ec;
    std::filesystem::remove(testPe, ec);
    std::filesystem::remove(testApk, ec);

    std::cout << "[TEST] Suite 5: Sideload Inspector & Hash Verification PASSED.\n";
}

void Test_Sideload_Execution() {
    std::cout << "[TEST] Running Suite 6: Sideload Staging & Deployment...\n";

    micamgr::SideloadInspectResult mockRes{
        .valid = true,
        .format = "APK/Android (AOSP)",
        .sha256 = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
        .architecture = "universal"
    };

    std::string log;
    bool ok = micamgr::SideloadInspector::executeSideload("sample.apk", mockRes, log);
    TEST_ASSERT(ok, "executeSideload succeeded for APK");
    TEST_ASSERT(log.find("MicaG TPM 2.0") != std::string::npos, "Sideload references MicaG TPM 2.0 attestation");

    std::cout << "[TEST] Suite 6: Sideload Staging & Deployment PASSED.\n";
}

void Test_Master_MicaManager() {
    std::cout << "[TEST] Running Suite 7: Master MicaManager Controller...\n";

    auto& mgr = micamgr::MicaManager::instance();
    mgr.refreshInstalledApps();

    TEST_ASSERT(mgr.catalog().size() > 0, "Catalog initialized");
    TEST_ASSERT(!mgr.installedApps().empty(), "Installed apps inventory populated");

    // Install package
    std::string msg;
    bool inst = mgr.installPackage("com.barrer.prismx", msg);
    TEST_ASSERT(inst, "installPackage succeeded for PrismX");

    auto pPrism = mgr.catalog().findById("com.barrer.prismx");
    TEST_ASSERT(pPrism.has_value(), "PrismX found");
    TEST_ASSERT(pPrism->state == micamgr::InstallState::Installed, "PrismX state updated to Installed");

    // Uninstall package
    bool uninst = mgr.uninstallPackage("com.barrer.prismx", msg);
    TEST_ASSERT(uninst, "uninstallPackage succeeded");
    pPrism = mgr.catalog().findById("com.barrer.prismx");
    TEST_ASSERT(pPrism->state == micamgr::InstallState::NotInstalled, "PrismX state updated to NotInstalled");

    std::cout << "[TEST] Suite 7: Master MicaManager Controller PASSED.\n";
}

void Test_Software_Manager_UI() {
    std::cout << "[TEST] Running Suite 8: SoftwareManagerUI Layout & Interaction...\n";

    micamgr::SoftwareManagerUI ui;
    ui.updateLayout(1000, 700);

    TEST_ASSERT(ui.activeTab() == micamgr::ManagerTab::Browse, "Default tab is Browse");
    TEST_ASSERT(ui.tabBrowseBounds().width > 0, "tabBrowseBounds computed");
    TEST_ASSERT(ui.tabLibraryBounds().width > 0, "tabLibraryBounds computed");

    // Tab switching via hit testing
    bool clickedLibrary = ui.onMouseDown(ui.tabLibraryBounds().x + 10, ui.tabLibraryBounds().y + 10);
    TEST_ASSERT(clickedLibrary, "onMouseDown handled click on Library tab");
    TEST_ASSERT(ui.activeTab() == micamgr::ManagerTab::Library, "Active tab is now Library");

    bool clickedBrowse = ui.onMouseDown(ui.tabBrowseBounds().x + 10, ui.tabBrowseBounds().y + 10);
    TEST_ASSERT(clickedBrowse, "onMouseDown handled click on Browse tab");
    TEST_ASSERT(ui.activeTab() == micamgr::ManagerTab::Browse, "Active tab is now Browse");

    // Search query interaction
    ui.onSearchInput("VLC");
    TEST_ASSERT(ui.searchQuery() == "VLC", "Search query set to VLC");
    TEST_ASSERT(!ui.filteredPackages().empty(), "Filtered packages not empty for VLC");

    std::cout << "[TEST] Suite 8: SoftwareManagerUI Layout & Interaction PASSED.\n";
}

void Test_Package_Pinning() {
    std::cout << "[TEST] Running Suite 9: Package Version Pinning & Protection...\n";

    auto& mgr = micamgr::MicaManager::instance();
    TEST_ASSERT(!mgr.isPackagePinned("7zip.7zip"), "7zip starts unpinned");

    bool pinOk = mgr.pinPackage("7zip.7zip", true);
    TEST_ASSERT(pinOk, "pinPackage returned true");
    TEST_ASSERT(mgr.isPackagePinned("7zip.7zip"), "7zip is now pinned");

    auto pinned = mgr.pinnedPackages();
    TEST_ASSERT(pinned.size() >= 1, "pinnedPackages contains at least 1 package");
    bool found7z = false;
    for (const auto& p : pinned) {
        if (p.id == "7zip.7zip") {
            found7z = true;
            TEST_ASSERT(p.isPinned, "p.isPinned is true");
            break;
        }
    }
    TEST_ASSERT(found7z, "7zip.7zip found in pinned list");

    // Test that UpdateEngine skips or reports failure when upgrading a pinned package
    auto optPkg = mgr.catalog().findById("7zip.7zip");
    TEST_ASSERT(optPkg.has_value(), "7zip found in catalog");
    auto rep = micamgr::UpdateEngine::updatePackage(*optPkg);
    TEST_ASSERT(!rep.success, "updatePackage skipped on pinned package");
    TEST_ASSERT(rep.message.find("pinned") != std::string::npos, "Message mentions pinned status");

    // Unpin
    bool unpinOk = mgr.pinPackage("7zip.7zip", false);
    TEST_ASSERT(unpinOk, "unpinPackage returned true");
    TEST_ASSERT(!mgr.isPackagePinned("7zip.7zip"), "7zip is unpinned");

    std::cout << "[TEST] Suite 9: Package Version Pinning & Protection PASSED.\n";
}

void Test_Curated_Stacks() {
    std::cout << "[TEST] Running Suite 10: Curated Sovereign Software Stacks...\n";

    auto& mgr = micamgr::MicaManager::instance();
    auto stacks = mgr.curatedStacks();
    TEST_ASSERT(stacks.size() >= 4, "At least 4 curated stacks defined");

    bool foundDev = false;
    for (const auto& s : stacks) {
        if (s.id == "developer") {
            foundDev = true;
            TEST_ASSERT(s.packageIds.size() >= 3, "Developer stack has multiple packages");
            break;
        }
    }
    TEST_ASSERT(foundDev, "Developer stack found");

    std::string msg;
    auto installed = mgr.installCuratedStack("developer", msg);
    TEST_ASSERT(!installed.empty(), "installCuratedStack provisioned developer packages");
    TEST_ASSERT(msg.find("Developer Sovereign Stack") != std::string::npos, "Log message contains stack title");

    std::cout << "[TEST] Suite 10: Curated Sovereign Software Stacks PASSED.\n";
}

void Test_Offline_Bundle_Export_Import() {
    std::cout << "[TEST] Running Suite 11: Air-Gapped / Offline Bundle Pack...\n";

    auto& mgr = micamgr::MicaManager::instance();
    std::filesystem::path tempDir = std::filesystem::temp_directory_path() / "micant_bundle_test";
    std::error_code ec;
    std::filesystem::remove_all(tempDir, ec);

    std::string msg;
    bool exported = mgr.exportBundle(tempDir, {"7zip.7zip", "Mozilla.Firefox"}, msg);
    TEST_ASSERT(exported, "exportBundle succeeded");
    TEST_ASSERT(std::filesystem::exists(tempDir / "micant_bundle.json"), "micant_bundle.json written to disk");

    std::string importMsg;
    bool imported = mgr.importBundle(tempDir, importMsg);
    TEST_ASSERT(imported, "importBundle succeeded");
    TEST_ASSERT(importMsg.find("imported offline bundle") != std::string::npos, "Import message reports success");

    std::filesystem::remove_all(tempDir, ec);
    std::cout << "[TEST] Suite 11: Air-Gapped / Offline Bundle Pack PASSED.\n";
}

void Test_Package_Details_Inspection() {
    std::cout << "[TEST] Running Suite 12: Package Details & Manifest Inspection...\n";

    auto& mgr = micamgr::MicaManager::instance();
    auto optPkg = mgr.getPackageDetails("Mozilla.Firefox");
    TEST_ASSERT(optPkg.has_value(), "getPackageDetails found Firefox");
    TEST_ASSERT(optPkg->vendor == "Mozilla Foundation", "Vendor is Mozilla Foundation");
    TEST_ASSERT(!optPkg->sha256.empty(), "SHA-256 is present");
    TEST_ASSERT(!optPkg->downloadUrl.empty(), "Download URL is present");
    TEST_ASSERT(!optPkg->license.empty(), "License is present");

    std::cout << "[TEST] Suite 12: Package Details & Manifest Inspection PASSED.\n";
}

int main() {
    std::cout << "======================================================================\n";
    std::cout << "Mica Software Manager (micamgr) Verification Suite\n";
    std::cout << "Standard: ISO C++23 | Zero Dependencies | Clean-Room Provenance\n";
    std::cout << "======================================================================\n";

    Test_Package_Helpers_And_Json();
    Test_Catalog_Engine();
    Test_Registry_Scanner_And_Matching();
    Test_Update_Engine();
    Test_Sideload_Inspector();
    Test_Sideload_Execution();
    Test_Master_MicaManager();
    Test_Software_Manager_UI();
    Test_Package_Pinning();
    Test_Curated_Stacks();
    Test_Offline_Bundle_Export_Import();
    Test_Package_Details_Inspection();

    std::cout << "======================================================================\n";
    std::cout << "ALL 12 MICA SOFTWARE MANAGER SUBSYSTEM TESTS PASSED (100% SUCCESS)\n";
    std::cout << "======================================================================\n";

    return 0;
}

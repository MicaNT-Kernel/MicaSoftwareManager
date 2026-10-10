#ifndef MICAMGR_UI_HPP
#define MICAMGR_UI_HPP

#include <micamgr/types.hpp>
#include <micamgr/manager.hpp>
#include <string>
#include <vector>
#include <functional>
#include <cstdint>

namespace micamgr {

enum class ManagerTab {
    Browse,
    Library,
    Sideload,
    Settings
};

struct UiRect {
    int x{0};
    int y{0};
    int width{0};
    int height{0};

    [[nodiscard]] bool contains(int px, int py) const noexcept {
        return px >= x && px < (x + width) && py >= y && py < (y + height);
    }
};

struct SourceCardUi {
    std::string sourceId;
    UiRect bounds;
    UiRect toggleBtnBounds;
    UiRect deleteBtnBounds;
};

class SoftwareManagerUI {
public:
    SoftwareManagerUI();

    void setActiveTab(ManagerTab tab) noexcept;
    [[nodiscard]] ManagerTab activeTab() const noexcept { return activeTab_; }

    void setSearchQuery(std::string_view query);
    [[nodiscard]] const std::string& searchQuery() const noexcept { return searchQuery_; }

    void setCategory(PackageCategory cat) noexcept;
    [[nodiscard]] PackageCategory activeCategory() const noexcept { return activeCategory_; }

    /// Re-evaluates filtered packages from cached catalog without rescanning registry (instantaneous).
    void updateFilteredPackages();

    /// Refreshes catalog view. If forceScan is true, triggers a full system inventory re-scan.
    void refresh(bool forceScan = false);

    /// Layout calculation for given window width & height.
    void updateLayout(int width, int height);

    /// Scrolling offset management
    void scroll(int deltaY) noexcept;
    [[nodiscard]] int scrollY() const noexcept { return scrollY_; }

    /// Sideload inspector target path.
    void setInspectFilePath(const std::filesystem::path& path);
    [[nodiscard]] const SideloadInspectResult& inspectResult() const noexcept { return inspectResult_; }

    /// Action handlers
    bool onMouseDown(int x, int y);
    void onSearchInput(std::string_view text);

    [[nodiscard]] const std::string& statusMessage() const noexcept { return statusMessage_; }
    void setStatusMessage(std::string_view msg) { statusMessage_ = msg; }

    void setOnAddSource(std::function<void()> cb) { onAddSource_ = std::move(cb); }

    [[nodiscard]] const std::vector<PackageManifest>& filteredPackages() const noexcept { return filteredPackages_; }
    [[nodiscard]] const std::vector<PackageManifest>& pendingUpdates() const noexcept { return pendingUpdates_; }

    /// Tab Bounds for hit-testing
    [[nodiscard]] UiRect tabBrowseBounds() const noexcept { return tabBrowseBounds_; }
    [[nodiscard]] UiRect tabLibraryBounds() const noexcept { return tabLibraryBounds_; }
    [[nodiscard]] UiRect tabSideloadBounds() const noexcept { return tabSideloadBounds_; }
    [[nodiscard]] UiRect tabSettingsBounds() const noexcept { return tabSettingsBounds_; }
    [[nodiscard]] UiRect searchBarBounds() const noexcept { return searchBarBounds_; }
    [[nodiscard]] UiRect updateAllBtnBounds() const noexcept { return updateAllBtnBounds_; }
    [[nodiscard]] UiRect addSourceBtnBounds() const noexcept { return addSourceBtnBounds_; }
    [[nodiscard]] const std::vector<SourceCardUi>& sourceCards() const noexcept { return sourceActionBounds_; }

#ifdef _WIN32
    /// Spawns a native Win32 desktop application window.
    static int runStandaloneWindow(int width = 1100, int height = 750);
#endif

private:
    ManagerTab activeTab_{ManagerTab::Browse};
    PackageCategory activeCategory_{PackageCategory::All};
    std::string searchQuery_;
    std::string statusMessage_{"Ready"};
    std::filesystem::path inspectedFile_;
    SideloadInspectResult inspectResult_;
    std::function<void()> onAddSource_;

    std::vector<PackageManifest> filteredPackages_;
    std::vector<PackageManifest> pendingUpdates_;

    int winWidth_{1100};
    int winHeight_{750};
    int scrollY_{0};
    int maxScrollY_{0};

    UiRect tabBrowseBounds_;
    UiRect tabLibraryBounds_;
    UiRect tabSideloadBounds_;
    UiRect tabSettingsBounds_;
    UiRect searchBarBounds_;
    UiRect updateAllBtnBounds_;
    UiRect addSourceBtnBounds_;
    std::vector<std::pair<UiRect, std::string>> cardActionBounds_;
    std::vector<SourceCardUi> sourceActionBounds_;
};

} // namespace micamgr

#endif // MICAMGR_UI_HPP

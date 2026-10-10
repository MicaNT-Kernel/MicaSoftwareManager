#include <micamgr/ui.hpp>
#include <micamgr/manager.hpp>
#include <algorithm>
#include <iostream>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace micamgr {

SoftwareManagerUI::SoftwareManagerUI() {
    updateFilteredPackages();
    updateLayout(winWidth_, winHeight_);
}

void SoftwareManagerUI::setActiveTab(ManagerTab tab) noexcept {
    activeTab_ = tab;
    updateFilteredPackages();
    updateLayout(winWidth_, winHeight_);
}

void SoftwareManagerUI::setCategory(PackageCategory cat) noexcept {
    activeCategory_ = cat;
    updateFilteredPackages();
    updateLayout(winWidth_, winHeight_);
}

void SoftwareManagerUI::setSearchQuery(std::string_view query) {
    searchQuery_ = query;
    updateFilteredPackages();
    updateLayout(winWidth_, winHeight_);
}

void SoftwareManagerUI::updateFilteredPackages() {
    auto& mgr = MicaManager::instance();
    pendingUpdates_ = mgr.catalog().filterByCategory(PackageCategory::Updates);

    if (activeTab_ == ManagerTab::Browse) {
        if (!searchQuery_.empty()) {
            filteredPackages_ = mgr.catalog().search(searchQuery_);
        } else {
            filteredPackages_ = mgr.catalog().filterByCategory(activeCategory_);
        }
    } else if (activeTab_ == ManagerTab::Library) {
        filteredPackages_ = pendingUpdates_;
        if (filteredPackages_.empty()) {
            for (const auto& p : mgr.catalog().allPackages()) {
                if (p.state == InstallState::Installed) {
                    filteredPackages_.push_back(p);
                }
            }
        }
    } else {
        filteredPackages_.clear();
    }
}

void SoftwareManagerUI::refresh(bool forceScan) {
    auto& mgr = MicaManager::instance();
    if (forceScan) {
        mgr.refreshInstalledApps();
    }
    updateFilteredPackages();
    updateLayout(winWidth_, winHeight_);
}

void SoftwareManagerUI::updateLayout(int width, int height) {
    winWidth_ = width;
    winHeight_ = height;

    int tabY = 48;
    int tabH = 36;
    tabBrowseBounds_ = UiRect{ 24, tabY, 110, tabH };
    tabLibraryBounds_ = UiRect{ 144, tabY, 150, tabH };
    tabSideloadBounds_ = UiRect{ 304, tabY, 160, tabH };
    tabSettingsBounds_ = UiRect{ 474, tabY, 110, tabH };

    searchBarBounds_ = UiRect{ width - 320, tabY + 2, 290, 32 };
    updateAllBtnBounds_ = UiRect{ width - 180, 100, 150, 32 };

    // Layout cards
    cardActionBounds_.clear();
    int cardY = 145;
    int cardH = 75;

    for (size_t i = 0; i < filteredPackages_.size(); ++i) {
        int cy = cardY + static_cast<int>(i) * (cardH + 10);
        if (cy + cardH > height - 40) break;
        UiRect actionBtn{ width - 150, cy + 22, 100, 32 };
        cardActionBounds_.push_back({ actionBtn, filteredPackages_[i].id });
    }
}

void SoftwareManagerUI::setInspectFilePath(const std::filesystem::path& path) {
    inspectedFile_ = path;
    inspectResult_ = SideloadInspector::inspectFile(path);
}

bool SoftwareManagerUI::onMouseDown(int x, int y) {
    if (tabBrowseBounds_.contains(x, y)) {
        setActiveTab(ManagerTab::Browse);
        return true;
    }
    if (tabLibraryBounds_.contains(x, y)) {
        setActiveTab(ManagerTab::Library);
        return true;
    }
    if (tabSideloadBounds_.contains(x, y)) {
        setActiveTab(ManagerTab::Sideload);
        return true;
    }
    if (tabSettingsBounds_.contains(x, y)) {
        setActiveTab(ManagerTab::Settings);
        return true;
    }

    if (activeTab_ == ManagerTab::Library && updateAllBtnBounds_.contains(x, y)) {
        statusMessage_ = "Updating all pending packages...";
        auto reports = MicaManager::instance().upgradeAll();
        statusMessage_ = "Updated " + std::to_string(reports.size()) + " packages.";
        updateFilteredPackages();
        updateLayout(winWidth_, winHeight_);
        return true;
    }

    for (const auto& [btnRect, pkgId] : cardActionBounds_) {
        if (btnRect.contains(x, y)) {
            std::string log;
            MicaManager::instance().installPackage(pkgId, log);
            statusMessage_ = "Installed " + pkgId;
            updateFilteredPackages();
            updateLayout(winWidth_, winHeight_);
            return true;
        }
    }

    return false;
}

void SoftwareManagerUI::onSearchInput(std::string_view text) {
    setSearchQuery(text);
}

#ifdef _WIN32
static SoftwareManagerUI* g_ui = nullptr;
static HFONT g_titleFont = nullptr;
static HFONT g_subFont = nullptr;
static HFONT g_cardTitleFont = nullptr;
static HPEN g_linePen = nullptr;

static void initGdiResources() {
    if (!g_titleFont) {
        g_titleFont = CreateFontW(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    }
    if (!g_subFont) {
        g_subFont = CreateFontW(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    }
    if (!g_cardTitleFont) {
        g_cardTitleFont = CreateFontW(17, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    }
    if (!g_linePen) {
        g_linePen = CreatePen(PS_SOLID, 1, RGB(35, 45, 65));
    }
}

static void freeGdiResources() {
    if (g_titleFont) { DeleteObject(g_titleFont); g_titleFont = nullptr; }
    if (g_subFont) { DeleteObject(g_subFont); g_subFont = nullptr; }
    if (g_cardTitleFont) { DeleteObject(g_cardTitleFont); g_cardTitleFont = nullptr; }
    if (g_linePen) { DeleteObject(g_linePen); g_linePen = nullptr; }
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_ERASEBKGND:
            return 1; // Prevent white background redraw flicker; WM_PAINT double buffers everything

        case WM_USER + 101: {
            // Asynchronous inventory scan completed in background
            if (g_ui) {
                g_ui->updateFilteredPackages();
                RECT rc;
                GetClientRect(hWnd, &rc);
                g_ui->updateLayout(rc.right - rc.left, rc.bottom - rc.top);
                InvalidateRect(hWnd, nullptr, FALSE);
            }
            return 0;
        }

        case WM_SIZE: {
            if (g_ui) {
                int w = LOWORD(lParam);
                int h = HIWORD(lParam);
                g_ui->updateLayout(w, h);
                InvalidateRect(hWnd, nullptr, FALSE);
            }
            return 0;
        }
        case WM_LBUTTONDOWN: {
            if (g_ui) {
                int x = LOWORD(lParam);
                int y = HIWORD(lParam);
                if (g_ui->onMouseDown(x, y)) {
                    InvalidateRect(hWnd, nullptr, FALSE);
                }
            }
            return 0;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            RECT rc;
            GetClientRect(hWnd, &rc);
            int width = rc.right - rc.left;
            int height = rc.bottom - rc.top;

            // Double buffered rendering
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBmp = CreateCompatibleBitmap(hdc, width, height);
            HBITMAP oldBmp = static_cast<HBITMAP>(SelectObject(memDC, memBmp));

            // Background: Dark Mica Slate (#0B0F19)
            HBRUSH bgBrush = CreateSolidBrush(RGB(11, 15, 25));
            FillRect(memDC, &rc, bgBrush);
            DeleteObject(bgBrush);

            // Title Bar
            SetBkMode(memDC, TRANSPARENT);
            SetTextColor(memDC, RGB(245, 248, 255));
            HFONT oldFont = static_cast<HFONT>(SelectObject(memDC, g_titleFont));

            TextOutW(memDC, 24, 12, L"Mica Software Manager", 21);

            SelectObject(memDC, g_subFont);
            SetTextColor(memDC, RGB(120, 140, 170));
            TextOutW(memDC, 275, 18, L"Sovereign Desktop Package Center (MicaNT & Windows NT)", 54);

            // Tab bar underline
            HPEN oldPen = static_cast<HPEN>(SelectObject(memDC, g_linePen));
            MoveToEx(memDC, 0, 88, nullptr);
            LineTo(memDC, width, 88);

            // Draw Tabs
            auto drawTab = [&](const UiRect& r, const wchar_t* title, bool active) {
                HBRUSH tBrush = CreateSolidBrush(active ? RGB(32, 45, 75) : RGB(18, 25, 40));
                RECT tr{ r.x, r.y, r.x + r.width, r.y + r.height };
                FillRect(memDC, &tr, tBrush);
                DeleteObject(tBrush);

                if (active) {
                    HBRUSH indBrush = CreateSolidBrush(RGB(65, 140, 255));
                    RECT ir{ r.x, r.y + r.height - 3, r.x + r.width, r.y + r.height };
                    FillRect(memDC, &ir, indBrush);
                    DeleteObject(indBrush);
                }

                SetTextColor(memDC, active ? RGB(255, 255, 255) : RGB(160, 175, 200));
                DrawTextW(memDC, title, -1, &tr, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            };

            drawTab(g_ui->tabBrowseBounds(), L"Browse Hub", g_ui->activeTab() == ManagerTab::Browse);
            drawTab(g_ui->tabLibraryBounds(), L"Library & Updates", g_ui->activeTab() == ManagerTab::Library);
            drawTab(g_ui->tabSideloadBounds(), L"Sideload Inspector", g_ui->activeTab() == ManagerTab::Sideload);
            drawTab(g_ui->tabSettingsBounds(), L"Sources", g_ui->activeTab() == ManagerTab::Settings);

            // Search bar preview
            RECT sr{ g_ui->searchBarBounds().x, g_ui->searchBarBounds().y, g_ui->searchBarBounds().x + g_ui->searchBarBounds().width, g_ui->searchBarBounds().y + g_ui->searchBarBounds().height };
            HBRUSH sBrush = CreateSolidBrush(RGB(20, 28, 45));
            FillRect(memDC, &sr, sBrush);
            DeleteObject(sBrush);
            SetTextColor(memDC, RGB(110, 130, 160));
            DrawTextW(memDC, L"  Search software...", -1, &sr, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

            // Content Area
            if (g_ui->activeTab() == ManagerTab::Browse || g_ui->activeTab() == ManagerTab::Library) {
                int startY = 105;
                if (g_ui->activeTab() == ManagerTab::Library) {
                    SetTextColor(memDC, RGB(220, 230, 245));
                    TextOutW(memDC, 24, startY, L"Pending Updates & Installed Software Inventory", 46);

                    // Update All Button
                    RECT ur{ g_ui->updateAllBtnBounds().x, g_ui->updateAllBtnBounds().y, g_ui->updateAllBtnBounds().x + g_ui->updateAllBtnBounds().width, g_ui->updateAllBtnBounds().y + g_ui->updateAllBtnBounds().height };
                    HBRUSH uBrush = CreateSolidBrush(RGB(35, 120, 230));
                    FillRect(memDC, &ur, uBrush);
                    DeleteObject(uBrush);
                    SetTextColor(memDC, RGB(255, 255, 255));
                    DrawTextW(memDC, L"Update All", -1, &ur, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                }

                int cardY = 145;
                int cardH = 75;
                for (const auto& pkg : g_ui->filteredPackages()) {
                    if (cardY + cardH > height - 40) break;
                    RECT cr{ 24, cardY, width - 24, cardY + cardH };
                    HBRUSH cBrush = CreateSolidBrush(RGB(17, 24, 38));
                    FillRect(memDC, &cr, cBrush);
                    DeleteObject(cBrush);

                    // Package title (using cached font)
                    SelectObject(memDC, g_cardTitleFont);
                    SetTextColor(memDC, RGB(240, 245, 255));

                    std::wstring wname(pkg.name.begin(), pkg.name.end());
                    TextOutW(memDC, 36, cardY + 12, wname.c_str(), static_cast<int>(wname.size()));

                    SelectObject(memDC, g_subFont);
                    SetTextColor(memDC, RGB(130, 150, 180));
                    std::string desc = pkg.vendor + " | v" + pkg.version + " | " + pkg.sourceToString() + " (" + pkg.architecture + ")";
                    std::wstring wdesc(desc.begin(), desc.end());
                    TextOutW(memDC, 36, cardY + 36, wdesc.c_str(), static_cast<int>(wdesc.size()));

                    // Action button
                    RECT br{ width - 150, cardY + 20, width - 40, cardY + 54 };
                    bool isUp = (pkg.state == InstallState::UpdateAvailable);
                    bool isInst = (pkg.state == InstallState::Installed);
                    HBRUSH bBrush = CreateSolidBrush(isUp ? RGB(210, 140, 20) : isInst ? RGB(30, 75, 50) : RGB(25, 95, 180));
                    FillRect(memDC, &br, bBrush);
                    DeleteObject(bBrush);

                    SetTextColor(memDC, RGB(255, 255, 255));
                    DrawTextW(memDC, isUp ? L"Update" : isInst ? L"Installed" : L"Get", -1, &br, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

                    cardY += cardH + 10;
                }
            } else if (g_ui->activeTab() == ManagerTab::Sideload) {
                SetTextColor(memDC, RGB(230, 240, 255));
                TextOutW(memDC, 24, 110, L"Sideload Inspector & Verification Sandbox", 42);
                SetTextColor(memDC, RGB(130, 150, 180));
                TextOutW(memDC, 24, 135, L"Drop or inspect any .exe, .msix, .apk, or portable .zip bundle for FIPS SHA-256 integrity.", 91);

                RECT dropBox{ 24, 170, width - 24, 380 };
                HBRUSH dBrush = CreateSolidBrush(RGB(15, 22, 35));
                FillRect(memDC, &dropBox, dBrush);
                DeleteObject(dBrush);
                DrawTextW(memDC, L"Drop Installer Package File Here to Inspect\n(Win32 PE / Android APK / MSIX / ZIP)", -1, &dropBox, DT_CENTER | DT_VCENTER);
            } else if (g_ui->activeTab() == ManagerTab::Settings) {
                SetTextColor(memDC, RGB(230, 240, 255));
                TextOutW(memDC, 24, 110, L"Repository Sources & Sovereign Configuration", 45);
                SetTextColor(memDC, RGB(130, 150, 180));
                TextOutW(memDC, 24, 135, L"- MicaNT Sovereign Repository (Barrer Software) [ACTIVE]", 56);
                TextOutW(memDC, 24, 155, L"- Windows Package Manager (winget-pkgs upstream) [ACTIVE]", 56);
                TextOutW(memDC, 24, 175, L"- Clean-Room AOSP Repository (MicaG TPM 2.0 Attestation) [ACTIVE]", 65);
            }

            // Status Bar at bottom
            RECT statusRc{ 0, height - 28, width, height };
            HBRUSH statBrush = CreateSolidBrush(RGB(16, 22, 34));
            FillRect(memDC, &statusRc, statBrush);
            DeleteObject(statBrush);

            SetTextColor(memDC, RGB(120, 140, 170));
            SelectObject(memDC, g_subFont);
            std::wstring wstat(g_ui->statusMessage().begin(), g_ui->statusMessage().end());
            TextOutW(memDC, 16, height - 22, wstat.c_str(), static_cast<int>(wstat.size()));

            BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

            SelectObject(memDC, oldFont);
            SelectObject(memDC, oldPen);
            SelectObject(memDC, oldBmp);
            DeleteObject(memBmp);
            DeleteDC(memDC);

            EndPaint(hWnd, &ps);
            return 0;
        }
        case WM_DESTROY: {
            freeGdiResources();
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

int SoftwareManagerUI::runStandaloneWindow(int width, int height) {
    HINSTANCE hInstance = GetModuleHandleW(nullptr);
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
    wc.lpszClassName = L"MicaSoftwareManagerWnd";

    RegisterClassExW(&wc);

    HWND hWnd = CreateWindowExW(
        0,
        L"MicaSoftwareManagerWnd",
        L"Mica Software Manager",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        nullptr, nullptr, hInstance, nullptr
    );

    if (!hWnd) return 1;

    initGdiResources();

    SoftwareManagerUI ui;
    g_ui = &ui;
    ui.updateLayout(width, height);
    ui.setStatusMessage("MicaNT Sovereign Engine | Ready (Scanning system inventory in background...)");

    // Launch background asynchronous inventory scan
    MicaManager::instance().startAsyncScan([hWnd](size_t foundApps, size_t updatesCount) {
        if (g_ui) {
            std::string msg = "System scan complete: " + std::to_string(foundApps) + " apps detected (" + std::to_string(updatesCount) + " updates available)";
            g_ui->setStatusMessage(msg);
            if (hWnd && IsWindow(hWnd)) {
                PostMessageW(hWnd, WM_USER + 101, 0, 0);
            }
        }
    });

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    g_ui = nullptr;
    return static_cast<int>(msg.wParam);
}
#endif

} // namespace micamgr

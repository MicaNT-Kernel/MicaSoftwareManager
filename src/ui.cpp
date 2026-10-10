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
    scrollY_ = 0;
    updateFilteredPackages();
    updateLayout(winWidth_, winHeight_);
}

void SoftwareManagerUI::setCategory(PackageCategory cat) noexcept {
    activeCategory_ = cat;
    scrollY_ = 0;
    updateFilteredPackages();
    updateLayout(winWidth_, winHeight_);
}

void SoftwareManagerUI::setSearchQuery(std::string_view query) {
    searchQuery_ = query;
    scrollY_ = 0;
    updateFilteredPackages();
    updateLayout(winWidth_, winHeight_);
}

void SoftwareManagerUI::scroll(int deltaY) noexcept {
    scrollY_ += deltaY;
    if (scrollY_ < 0) scrollY_ = 0;
    if (scrollY_ > maxScrollY_) scrollY_ = maxScrollY_;
}

void SoftwareManagerUI::updateFilteredPackages() {
    auto& mgr = MicaManager::instance();
    auto isSrcEnabled = [&](PackageSource src) { return mgr.isSourceEnabled(src); };
    pendingUpdates_ = mgr.catalog().filterByCategory(PackageCategory::Updates, isSrcEnabled);

    if (activeTab_ == ManagerTab::Browse) {
        if (!searchQuery_.empty()) {
            filteredPackages_ = mgr.catalog().search(searchQuery_, isSrcEnabled);
        } else {
            filteredPackages_ = mgr.catalog().filterByCategory(activeCategory_, isSrcEnabled);
        }
    } else if (activeTab_ == ManagerTab::Library) {
        filteredPackages_.clear();
        std::string lq;
        lq.reserve(searchQuery_.size());
        for (char c : searchQuery_) lq.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));

        // 1. Pending updates first
        for (const auto& u : pendingUpdates_) {
            if (lq.empty()) {
                filteredPackages_.push_back(u);
            } else {
                std::string ln, lid, lv;
                for (char c : u.name) ln.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
                for (char c : u.id) lid.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
                for (char c : u.vendor) lv.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
                if (ln.find(lq) != std::string::npos || lid.find(lq) != std::string::npos || lv.find(lq) != std::string::npos) {
                    filteredPackages_.push_back(u);
                }
            }
        }

        // 2. Real installed inventory from catalog
        for (const auto& p : mgr.catalog().allPackages()) {
            if (p.state == InstallState::Installed) {
                bool exists = false;
                for (const auto& existing : filteredPackages_) {
                    if (existing.id == p.id) {
                        exists = true;
                        break;
                    }
                }
                if (!exists) {
                    if (lq.empty()) {
                        filteredPackages_.push_back(p);
                    } else {
                        std::string ln, lid, lv;
                        for (char c : p.name) ln.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
                        for (char c : p.id) lid.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
                        for (char c : p.vendor) lv.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
                        if (ln.find(lq) != std::string::npos || lid.find(lq) != std::string::npos || lv.find(lq) != std::string::npos) {
                            filteredPackages_.push_back(p);
                        }
                    }
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
    addSourceBtnBounds_ = UiRect{ width - 180, 100, 150, 32 };

    cardActionBounds_.clear();
    sourceActionBounds_.clear();

    if (activeTab_ == ManagerTab::Browse || activeTab_ == ManagerTab::Library) {
        int cardY = 145;
        int cardH = 75;
        int totalH = cardY + static_cast<int>(filteredPackages_.size()) * (cardH + 10) + 40;
        maxScrollY_ = (totalH > height) ? (totalH - height) : 0;
        if (scrollY_ > maxScrollY_) scrollY_ = maxScrollY_;

        for (size_t i = 0; i < filteredPackages_.size(); ++i) {
            int cy = cardY + static_cast<int>(i) * (cardH + 10) - scrollY_;
            if (cy + cardH >= 90 && cy <= height - 30) {
                UiRect actionBtn{ width - 150, cy + 22, 100, 32 };
                cardActionBounds_.push_back({ actionBtn, filteredPackages_[i].id });
            }
        }
    } else if (activeTab_ == ManagerTab::Settings) {
        auto sources = MicaManager::instance().getSources();
        int srcCardY = 145;
        int srcCardH = 72;
        int totalH = srcCardY + static_cast<int>(sources.size()) * (srcCardH + 10) + 40;
        maxScrollY_ = (totalH > height) ? (totalH - height) : 0;
        if (scrollY_ > maxScrollY_) scrollY_ = maxScrollY_;

        for (size_t i = 0; i < sources.size(); ++i) {
            int cy = srcCardY + static_cast<int>(i) * (srcCardH + 10) - scrollY_;
            if (cy + srcCardH >= 90 && cy <= height - 30) {
                SourceCardUi sc;
                sc.sourceId = sources[i].id;
                sc.bounds = UiRect{ 24, cy, width - 48, srcCardH };
                sc.toggleBtnBounds = UiRect{ width - 210, cy + 20, 85, 32 };
                sc.deleteBtnBounds = UiRect{ width - 115, cy + 20, 80, 32 };
                sourceActionBounds_.push_back(sc);
            }
        }
    } else {
        maxScrollY_ = 0;
        scrollY_ = 0;
    }
}

void SoftwareManagerUI::setInspectFilePath(const std::filesystem::path& path) {
    inspectedFile_ = path;
    inspectResult_ = SideloadInspector::inspectFile(path);
}

bool SoftwareManagerUI::onMouseDown(int x, int y) {
    if (tabBrowseBounds_.contains(x, y)) {
        scrollY_ = 0;
        setActiveTab(ManagerTab::Browse);
        return true;
    }
    if (tabLibraryBounds_.contains(x, y)) {
        scrollY_ = 0;
        setActiveTab(ManagerTab::Library);
        return true;
    }
    if (tabSideloadBounds_.contains(x, y)) {
        scrollY_ = 0;
        setActiveTab(ManagerTab::Sideload);
        return true;
    }
    if (tabSettingsBounds_.contains(x, y)) {
        scrollY_ = 0;
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

    if (activeTab_ == ManagerTab::Settings) {
        if (addSourceBtnBounds_.contains(x, y)) {
            if (onAddSource_) {
                onAddSource_();
                return true;
            }
        }
        for (const auto& sc : sourceActionBounds_) {
            if (sc.toggleBtnBounds.contains(x, y)) {
                auto sources = MicaManager::instance().getSources();
                for (const auto& s : sources) {
                    if (s.id == sc.sourceId) {
                        std::string msg;
                        MicaManager::instance().setSourceEnabled(sc.sourceId, !s.isEnabled, msg);
                        statusMessage_ = msg;
                        updateFilteredPackages();
                        updateLayout(winWidth_, winHeight_);
                        return true;
                    }
                }
            }
            if (sc.deleteBtnBounds.contains(x, y)) {
                std::string msg;
                if (MicaManager::instance().removeSource(sc.sourceId, msg)) {
                    statusMessage_ = msg;
                } else {
                    statusMessage_ = msg;
                }
                updateFilteredPackages();
                updateLayout(winWidth_, winHeight_);
                return true;
            }
        }
    }

    for (const auto& [btnRect, pkgId] : cardActionBounds_) {
        if (btnRect.contains(x, y)) {
            std::string log;
            auto optPkg = MicaManager::instance().getPackageDetails(pkgId);
            if (optPkg && optPkg->state == InstallState::Installed && activeTab_ == ManagerTab::Library) {
                MicaManager::instance().uninstallPackage(pkgId, log);
                statusMessage_ = "Uninstalled " + pkgId;
            } else {
                MicaManager::instance().installPackage(pkgId, log);
                statusMessage_ = "Installed " + pkgId;
            }
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

static HWND g_hAddDlg = nullptr;
static HWND g_hIdEdit = nullptr;
static HWND g_hNameEdit = nullptr;
static HWND g_hUrlEdit = nullptr;
static HWND g_hTypeEdit = nullptr;

static LRESULT CALLBACK AddSourceWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HFONT hFont = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

            CreateWindowW(L"STATIC", L"Source Identifier (alphanumeric ID):", WS_CHILD | WS_VISIBLE, 20, 15, 400, 18, hWnd, nullptr, nullptr, nullptr);
            g_hIdEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"custom-feed", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 20, 35, 400, 24, hWnd, (HMENU)101, nullptr, nullptr);

            CreateWindowW(L"STATIC", L"Repository Display Name:", WS_CHILD | WS_VISIBLE, 20, 68, 400, 18, hWnd, nullptr, nullptr, nullptr);
            g_hNameEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"Community Sovereign Feed", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 20, 88, 400, 24, hWnd, (HMENU)102, nullptr, nullptr);

            CreateWindowW(L"STATIC", L"Repository URL or Local Directory Path:", WS_CHILD | WS_VISIBLE, 20, 121, 400, 18, hWnd, nullptr, nullptr, nullptr);
            g_hUrlEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"https://repo.micant.org/pkgs.json", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 20, 141, 400, 24, hWnd, (HMENU)103, nullptr, nullptr);

            CreateWindowW(L"STATIC", L"Feed Type (custom / sovereign / winget / aosp):", WS_CHILD | WS_VISIBLE, 20, 174, 400, 18, hWnd, nullptr, nullptr, nullptr);
            g_hTypeEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"custom", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 20, 194, 400, 24, hWnd, (HMENU)104, nullptr, nullptr);

            CreateWindowW(L"BUTTON", L"Add Source", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 200, 235, 105, 30, hWnd, (HMENU)1001, nullptr, nullptr);
            CreateWindowW(L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 315, 235, 105, 30, hWnd, (HMENU)1002, nullptr, nullptr);

            if (hFont) {
                SendMessageW(g_hIdEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
                SendMessageW(g_hNameEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
                SendMessageW(g_hUrlEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
                SendMessageW(g_hTypeEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
            }
            return 0;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1001) {
                wchar_t idBuf[128]{}, nameBuf[128]{}, urlBuf[512]{}, typeBuf[64]{};
                GetWindowTextW(g_hIdEdit, idBuf, 127);
                GetWindowTextW(g_hNameEdit, nameBuf, 127);
                GetWindowTextW(g_hUrlEdit, urlBuf, 511);
                GetWindowTextW(g_hTypeEdit, typeBuf, 63);

                std::wstring wid(idBuf), wname(nameBuf), wurl(urlBuf), wtype(typeBuf);
                std::string sid(wid.begin(), wid.end());
                std::string sname(wname.begin(), wname.end());
                std::string surl(wurl.begin(), wurl.end());
                std::string stype(wtype.begin(), wtype.end());

                if (!sid.empty() && !sname.empty() && !surl.empty()) {
                    RepositorySource src{
                        .id = sid,
                        .name = sname,
                        .urlOrPath = surl,
                        .type = stype.empty() ? "custom" : stype,
                        .isEnabled = true,
                        .isDefault = false
                    };
                    std::string msg;
                    MicaManager::instance().addSource(src, msg);
                    if (g_ui) {
                        g_ui->setStatusMessage(msg);
                        g_ui->updateFilteredPackages();
                        HWND hPar = GetParent(hWnd);
                        if (hPar && IsWindow(hPar)) {
                            RECT prc;
                            GetClientRect(hPar, &prc);
                            g_ui->updateLayout(prc.right - prc.left, prc.bottom - prc.top);
                            InvalidateRect(hPar, nullptr, FALSE);
                        }
                    }
                }
                DestroyWindow(hWnd);
                return 0;
            } else if (LOWORD(wParam) == 1002 || LOWORD(wParam) == IDCANCEL) {
                DestroyWindow(hWnd);
                return 0;
            }
            break;
        }
        case WM_CLOSE: {
            DestroyWindow(hWnd);
            return 0;
        }
        case WM_DESTROY: {
            HWND hPar = GetParent(hWnd);
            if (hPar && IsWindow(hPar)) EnableWindow(hPar, TRUE);
            g_hAddDlg = nullptr;
            return 0;
        }
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

static void openAddSourceDialog(HWND hParent) {
    if (g_hAddDlg && IsWindow(g_hAddDlg)) {
        SetForegroundWindow(g_hAddDlg);
        return;
    }
    HINSTANCE hInst = GetModuleHandleW(nullptr);
    WNDCLASSEXW dwc{};
    dwc.cbSize = sizeof(WNDCLASSEXW);
    dwc.lpfnWndProc = AddSourceWndProc;
    dwc.hInstance = hInst;
    dwc.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
    dwc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    dwc.lpszClassName = L"MicaAddSourceDialogWnd";
    RegisterClassExW(&dwc);

    RECT prc;
    GetWindowRect(hParent, &prc);
    int dlgW = 460;
    int dlgH = 320;
    int dx = prc.left + (prc.right - prc.left - dlgW) / 2;
    int dy = prc.top + (prc.bottom - prc.top - dlgH) / 2;

    EnableWindow(hParent, FALSE);
    g_hAddDlg = CreateWindowExW(
        WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
        L"MicaAddSourceDialogWnd",
        L"Add Repository Source - Mica Software Manager",
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        dx, dy, dlgW, dlgH,
        hParent, nullptr, hInst, nullptr
    );
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

        case WM_MOUSEWHEEL: {
            if (g_ui) {
                int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
                g_ui->scroll(-(zDelta / WHEEL_DELTA) * 60);
                RECT rc;
                GetClientRect(hWnd, &rc);
                g_ui->updateLayout(rc.right - rc.left, rc.bottom - rc.top);
                InvalidateRect(hWnd, nullptr, FALSE);
            }
            return 0;
        }

        case WM_KEYDOWN: {
            if (g_ui) {
                int step = 0;
                if (wParam == VK_UP) step = -40;
                else if (wParam == VK_DOWN) step = 40;
                else if (wParam == VK_PRIOR) step = -200;
                else if (wParam == VK_NEXT) step = 200;
                if (step != 0) {
                    g_ui->scroll(step);
                    RECT rc;
                    GetClientRect(hWnd, &rc);
                    g_ui->updateLayout(rc.right - rc.left, rc.bottom - rc.top);
                    InvalidateRect(hWnd, nullptr, FALSE);
                    return 0;
                }
            }
            break;
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
                    std::string libHeader = "Installed Software & Pending Updates (" + std::to_string(g_ui->filteredPackages().size()) + " items)";
                    std::wstring wLibHeader(libHeader.begin(), libHeader.end());
                    TextOutW(memDC, 24, startY, wLibHeader.c_str(), static_cast<int>(wLibHeader.size()));

                    // Update All Button
                    RECT ur{ g_ui->updateAllBtnBounds().x, g_ui->updateAllBtnBounds().y, g_ui->updateAllBtnBounds().x + g_ui->updateAllBtnBounds().width, g_ui->updateAllBtnBounds().y + g_ui->updateAllBtnBounds().height };
                    HBRUSH uBrush = CreateSolidBrush(RGB(35, 120, 230));
                    FillRect(memDC, &ur, uBrush);
                    DeleteObject(uBrush);
                    SetTextColor(memDC, RGB(255, 255, 255));
                    DrawTextW(memDC, L"Update All", -1, &ur, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                } else {
                    SetTextColor(memDC, RGB(220, 230, 245));
                    std::string browseHeader = "Browse Catalog (" + std::to_string(g_ui->filteredPackages().size()) + " packages available)";
                    std::wstring wBrowseHeader(browseHeader.begin(), browseHeader.end());
                    TextOutW(memDC, 24, startY, wBrowseHeader.c_str(), static_cast<int>(wBrowseHeader.size()));
                }

                // Clip rendering to card viewport (between y=140 and y=height-32)
                HRGN clipRgn = CreateRectRgn(0, 140, width, height - 32);
                SelectClipRgn(memDC, clipRgn);

                int cardY = 145;
                int cardH = 75;
                const auto& pkgs = g_ui->filteredPackages();
                for (size_t i = 0; i < pkgs.size(); ++i) {
                    const auto& pkg = pkgs[i];
                    int cy = cardY + static_cast<int>(i) * (cardH + 10) - g_ui->scrollY();
                    if (cy + cardH < 140 || cy > height - 32) continue;

                    RECT cr{ 24, cy, width - 24, cy + cardH };
                    HBRUSH cBrush = CreateSolidBrush(RGB(17, 24, 38));
                    FillRect(memDC, &cr, cBrush);
                    DeleteObject(cBrush);

                    // Package title (using cached font)
                    SelectObject(memDC, g_cardTitleFont);
                    SetTextColor(memDC, RGB(240, 245, 255));

                    std::wstring wname(pkg.name.begin(), pkg.name.end());
                    TextOutW(memDC, 36, cy + 12, wname.c_str(), static_cast<int>(wname.size()));

                    SelectObject(memDC, g_subFont);
                    SetTextColor(memDC, RGB(130, 150, 180));
                    std::string desc = pkg.vendor + " | v" + (pkg.installedVersion.empty() ? pkg.version : pkg.installedVersion) + " | " + pkg.sourceToString() + " (" + pkg.architecture + ")";
                    if (pkg.state == InstallState::UpdateAvailable) {
                        desc += " -> New v" + pkg.version + " available";
                    }
                    std::wstring wdesc(desc.begin(), desc.end());
                    TextOutW(memDC, 36, cy + 38, wdesc.c_str(), static_cast<int>(wdesc.size()));

                    // Action button
                    RECT br{ width - 150, cy + 20, width - 40, cy + 54 };
                    bool isUp = (pkg.state == InstallState::UpdateAvailable);
                    bool isInst = (pkg.state == InstallState::Installed);

                    HBRUSH bBrush = nullptr;
                    const wchar_t* btnText = L"Get";
                    if (isUp) {
                        bBrush = CreateSolidBrush(RGB(210, 140, 20)); // Orange
                        btnText = L"Update";
                    } else if (isInst) {
                        if (g_ui->activeTab() == ManagerTab::Library) {
                            bBrush = CreateSolidBrush(RGB(110, 40, 40)); // Reddish
                            btnText = L"Uninstall";
                        } else {
                            bBrush = CreateSolidBrush(RGB(30, 75, 50)); // Green
                            btnText = L"Installed";
                        }
                    } else {
                        bBrush = CreateSolidBrush(RGB(25, 95, 180)); // Blue
                        btnText = L"Get";
                    }
                    FillRect(memDC, &br, bBrush);
                    DeleteObject(bBrush);

                    SetTextColor(memDC, RGB(255, 255, 255));
                    DrawTextW(memDC, btnText, -1, &br, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                }

                SelectClipRgn(memDC, nullptr);
                DeleteObject(clipRgn);
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
                SelectObject(memDC, g_titleFont);
                TextOutW(memDC, 24, 102, L"Repository Sources & Sovereign Feeds", 36);

                // "+ Add Source" Button
                RECT ar{ g_ui->addSourceBtnBounds().x, g_ui->addSourceBtnBounds().y, g_ui->addSourceBtnBounds().x + g_ui->addSourceBtnBounds().width, g_ui->addSourceBtnBounds().y + g_ui->addSourceBtnBounds().height };
                HBRUSH aBrush = CreateSolidBrush(RGB(35, 120, 230));
                FillRect(memDC, &ar, aBrush);
                DeleteObject(aBrush);
                SetTextColor(memDC, RGB(255, 255, 255));
                SelectObject(memDC, g_subFont);
                DrawTextW(memDC, L"+ Add Source", -1, &ar, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

                // Clip rendering for source list
                HRGN clipRgn = CreateRectRgn(0, 140, width, height - 32);
                SelectClipRgn(memDC, clipRgn);

                auto sources = MicaManager::instance().getSources();
                int srcCardY = 145;
                int srcCardH = 72;
                for (size_t i = 0; i < sources.size(); ++i) {
                    const auto& s = sources[i];
                    int cy = srcCardY + static_cast<int>(i) * (srcCardH + 10) - g_ui->scrollY();
                    if (cy + srcCardH < 140 || cy > height - 32) continue;

                    RECT cr{ 24, cy, width - 24, cy + srcCardH };
                    HBRUSH cBrush = CreateSolidBrush(RGB(17, 24, 38));
                    FillRect(memDC, &cr, cBrush);
                    DeleteObject(cBrush);

                    // Source Name & ID
                    SelectObject(memDC, g_cardTitleFont);
                    SetTextColor(memDC, RGB(240, 245, 255));
                    std::string sTitle = s.name + " (" + s.id + ")";
                    std::wstring wsTitle(sTitle.begin(), sTitle.end());
                    TextOutW(memDC, 36, cy + 12, wsTitle.c_str(), static_cast<int>(wsTitle.size()));

                    // Details
                    SelectObject(memDC, g_subFont);
                    SetTextColor(memDC, RGB(130, 150, 180));
                    std::string sSub = s.urlOrPath + " | Type: " + s.type + (s.isDefault ? " [Default System Source]" : " [Custom Source]");
                    std::wstring wsSub(sSub.begin(), sSub.end());
                    TextOutW(memDC, 36, cy + 38, wsSub.c_str(), static_cast<int>(wsSub.size()));

                    // Status badge: [ENABLED] or [DISABLED]
                    RECT stBadge{ width - 320, cy + 20, width - 225, cy + 52 };
                    HBRUSH stbBrush = CreateSolidBrush(s.isEnabled ? RGB(25, 90, 50) : RGB(90, 35, 35));
                    FillRect(memDC, &stBadge, stbBrush);
                    DeleteObject(stbBrush);
                    SetTextColor(memDC, s.isEnabled ? RGB(140, 240, 170) : RGB(255, 160, 160));
                    DrawTextW(memDC, s.isEnabled ? L"ENABLED" : L"DISABLED", -1, &stBadge, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

                    // Toggle Button: Enable / Disable
                    RECT tr{ width - 210, cy + 20, width - 125, cy + 52 };
                    HBRUSH tBrush = CreateSolidBrush(s.isEnabled ? RGB(110, 45, 45) : RGB(30, 110, 60));
                    FillRect(memDC, &tr, tBrush);
                    DeleteObject(tBrush);
                    SetTextColor(memDC, RGB(255, 255, 255));
                    DrawTextW(memDC, s.isEnabled ? L"Disable" : L"Enable", -1, &tr, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

                    // Delete Button (Disabled for default built-in sources)
                    RECT dr{ width - 115, cy + 20, width - 35, cy + 52 };
                    if (s.isDefault) {
                        HBRUSH dBrush = CreateSolidBrush(RGB(35, 45, 60));
                        FillRect(memDC, &dr, dBrush);
                        DeleteObject(dBrush);
                        SetTextColor(memDC, RGB(90, 105, 125));
                        DrawTextW(memDC, L"Locked", -1, &dr, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                    } else {
                        HBRUSH dBrush = CreateSolidBrush(RGB(150, 35, 35));
                        FillRect(memDC, &dr, dBrush);
                        DeleteObject(dBrush);
                        SetTextColor(memDC, RGB(255, 255, 255));
                        DrawTextW(memDC, L"Delete", -1, &dr, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                    }
                }

                SelectClipRgn(memDC, nullptr);
                DeleteObject(clipRgn);
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
    ui.setOnAddSource([hWnd]() {
        openAddSourceDialog(hWnd);
    });
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

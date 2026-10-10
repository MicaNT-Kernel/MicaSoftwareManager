#include <micamgr/package.hpp>
#include <sstream>
#include <charconv>

namespace micamgr {

int PackageHelper::compareVersions(std::string_view v1, std::string_view v2) {
    size_t i = 0, j = 0;
    while (i < v1.size() || j < v2.size()) {
        int64_t n1 = 0, n2 = 0;
        while (i < v1.size() && v1[i] >= '0' && v1[i] <= '9') {
            n1 = n1 * 10 + (v1[i] - '0');
            ++i;
        }
        while (j < v2.size() && v2[j] >= '0' && v2[j] <= '9') {
            n2 = n2 * 10 + (v2[j] - '0');
            ++j;
        }
        if (n1 < n2) return -1;
        if (n1 > n2) return 1;

        if (i < v1.size() && (v1[i] == '.' || v1[i] == '-' || v1[i] == '+')) ++i;
        if (j < v2.size() && (v2[j] == '.' || v2[j] == '-' || v2[j] == '+')) ++j;
    }
    return 0;
}

static std::string escapeJson(std::string_view str) {
    std::string out;
    out.reserve(str.size() + 8);
    for (char c : str) {
        if (c == '"') out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else if (c == '\t') out += "\\t";
        else out += c;
    }
    return out;
}

static std::string extractField(std::string_view json, std::string_view field) {
    std::string needle = "\"" + std::string(field) + "\"";
    auto pos = json.find(needle);
    if (pos == std::string_view::npos) return {};
    pos = json.find(':', pos);
    if (pos == std::string_view::npos) return {};
    pos = json.find_first_not_of(" \t\r\n", pos + 1);
    if (pos == std::string_view::npos) return {};

    if (json[pos] == '"') {
        auto end = json.find('"', pos + 1);
        if (end == std::string_view::npos) return {};
        return std::string(json.substr(pos + 1, end - pos - 1));
    } else {
        auto end = json.find_first_of(",}\r\n", pos);
        if (end == std::string_view::npos) end = json.size();
        return std::string(json.substr(pos, end - pos));
    }
}

std::string PackageHelper::toJson(const PackageManifest& pkg) {
    std::ostringstream ss;
    ss << "    {\n";
    ss << "      \"id\": \"" << escapeJson(pkg.id) << "\",\n";
    ss << "      \"name\": \"" << escapeJson(pkg.name) << "\",\n";
    ss << "      \"version\": \"" << escapeJson(pkg.version) << "\",\n";
    ss << "      \"installed_version\": \"" << escapeJson(pkg.installedVersion) << "\",\n";
    ss << "      \"vendor\": \"" << escapeJson(pkg.vendor) << "\",\n";
    ss << "      \"license\": \"" << escapeJson(pkg.license) << "\",\n";
    ss << "      \"homepage\": \"" << escapeJson(pkg.homepage) << "\",\n";
    ss << "      \"download_url\": \"" << escapeJson(pkg.downloadUrl) << "\",\n";
    ss << "      \"sha256\": \"" << escapeJson(pkg.sha256) << "\",\n";
    ss << "      \"architecture\": \"" << escapeJson(pkg.architecture) << "\",\n";
    ss << "      \"category\": \"" << pkg.categoryToString() << "\",\n";
    ss << "      \"source\": \"" << pkg.sourceToString() << "\",\n";
    ss << "      \"description\": \"" << escapeJson(pkg.description) << "\",\n";
    ss << "      \"icon_glyph\": \"" << escapeJson(pkg.iconGlyph) << "\",\n";
    ss << "      \"icon_id\": \"" << escapeJson(pkg.iconId) << "\",\n";
    ss << "      \"state\": " << static_cast<int>(pkg.state) << "\n";
    ss << "    }";
    return ss.str();
}

std::optional<PackageManifest> PackageHelper::fromJson(std::string_view json) {
    PackageManifest pkg;
    pkg.id = extractField(json, "id");
    if (pkg.id.empty()) return std::nullopt;

    pkg.name = extractField(json, "name");
    pkg.version = extractField(json, "version");
    pkg.installedVersion = extractField(json, "installed_version");
    pkg.vendor = extractField(json, "vendor");
    pkg.license = extractField(json, "license");
    pkg.homepage = extractField(json, "homepage");
    pkg.downloadUrl = extractField(json, "download_url");
    pkg.sha256 = extractField(json, "sha256");
    pkg.architecture = extractField(json, "architecture");
    pkg.description = extractField(json, "description");
    pkg.iconGlyph = extractField(json, "icon_glyph");
    pkg.iconId = extractField(json, "icon_id");

    std::string catStr = extractField(json, "category");
    if (catStr == "Featured") pkg.category = PackageCategory::Featured;
    else if (catStr == "Productivity") pkg.category = PackageCategory::Productivity;
    else if (catStr == "Developer") pkg.category = PackageCategory::Developer;
    else if (catStr == "Media") pkg.category = PackageCategory::Media;
    else if (catStr == "Utilities") pkg.category = PackageCategory::Utilities;
    else if (catStr == "Games") pkg.category = PackageCategory::Games;
    else if (catStr == "Android & WSA") pkg.category = PackageCategory::AndroidWsa;
    else if (catStr == "System & Runtime") pkg.category = PackageCategory::System;
    else pkg.category = PackageCategory::Utilities;

    std::string srcStr = extractField(json, "source");
    if (srcStr == "WinGet") pkg.source = PackageSource::WinGet;
    else if (srcStr.find("Android") != std::string::npos) pkg.source = PackageSource::AospWsa;
    else if (srcStr.find("MicaNT") != std::string::npos) pkg.source = PackageSource::MicaNative;
    else if (srcStr == "Portable") pkg.source = PackageSource::PortableZip;
    else pkg.source = PackageSource::Unknown;

    std::string stateStr = extractField(json, "state");
    if (!stateStr.empty()) {
        try {
            pkg.state = static_cast<InstallState>(std::stoi(stateStr));
        } catch (...) {}
    }

    return pkg;
}

std::string PackageHelper::toManifestListJson(const std::vector<PackageManifest>& packages) {
    std::ostringstream ss;
    ss << "{\n";
    ss << "  \"hub\": \"Mica Software Manager\",\n";
    ss << "  \"version\": \"1.0.0\",\n";
    ss << "  \"count\": " << packages.size() << ",\n";
    ss << "  \"packages\": [\n";
    for (size_t i = 0; i < packages.size(); ++i) {
        ss << toJson(packages[i]);
        if (i + 1 < packages.size()) ss << ",";
        ss << "\n";
    }
    ss << "  ]\n";
    ss << "}\n";
    return ss.str();
}

std::vector<PackageManifest> PackageHelper::fromManifestListJson(std::string_view json) {
    std::vector<PackageManifest> list;
    auto pos = json.find("\"packages\":");
    if (pos == std::string_view::npos) return list;
    pos = json.find('[', pos);
    if (pos == std::string_view::npos) return list;

    while (pos < json.size()) {
        auto objStart = json.find('{', pos);
        if (objStart == std::string_view::npos) break;
        auto objEnd = json.find('}', objStart);
        if (objEnd == std::string_view::npos) break;

        std::string_view block = json.substr(objStart, objEnd - objStart + 1);
        auto optPkg = fromJson(block);
        if (optPkg) {
            list.push_back(std::move(*optPkg));
        }
        pos = objEnd + 1;
    }
    return list;
}

} // namespace micamgr

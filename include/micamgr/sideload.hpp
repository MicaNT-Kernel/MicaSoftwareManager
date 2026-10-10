#ifndef MICAMGR_SIDELOAD_HPP
#define MICAMGR_SIDELOAD_HPP

#include <micamgr/types.hpp>
#include <string>
#include <filesystem>

namespace micamgr {

class SideloadInspector {
public:
    /// Computes the SHA-256 hash of a file on disk.
    static std::string computeSha256(const std::filesystem::path& filePath);

    /// Inspects and classifies a local installer file (.exe, .msix, .apk, .zip).
    static SideloadInspectResult inspectFile(const std::filesystem::path& filePath);

    /// Executes the sideload installation for an inspected file.
    static bool executeSideload(const std::filesystem::path& filePath, const SideloadInspectResult& inspection, std::string& outLog);
};

} // namespace micamgr

#endif // MICAMGR_SIDELOAD_HPP

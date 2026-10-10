# Mica Software Manager (`micamgr`)

[![Standard: ISO C++23](https://img.shields.io/badge/Standard-ISO%20C%2B%2B23-blue.svg)](https://isocpp.org/)
[![Platform: MicaNT / Windows NT](https://img.shields.io/badge/Platform-MicaNT%20%7C%20Windows%20NT-0078D6.svg)](https://github.com/MicaNT-Kernel)
[![Zero Telemetry](https://img.shields.io/badge/Telemetry-0%25%20Zero-success.svg)](https://github.com/MicaNT-Kernel)
[![Clean-Room Provenance](https://img.shields.io/badge/Clean--Room-Certified-green.svg)](https://github.com/MicaNT-Kernel)

**Mica Software Manager** (`micamgr`) is the official sovereign software management hub and package center for the **MicaNT** operating system and modern **Windows NT**. 

Unlike commercial app stores that push sponsored advertisements, track user telemetry, and enforce proprietary account logins, Mica Software Manager is engineered from first principles in **pure ISO C++23** to provide a fast, private, and universal software delivery ecosystem.

---

## Key Pillars

1. **Zero Telemetry & Zero Ads**:
   - No Microsoft Account or Barrer Account required.
   - Zero telemetry tracking, analytics telemetry pings, or background ad beacons.
   - Curated, clean, and community-audited software.

2. **Unified Tri-Subsystem Package Architecture**:
   - **MicaNT Sovereign Suite**: Native clean-room tools including [SurShell](https://github.com/MicaNT-Kernel/SurShell), [MicaG](https://github.com/MicaNT-Kernel/MicaG), Sentinel, Haven AI, and PrismX.
   - **Win32 NT Ecosystem**: Seamless non-interactive integration with upstream `winget-pkgs` (over 5,000 certified Win32 apps).
   - **Clean-Room Android AOSP (WSA)**: Native Android package staging verified by [MicaG](https://github.com/MicaNT-Kernel/MicaG) hardware TPM 2.0 attestation and FIPS 180-4 SHA-256 integrity.

3. **Installed Software & Updates Engine**:
   - Scans system installation registries (`HKLM` and `HKCU` Uninstall records).
   - Automatically cross-references versions against upstream manifests.
   - Non-interactive, 1-click **"Update All"** batch execution.

4. **Sideload Inspector & Verification Sandbox**:
   - Drag-and-drop inspector for `.exe`, `.msix`, `.apk`, and `.zip` installer bundles.
   - Validates PE/ZIP headers, target architecture (`x64`, `ARM64`, `universal`), and computes cryptographic SHA-256 digests before installation.

---

## Dual Interface: CLI & Sovereign GUI

### Command-Line Interface (`micamgr.exe`)
```bash
# Search for software across all sources
micamgr search firefox

# List installed software discovered on the system
micamgr list --installed

# Check for updates and view version diffs
micamgr list --updates

# 1-Click upgrade all outdated software packages
micamgr update --all

# Install a package silently
micamgr install VideoLAN.VLC

# Inspect local installer bundle
micamgr inspect ./installer.exe
```

### Sovereign Desktop GUI
Launch without arguments or with `micamgr gui` to experience the double-buffered dark Mica desktop application with tabbed navigation:
- **Browse Hub**: Categorized showcase of sovereign, Win32, and Android software.
- **Library & Updates**: Real-time inventory of installed apps with 1-click batch updating.
- **Sideload Inspector**: Verification sandbox for local installer packages.
- **Sources**: Configurable repository feeds.

---

## SurShell Integration

Mica Software Manager integrates directly with [**SurShell**](https://github.com/MicaNT-Kernel/SurShell):
- Registered as a first-class citizen in the SurShell Start Menu and App Hub.
- Accessible via global shortcut `Win+I` / App Hub or `micamgr`.
- Can run standalone or dock seamlessly within SurShell virtual workspaces.

---

## Building from Source

```powershell
# Clone the repository
git clone https://github.com/MicaNT-Kernel/MicaSoftwareManager.git
cd MicaSoftwareManager

# Configure with CMake and Ninja
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

# Build (strictly -j2 concurrency recommended)
cmake --build build --config Release -j2

# Run verification test suite
.\build\bin\micamgr_tests.exe

# Launch Mica Software Manager
.\build\bin\micamgr.exe
```

---

## Clean-Room Provenance & License

Mica Software Manager is developed by **Barrer Software** as free and open-source software under the [MIT License](LICENSE). All interfaces, package serialization algorithms, and registry scanner routines are clean-room implementations created without proprietary dependencies.

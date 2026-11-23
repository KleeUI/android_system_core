// Device-specific Recovery fastboot preservation policy for the Cupid test package.
#pragma once

#include <array>
#include <string_view>

namespace fastboot_preserve {

enum class SnapshotState {
    kClear,
    kActive,
    kUnknown,
};

enum class Mode {
    kLegacy,
    kPreserve,
    kMisconfiguredCupid,
};

constexpr Mode GetMode(std::string_view product_device, std::string_view preserve_property) {
    if (product_device != "cupid") return Mode::kLegacy;
    return preserve_property == "true" ? Mode::kPreserve : Mode::kMisconfiguredCupid;
}

constexpr bool CanInitializeBootControl(Mode mode) {
    // Service activation can initialize misc before any command is handled.
    // Cupid's root Recovery writer does not need the boot-control HAL.
    return mode == Mode::kLegacy;
}

constexpr std::array<std::string_view, 12> kAllowedAFlashTargets = {
        "boot_a",         "vendor_boot_a", "recovery_a",     "dtbo_a",
        "vbmeta_a",       "vbmeta_system_a", "system_a",       "vendor_a",
        "system_ext_a",   "product_a",     "odm_a",          "vendor_dlkm_a",
};

constexpr std::array<std::string_view, 6> kLogicalAFlashTargets = {
        "system_a", "vendor_a", "system_ext_a", "product_a", "odm_a", "vendor_dlkm_a",
};

constexpr bool IsAllowedAFlashTarget(std::string_view partition, std::string_view slot_suffix) {
    if (slot_suffix != "_a") return false;
    for (const auto candidate : kAllowedAFlashTargets) {
        if (partition == candidate) return true;
    }
    return false;
}

constexpr bool IsLogicalAFlashTarget(std::string_view partition) {
    for (const auto candidate : kLogicalAFlashTargets) {
        if (partition == candidate) return true;
    }
    return false;
}

constexpr bool CanFlash(std::string_view partition, std::string_view slot_suffix,
                        SnapshotState snapshot) {
    return snapshot == SnapshotState::kClear && IsAllowedAFlashTarget(partition, slot_suffix);
}

}  // namespace fastboot_preserve

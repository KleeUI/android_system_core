#include "fastboot_preserve_policy.h"

#include <gtest/gtest.h>

TEST(FastbootPreservePolicyTest, CupidRequiresThePreservationProperty) {
    EXPECT_EQ(fastboot_preserve::GetMode("cupid", "true"), fastboot_preserve::Mode::kPreserve);
    EXPECT_EQ(fastboot_preserve::GetMode("cupid", "false"),
              fastboot_preserve::Mode::kMisconfiguredCupid);
    EXPECT_EQ(fastboot_preserve::GetMode("cupid", ""),
              fastboot_preserve::Mode::kMisconfiguredCupid);
    for (const auto property : {"1", "TRUE", " true", "true ", "false", "unknown"}) {
        EXPECT_EQ(fastboot_preserve::GetMode("cupid", property),
                  fastboot_preserve::Mode::kMisconfiguredCupid);
    }
    EXPECT_EQ(fastboot_preserve::GetMode("other", ""), fastboot_preserve::Mode::kLegacy);
    EXPECT_EQ(fastboot_preserve::GetMode("other", "true"), fastboot_preserve::Mode::kLegacy);
}

TEST(FastbootPreservePolicyTest, AllowsOnlyManifestBoundAFlashTargetsWhenSnapshotIsClear) {
    for (const auto target : fastboot_preserve::kAllowedAFlashTargets) {
        EXPECT_TRUE(fastboot_preserve::CanFlash(target, "_a", fastboot_preserve::SnapshotState::kClear));
    }
}

TEST(FastbootPreservePolicyTest, CupidNeverActivatesTheBootControlHal) {
    for (const auto property : {"true", "false", "", "1", "TRUE"}) {
        EXPECT_FALSE(fastboot_preserve::CanInitializeBootControl(
                fastboot_preserve::GetMode("cupid", property)));
    }
    EXPECT_TRUE(fastboot_preserve::CanInitializeBootControl(
            fastboot_preserve::GetMode("other", "")));
    EXPECT_TRUE(fastboot_preserve::CanInitializeBootControl(
            fastboot_preserve::GetMode("other", "true")));
}

TEST(FastbootPreservePolicyTest, RejectsOtherSlotsAndProtectedPartitions) {
    EXPECT_FALSE(fastboot_preserve::CanFlash("system_b", "_a", fastboot_preserve::SnapshotState::kClear));
    EXPECT_FALSE(fastboot_preserve::CanFlash("system_a", "_b", fastboot_preserve::SnapshotState::kClear));
    EXPECT_FALSE(fastboot_preserve::CanFlash("userdata", "_a", fastboot_preserve::SnapshotState::kClear));
    EXPECT_FALSE(fastboot_preserve::CanFlash("metadata", "_a", fastboot_preserve::SnapshotState::kClear));
    EXPECT_FALSE(fastboot_preserve::CanFlash("super", "_a", fastboot_preserve::SnapshotState::kClear));
    EXPECT_FALSE(fastboot_preserve::CanFlash("misc", "_a", fastboot_preserve::SnapshotState::kClear));
}

TEST(FastbootPreservePolicyTest, RefusesActiveOrUnknownSnapshotState) {
    // The omitted Recovery boot HAL leaves state unknown. Neither raw nor
    // logical targets may turn that absence into an assumed clear state.
    for (const auto target : fastboot_preserve::kAllowedAFlashTargets) {
        EXPECT_FALSE(fastboot_preserve::CanFlash(target, "_a",
                                                fastboot_preserve::SnapshotState::kActive));
        EXPECT_FALSE(fastboot_preserve::CanFlash(target, "_a",
                                                fastboot_preserve::SnapshotState::kUnknown));
    }
}

TEST(FastbootPreservePolicyTest, RefusesAliasesUnsuffixedAndProtectedTargets) {
    for (const auto target : {"", "boot", "system", "system_b", "boot_b", "super", "super_a",
                              "userdata", "userdata_a", "metadata", "misc", "persist", "qmcs",
                              "../system_a", "/dev/block/by-name/system_a", "system_a/"}) {
        EXPECT_FALSE(fastboot_preserve::CanFlash(target, "_a",
                                                fastboot_preserve::SnapshotState::kClear));
    }
    for (const auto suffix : {"", "a", "_b", "_a "}) {
        for (const auto target : fastboot_preserve::kAllowedAFlashTargets) {
            EXPECT_FALSE(fastboot_preserve::CanFlash(target, suffix,
                                                    fastboot_preserve::SnapshotState::kClear));
        }
    }
}

TEST(FastbootPreservePolicyTest, LogicalTargetClassificationMatchesThePackagePlan) {
    for (const auto target : fastboot_preserve::kLogicalAFlashTargets) {
        EXPECT_TRUE(fastboot_preserve::IsLogicalAFlashTarget(target));
    }
    for (const auto target : fastboot_preserve::kAllowedAFlashTargets) {
        if (target == "system_a" || target == "vendor_a" || target == "system_ext_a" ||
            target == "product_a" || target == "odm_a" || target == "vendor_dlkm_a") {
            continue;
        }
        EXPECT_FALSE(fastboot_preserve::IsLogicalAFlashTarget(target));
    }
}

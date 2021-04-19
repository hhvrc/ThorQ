#include "ovrsettings.h"

#include <fmt/core.h>

bool ThorQ::VR::OVRSettings::AllowGlobalActionSetPriority(bool allow)
{
    vr::EVRSettingsError settingsError;
    vr::VRSettings()->SetBool(vr::k_pch_SteamVR_Section, vr::k_pch_SteamVR_AllowGlobalActionSetPriority, allow, &settingsError);

    if (settingsError != vr::VRSettingsError_None) {
        fmt::print(stderr, "Failed to set global actionset priority: {}\n", vr::VRSettings()->GetSettingsErrorNameFromEnum(settingsError));
        return false;
    }

    return true;
}

#include "ovraction.h"

#include "ovrtrackeddevice.h"

#include <fmt/core.h>

ThorQ::VR::OVRAction::OVRAction(const char* actionName)
    : m_handle(vr::k_ulInvalidActionHandle)
{
    vr::EVRInputError error = vr::VRInput()->GetActionHandle(actionName, &m_handle);

    if (error != vr::VRInputError_None) {
        m_handle = vr::k_ulInvalidActionHandle;
        fmt::print(stderr, "Failed to get action handle: {}\n", error);
    }
}

bool ThorQ::VR::OVRAction::triggerHapticFeedback(float secondsFromNow, float amplitude, float frequency, float duration, OVRInputSource restrictToDevice)
{
    vr::EVRInputError error = vr::VRInput()->TriggerHapticVibrationAction(m_handle, secondsFromNow, duration, frequency, amplitude, restrictToDevice.handle());

    if (error != vr::VRInputError_None) {
        fmt::print(stderr, "Error triggering haptic vibration: {}\n", error);
        return false;
    }

    return false;
}

bool ThorQ::VR::OVRAction::getDigitalData(vr::InputDigitalActionData_t& data, OVRInputSource restrictToDevice)
{
    vr::EVRInputError error = vr::VRInput()->GetDigitalActionData(m_handle, &data, sizeof(vr::InputDigitalActionData_t), restrictToDevice.handle());

    if (error == vr::VRInputError_NoData) {
        fmt::print(stderr, "No Data\n");
        return false;
    }
    else if (error != vr::VRInputError_None) {
        fmt::print(stderr, "Error getting digital action data: {}\n", error);
        return false;
    }

    return false;
}

bool ThorQ::VR::OVRAction::getAnalogData(vr::InputAnalogActionData_t& data, OVRInputSource restrictToDevice)
{
    vr::EVRInputError error = vr::VRInput()->GetAnalogActionData(m_handle, &data, sizeof(vr::InputAnalogActionData_t), restrictToDevice.handle());

    if (error == vr::VRInputError_NoData) {
        fmt::print(stderr, "No Data\n");
        return false;
    }
    else if (error != vr::VRInputError_None) {
        fmt::print(stderr, "Error getting analog action data: {}\n", error);
        return false;
    }

    return false;
}

#include "ovrinputvalue.h"

#include <fmt/core.h>

ThorQ::VR::OVRInputSource::OVRInputSource(const char* inputSourcePath)
    : m_handle(vr::k_ulInvalidInputValueHandle)
{
    if (inputSourcePath != nullptr) {
        vr::EVRInputError error = vr::VRInput()->GetInputSourceHandle(inputSourcePath, &m_handle);
        if (error != vr::VRInputError_None) {
            fmt::print(stderr, "Failed to get input source handle: {}\n", error);
            m_handle = vr::k_ulInvalidInputValueHandle;
        }
    }
}

ThorQ::VR::OVRTrackedDevice ThorQ::VR::OVRInputSource::trackedDevice() const
{
    vr::EVRInputError err;
    vr::InputOriginInfo_t sourceInfo;

    err = vr::VRInput()->GetOriginTrackedDeviceInfo(m_handle, &sourceInfo, sizeof(vr::InputOriginInfo_t));

    if (err != vr::VRInputError_None)
    {
        fmt::print(stderr, "Error getting device for input source: {}\n", err);
        return vr::TrackedControllerRole_Invalid;
    }

    return sourceInfo.trackedDeviceIndex;
}

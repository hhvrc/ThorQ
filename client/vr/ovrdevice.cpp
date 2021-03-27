#include "ovrdevice.h"

#include <fmt/core.h>

ThorQ::VR::OVRDevice::OVRDevice(const char* devicePath)
    : m_handle(vr::k_ulInvalidInputValueHandle)
{
    if (devicePath != nullptr) {
        vr::EVRInputError error = vr::VRInput()->GetInputSourceHandle(devicePath, &m_handle);
        if (error != vr::VRInputError_None) {
            fmt::print(stderr, "Failed to get action handle: {}\n", error);
            m_handle = vr::k_ulInvalidInputValueHandle;
        }
    }
}

ThorQ::VR::OVRDevice::OVRDevice(vr::VRInputValueHandle_t deviceHandle)
    : m_handle(deviceHandle)
{
}

ThorQ::VR::OVRDevice::~OVRDevice()
{
}

vr::VRInputValueHandle_t ThorQ::VR::OVRDevice::handle() const noexcept
{
    return m_handle;
}

bool ThorQ::VR::OVRDevice::isValid() const noexcept
{
    return m_handle != vr::k_ulInvalidInputValueHandle;
}

bool ThorQ::VR::OVRDevice::operator==(const ThorQ::VR::OVRDevice& other) const noexcept
{
    return m_handle == other.m_handle;
}

bool ThorQ::VR::OVRDevice::operator!=(const ThorQ::VR::OVRDevice &other) const noexcept
{
    return m_handle != other.m_handle;
}

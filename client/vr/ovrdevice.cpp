#include "ovrdevice.h"

ThorQ::VR::OVRDevice::OVRDevice(const char *devicePath)
    : m_handle(vr::k_ulInvalidInputValueHandle)
{
    if (vr::VRInput()->GetInputSourceHandle(devicePath, &m_handle) != vr::VRInputError_None) {
        m_handle = vr::k_ulInvalidInputValueHandle;
    }
}

ThorQ::VR::OVRDevice::~OVRDevice()
{
}

bool ThorQ::VR::OVRDevice::isValid() const
{
    return m_handle != vr::k_ulInvalidInputValueHandle;
}

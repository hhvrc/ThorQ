#ifndef OVRTRACKEDDEVICE_H
#define OVRTRACKEDDEVICE_H

#include "openvr.h"

namespace ThorQ::VR {
struct OVRTrackedDevice
{
public:
    static inline OVRTrackedDevice Invalid()
    {
        return OVRTrackedDevice(vr::k_unTrackedDeviceIndexInvalid);
    };

    OVRTrackedDevice(vr::ETrackedControllerRole controllerRole);
    OVRTrackedDevice(vr::TrackedDeviceIndex_t deviceHandle) : m_handle(deviceHandle) {}
    ~OVRTrackedDevice() {}

    inline vr::TrackedDeviceIndex_t handle() const noexcept
    {
        return m_handle;
    }

    inline bool isValid() const noexcept
    {
        return m_handle != vr::k_unTrackedDeviceIndexInvalid;
    }

    vr::ETrackedControllerRole controllerRole() const;

    inline bool operator==(const OVRTrackedDevice& other) const noexcept
    {
        return m_handle == other.m_handle;
    }

    inline bool operator!=(const OVRTrackedDevice& other) const noexcept
    {
        return m_handle != other.m_handle;
    }
private:
    vr::TrackedDeviceIndex_t m_handle;
};
}

#endif // OVRDEVICE_H

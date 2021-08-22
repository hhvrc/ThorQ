#ifndef OVRINPUTSOURCE_H
#define OVRINPUTSOURCE_H

#include "openvr.h"
#include "ovrtrackeddevice.h"

namespace ThorQ::VR {
struct OVRInputSource
{
public:
    static inline OVRInputSource Invalid()
    {
        return OVRInputSource(vr::k_ulInvalidInputValueHandle);
    };

    OVRInputSource(const char* inputSourcePath);
    OVRInputSource(vr::VRInputValueHandle_t inputSourceHandle) : m_handle(inputSourceHandle) {}
    ~OVRInputSource() {}

    inline vr::VRInputValueHandle_t handle() const noexcept
    {
        return m_handle;
    }

    inline bool isValid() const noexcept
    {
        return m_handle != vr::k_ulInvalidInputValueHandle;
    }

    ThorQ::VR::OVRTrackedDevice trackedDevice() const;

    inline bool operator==(const OVRInputSource& other) const noexcept
    {
        return m_handle == other.m_handle;
    }

    inline bool operator!=(const OVRInputSource& other) const noexcept
    {
        return m_handle != other.m_handle;
    }
private:
    vr::VRInputValueHandle_t m_handle;
};
}

#endif // OVRINPUTSOURCE_H

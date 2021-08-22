#ifndef OVRACTION_H
#define OVRACTION_H

#include "openvr.h"
#include "ovrinputvalue.h"

namespace ThorQ::VR {
struct OVRAction
{
public:
    static inline OVRAction Invalid()
    {
        return OVRAction(vr::k_ulInvalidActionHandle);
    };

    OVRAction(const char* actionName);
    OVRAction(vr::VRActionHandle_t actionHandle) : m_handle(actionHandle) {}
    ~OVRAction() {}

    inline vr::VRActionHandle_t handle() const noexcept
    {
        return m_handle;
    }

    inline bool isValid() const noexcept
    {
        return m_handle != vr::k_ulInvalidActionHandle;
    }

    bool triggerHapticFeedback(float secondsFromNow, float amplitude, float frequency, float duration, OVRInputSource restrictToDevice = OVRInputSource::Invalid());

    bool getDigitalData(vr::InputDigitalActionData_t& data, OVRInputSource restrictToDevice = OVRInputSource::Invalid());
    bool getAnalogData(vr::InputAnalogActionData_t& data, OVRInputSource restrictToDevice = OVRInputSource::Invalid());

    inline bool operator==(const OVRAction& other) const noexcept
    {
        return m_handle == other.m_handle;
    }

    inline bool operator!=(const OVRAction& other) const noexcept
    {
        return m_handle != other.m_handle;
    }
private:
    vr::VRActionHandle_t m_handle;
};
}

#endif // OVRACTION_H

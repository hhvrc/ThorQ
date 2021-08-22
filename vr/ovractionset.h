#ifndef OVRACTIONSET_H
#define OVRACTIONSET_H

#include "openvr.h"

namespace ThorQ::VR {
struct OVRAction;
struct OVRActionSet
{
public:
    static inline OVRActionSet Invalid()
    {
        return OVRActionSet(vr::k_ulInvalidActionSetHandle);
    };

    OVRActionSet(const char* actionSetName);
    OVRActionSet(vr::VRActionSetHandle_t actionSetHandle) : m_handle(actionSetHandle) {}
    ~OVRActionSet() {}

    inline vr::VRActionSetHandle_t handle() const noexcept
    {
        return m_handle;
    }

    inline bool isValid() const noexcept
    {
        return m_handle != vr::k_ulInvalidActionSetHandle;
    }

    bool openBindingUI(const ThorQ::VR::OVRAction& action);

    inline bool operator==(const OVRActionSet& other) const noexcept
    {
        return m_handle == other.m_handle;
    }

    inline bool operator!=(const OVRActionSet& other) const noexcept
    {
        return m_handle != other.m_handle;
    }
private:
    vr::VRActionSetHandle_t m_handle;
};
}

#endif // OVRACTIONSET_H

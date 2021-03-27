#include "ovractionset.h"

#include "ovraction.h"

#include <fmt/core.h>

ThorQ::VR::OVRActionSet::OVRActionSet(const char* actionSetName)
    : m_handle(vr::k_ulInvalidActionSetHandle)
{
    vr::VRInput()->GetActionSetHandle(actionSetName, &m_handle);
}

ThorQ::VR::OVRActionSet::~OVRActionSet()
{

}

vr::VRActionSetHandle_t ThorQ::VR::OVRActionSet::handle() const
{
    return m_handle;
}

bool ThorQ::VR::OVRActionSet::isValid() const
{
    return m_handle != vr::k_ulInvalidActionSetHandle;
}

bool ThorQ::VR::OVRActionSet::openBindingUI(const ThorQ::VR::OVRAction& action)
{
    vr::EVRInputError error = vr::VRInput()->ShowActionOrigins(m_handle, action.handle());

    if (error != vr::VRInputError_None) {
        fmt::print(stderr, "Error triggering haptic vibration: {}\n", error);
        return false;
    }

    return true;
}

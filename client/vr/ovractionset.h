#ifndef OVRACTIONSET_H
#define OVRACTIONSET_H

#include "openvr.h"

namespace ThorQ {
namespace VR {
class OVRAction;
class OVRActionSet
{
public:
    OVRActionSet(const char* actionSetName);
    ~OVRActionSet();

    vr::VRActionSetHandle_t handle() const;
    bool isValid() const;

    bool openBindingUI(const ThorQ::VR::OVRAction& action);
private:
    vr::VRActionSetHandle_t m_handle;
};
}
}

#endif // OVRACTIONSET_H

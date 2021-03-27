#ifndef OVRACTION_H
#define OVRACTION_H

#include "openvr.h"

namespace ThorQ {
namespace VR {
class OVRDevice;
class OVRAction
{
public:
    OVRAction(const char* actionName);
    ~OVRAction();

    vr::VRActionHandle_t handle() const;
    bool isValid() const;

    bool triggerHapticFeedback(float secondsFromNow, float amplitude, float frequency, float duration, vr::VRInputValueHandle_t restrictToDeviceHandle = vr::k_ulInvalidInputValueHandle);

    bool getDigitalData(vr::InputDigitalActionData_t& data, vr::VRInputValueHandle_t restrictToDeviceHandle = vr::k_ulInvalidInputValueHandle);
    bool getAnalogData(vr::InputAnalogActionData_t& data, vr::VRInputValueHandle_t restrictToDeviceHandle = vr::k_ulInvalidInputValueHandle);
private:
    vr::VRActionHandle_t m_handle;
};
}
}

#endif // OVRACTION_H

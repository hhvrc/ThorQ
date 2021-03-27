#ifndef OVRDEVICE_H
#define OVRDEVICE_H

#include "openvr.h"

namespace ThorQ {
namespace VR {
class OVRDevice
{
public:
    OVRDevice(const char* devicePath);
    OVRDevice(vr::VRInputValueHandle_t deviceHandle);
    ~OVRDevice();

    vr::VRInputValueHandle_t handle() const noexcept;
    bool isValid() const noexcept;

    bool operator==(const OVRDevice& other) const noexcept;
    bool operator!=(const OVRDevice& other) const noexcept;
private:
    vr::VRInputValueHandle_t m_handle;
};
}
}

#endif // OVRDEVICE_H

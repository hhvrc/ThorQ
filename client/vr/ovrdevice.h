#ifndef OVRDEVICE_H
#define OVRDEVICE_H

#include <QObject>

#include "openvr.h"

namespace ThorQ {
namespace VR {
class OVRDevice
{
public:
    OVRDevice(const char* devicePath);
    ~OVRDevice();

    bool isValid() const;



    vr::VRInputValueHandle_t handle() const { return m_handle; }
private:
    vr::VRInputValueHandle_t m_handle;
};
}
}

#endif // OVRDEVICE_H

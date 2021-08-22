#include "ovrtrackeddevice.h"

ThorQ::VR::OVRTrackedDevice::OVRTrackedDevice(vr::ETrackedControllerRole controllerRole)
{
    m_handle = vr::VRSystem()->GetTrackedDeviceIndexForControllerRole(controllerRole);
}

vr::ETrackedControllerRole ThorQ::VR::OVRTrackedDevice::controllerRole() const
{
    return vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(m_handle);
}

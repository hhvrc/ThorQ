#include "ovraction.h"

ThorQ::VR::OVRAction::OVRAction(const char* actionName, QObject *parent)
    : QObject(parent)
    , m_handle(vr::k_ulInvalidActionHandle)
{
    if (vr::VRInput()->GetActionHandle(actionName, &m_handle) != vr::VRInputError_None) {
        m_handle = vr::k_ulInvalidActionHandle;
    }
}

ThorQ::VR::OVRAction::~OVRAction() {
}

bool ThorQ::VR::OVRAction::isValid() const
{
    return m_handle != vr::k_ulInvalidActionHandle;
}

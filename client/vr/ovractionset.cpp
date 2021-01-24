#include "ovractionset.h"

ThorQ::VR::OVRActionSet::OVRActionSet(const char* actionSetName, QObject* parent)
    : QObject(parent)
    , m_handle(vr::k_ulInvalidActionSetHandle)
{
    vr::VRInput()->GetActionSetHandle(actionSetName, &m_handle);
}

ThorQ::VR::OVRActionSet::~OVRActionSet()
{

}

#include "ovractionset.h"

ThorQ::VR::OVRActionSet::OVRActionSet(QObject* parent)
    : QObject(parent)
    , m_handle(vr::k_ulInvalidActionSetHandle)
{

}

ThorQ::VR::OVRActionSet::~OVRActionSet()
{

}

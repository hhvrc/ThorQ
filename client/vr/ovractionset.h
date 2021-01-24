#ifndef OVRACTIONSET_H
#define OVRACTIONSET_H

#include <QObject>

#include "openvr.h"

namespace ThorQ {
namespace VR {
class OVRActionSet : public QObject
{
    Q_OBJECT
public:
    OVRActionSet(const char* actionSetName, QObject* parent);
    ~OVRActionSet();

    vr::VRActionSetHandle_t handle() const { return m_handle; }
private:
    vr::VRActionSetHandle_t m_handle;
};
}
}

#endif // OVRACTIONSET_H

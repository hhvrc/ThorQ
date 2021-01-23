#ifndef OVRACTION_H
#define OVRACTION_H

#include <QObject>

#include "openvr.h"

namespace ThorQ {
namespace VR {
class OVRAction : public QObject
{
    Q_OBJECT
public:
    OVRAction(const char* actionName, QObject* parent);
    ~OVRAction();

    bool isValid() const;

    bool getDigitalData();
    bool getAnalogData();
private:
    vr::VRActionHandle_t m_handle;
};
}
}

#endif // OVRACTION_H

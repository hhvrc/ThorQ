#ifndef OVROVERLAY_H
#define OVROVERLAY_H

#include <QMatrix4x4>

#include "openvr.h"

namespace ThorQ {
namespace VR {
class OVRDevice;
class OVROverlay
{
public:
    OVROverlay(const char* overlayKey, const char* overlayName);
    ~OVROverlay();

    vr::VROverlayHandle_t handle() const noexcept;
    bool isValid() const noexcept;

    void destroy();

    bool setVisibility(bool visible);
    bool setWidth(float widthInMeters);
    bool setAlpha(float alpha);
    bool setTint(float r, float g, float b);

    enum class InputMethod {
        None  = vr::VROverlayInputMethod::VROverlayInputMethod_None,
        Mouse = vr::VROverlayInputMethod::VROverlayInputMethod_Mouse
    };
    bool setInputMethod(OVROverlay::InputMethod inputMethod);

    bool setTexture(unsigned int glTextureId);

    bool setDeviceRelativeTransform(const ThorQ::VR::OVRDevice& device, const QMatrix4x4& offset);

    bool pollEvent(vr::VREvent_t& event);
private:
    vr::VROverlayHandle_t m_handle;
};
}
}

#endif // OVROVERLAY_H

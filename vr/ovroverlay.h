#ifndef OVROVERLAY_H
#define OVROVERLAY_H

#include <QMatrix4x4>

#include "openvr.h"

namespace ThorQ::VR {
struct OVRTrackedDevice;
struct OVROverlay
{
public:
    static inline OVROverlay Invalid()
    {
        return OVROverlay(vr::k_ulOverlayHandleInvalid);
    };

    OVROverlay(const char* overlayKey, const char* overlayName);
    OVROverlay(vr::VROverlayHandle_t overlayHandle) : m_handle(overlayHandle) {}
    ~OVROverlay();

    inline vr::VROverlayHandle_t handle() const noexcept
    {
        return m_handle;
    }

    inline bool isValid() const noexcept
    {
        return m_handle != vr::k_ulOverlayHandleInvalid;
    }

    void destroy();

    bool setVisibility(bool visible);
    bool setWidth(float widthInMeters);
    bool setAlpha(float alpha);
    bool setTint(float r, float g, float b);
    bool setCurvature(float curvature);

    enum class InputMethod {
        None  = vr::VROverlayInputMethod::VROverlayInputMethod_None,
        Mouse = vr::VROverlayInputMethod::VROverlayInputMethod_Mouse
    };
    bool setInputMethod(OVROverlay::InputMethod inputMethod);

    bool setTexture(unsigned int glTextureId);

    bool setDeviceRelativeTransform(const ThorQ::VR::OVRTrackedDevice& device, const QMatrix4x4& offset);

    bool pollEvent(vr::VREvent_t& event);

    inline bool operator==(const OVROverlay& other) const noexcept
    {
        return m_handle == other.m_handle;
    }

    inline bool operator!=(const OVROverlay& other) const noexcept
    {
        return m_handle != other.m_handle;
    }
private:
    vr::VROverlayHandle_t m_handle;
};
}

#endif // OVROVERLAY_H

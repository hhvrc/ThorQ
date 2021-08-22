#include "ovroverlay.h"

#include "ovrhelpers.h"
#include "ovrtrackeddevice.h"

#include <fmt/core.h>

ThorQ::VR::OVROverlay::OVROverlay(const char* overlayKey, const char* overlayName)
    : m_handle(vr::k_ulOverlayHandleInvalid)
{
    fmt::print("Finding overlay\n");
    vr::EVROverlayError error = vr::VROverlay()->FindOverlay(overlayKey, &m_handle);
    if (error != vr::VROverlayError_None) {
        m_handle = vr::k_ulOverlayHandleInvalid;

        if (error == vr::VROverlayError_UnknownOverlay) {
            fmt::print("Creating overlay\n");
            error = vr::VROverlay()->CreateOverlay(overlayKey, overlayName, &m_handle);

            if (error != vr::VROverlayError_None) {
                fmt::print(stderr, "Error creating overlay: {}\n", vr::VROverlay()->GetOverlayErrorNameFromEnum(error));
                m_handle = vr::k_ulOverlayHandleInvalid;
            }
        }
        else {
            fmt::print(stderr, "Error finding overlay: {}\n", vr::VROverlay()->GetOverlayErrorNameFromEnum(error));
        }
    }
}

ThorQ::VR::OVROverlay::~OVROverlay()
{
    destroy();
}

void ThorQ::VR::OVROverlay::destroy()
{
    if (isValid()) {
        vr::VROverlay()->DestroyOverlay(m_handle);
    }
}

bool ThorQ::VR::OVROverlay::setVisibility(bool visible)
{
    if (!isValid()) {
        return false;
    }

    vr::EVROverlayError error = vr::VROverlayError_None;

    if (visible) {
        fmt::print("Show overlay\n");

        error = vr::VROverlay()->ShowOverlay(m_handle);
        if (error != vr::VROverlayError_None) {
            fmt::print(stderr, "Error showing overlay: {}\n", vr::VROverlay()->GetOverlayErrorNameFromEnum(error));
            return false;
        }
    }
    else {
        fmt::print("Hide overlay\n");

        error = vr::VROverlay()->HideOverlay(m_handle);
        if (error != vr::VROverlayError_None) {
            fmt::print(stderr, "Error hiding overlay: {}\n", vr::VROverlay()->GetOverlayErrorNameFromEnum(error));
            return false;
        }
    }

    return true;
}

bool ThorQ::VR::OVROverlay::setWidth(float widthInMeters)
{
    if (!isValid()) {
        return false;
    }

    vr::EVROverlayError error = vr::VROverlay()->SetOverlayWidthInMeters(m_handle, widthInMeters);

    if (error != vr::VROverlayError_None) {
        fmt::print(stderr, "Error setting overlay width: {}\n", vr::VROverlay()->GetOverlayErrorNameFromEnum(error));
        return false;
    }

    return true;
}

bool ThorQ::VR::OVROverlay::setAlpha(float alpha)
{
    if (!isValid()) {
        return false;
    }

    vr::EVROverlayError error = vr::VROverlay()->SetOverlayAlpha(m_handle, alpha);

    if (error != vr::VROverlayError_None) {
        fmt::print(stderr, "Error setting overlay alpha: {}\n", vr::VROverlay()->GetOverlayErrorNameFromEnum(error));
        return false;
    }

    return true;
}

bool ThorQ::VR::OVROverlay::setTint(float r, float g, float b)
{
    if (!isValid()) {
        return false;
    }

    vr::EVROverlayError error = vr::VROverlay()->SetOverlayColor(m_handle, r, g, b);

    if (error != vr::VROverlayError_None) {
        fmt::print(stderr, "Error setting overlay tint: {}\n", vr::VROverlay()->GetOverlayErrorNameFromEnum(error));
        return false;
    }

    return true;
}

bool ThorQ::VR::OVROverlay::setCurvature(float curvature)
{
    if (!isValid()) {
        return false;
    }

    vr::EVROverlayError error = vr::VROverlay()->SetOverlayCurvature(m_handle, curvature);

    if (error != vr::VROverlayError_None) {
        fmt::print(stderr, "Error setting overlay curvature: {}\n", vr::VROverlay()->GetOverlayErrorNameFromEnum(error));
        return false;
    }

    return true;
}

bool ThorQ::VR::OVROverlay::setInputMethod(ThorQ::VR::OVROverlay::InputMethod inputMethod)
{
    if (!isValid()) {
        return false;
    }

    vr::EVROverlayError error = vr::VROverlay()->SetOverlayInputMethod(m_handle, (vr::VROverlayInputMethod)inputMethod);

    if (error != vr::VROverlayError_None) {
        fmt::print(stderr, "Error setting overlay input method: {}\n", vr::VROverlay()->GetOverlayErrorNameFromEnum(error));
        return false;
    }

    return true;
}

bool ThorQ::VR::OVROverlay::setTexture(unsigned int glTextureId)
{
    if (!isValid()) {
        return false;
    }

    if (glTextureId != 0) {
        vr::Texture_t texture;

        texture.handle = (void*)(std::uintptr_t)glTextureId;
        texture.eType = vr::TextureType_OpenGL;
        texture.eColorSpace = vr::ColorSpace_Auto;

        // Give framebuffer id to OpenVR
        vr::EVROverlayError err = vr::VROverlay()->SetOverlayTexture(m_handle, &texture);
        if (err == vr::VROverlayError_None) {
            return true;
        }

        fmt::print(stderr, "Error setting overlay texture: {}\n", vr::VROverlay()->GetOverlayErrorNameFromEnum(err));
    }

    return false;
}

bool ThorQ::VR::OVROverlay::setDeviceRelativeTransform(const ThorQ::VR::OVRTrackedDevice& device, const QMatrix4x4& offset)
{
    if (!isValid()) {
        return false;
    }

    vr::HmdMatrix34_t vrMat;
    ToHmdMatrix34(offset, vrMat);

    // Position
    vr::EVROverlayError error = vr::VROverlay()->SetOverlayTransformTrackedDeviceRelative(m_handle, device.handle(), &vrMat);

    if (error != vr::VROverlayError_None) {
        fmt::print(stderr, "Error transforming overlay: {}\n", vr::VROverlay()->GetOverlayErrorNameFromEnum(error));
        return false;
    }

    return true;
}

bool ThorQ::VR::OVROverlay::pollEvent(vr::VREvent_t& event)
{
    return isValid() && vr::VROverlay()->PollNextOverlayEvent(m_handle, &event, sizeof(vr::VREvent_t));
}

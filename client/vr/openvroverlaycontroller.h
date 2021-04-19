#ifndef OPENVROVERLAYCONTROLLER_H
#define OPENVROVERLAYCONTROLLER_H

#include "openvr.h"

#include <QtCore/QtCore>
// because of incompatibilities with QtOpenGL and GLEW we need to cherry pick includes
#include <QtGui/QVector2D>
#include <QtGui/QMatrix4x4>
#include <QtCore/QVector>
#include <QtGui/QVector2D>
#include <QtGui/QVector3D>
#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLFramebufferObject>
#include <QtWidgets/QGraphicsScene>
#include <QtGui/QOffscreenSurface>

#include "constants.h"

#define OPENVR_APPLICATION_NAME THORQ_APPLICATION_NAME
#define OPENVR_APPLICATION_KEY THORQ_ORGANIZATION_NAME "." THORQ_APPLICATION_NAME

#include "ovrtrackeddevice.h"
#include "ovrinputvalue.h"
#include "ovroverlay.h"
#include "ovraction.h"
#include "ovractionset.h"
#include "ovrsettings.h"

namespace ThorQ::VR {
bool IsSteamVRInstalled();
bool IsSteamVRRunning();
bool IsHmdPresent();

bool Initialize();
void Shutdown();

bool IsManifestInstalled();
bool InstallManifest();
bool RemoveManifest();

/**
 * @brief The OpenVROverlayController class
 */
class OpenVROverlayController : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(OpenVROverlayController)
public:
public:
	OpenVROverlayController(QObject* parent = nullptr);
	~OpenVROverlayController() override;

    bool isValid() const;

	QWidget* widget() const;

    bool isVisible() const;
    float width() const;
    float alpha() const;
    QColor tint() const;
    float curvature() const;
signals:
	void vrQuit();

	void widgetChanged(QWidget* widget);

    void isVisibleChanged(bool visible);
    void widthChanged(float width);
    void alphaChanged(float alpha);
    void tintChanged(const QColor& color);
    void curvatureChanged(float curvature);
public slots:
	bool setWidget(QWidget* widget);

    void setIsVisible(bool isVisible);
    void setWidth(float width);
    void setAlpha(float alpha);
    void setTint(const QColor& color);
    void setCurvature(float curvature);

    bool openBindingUI();
protected:
    void halt();

	void update();

    bool pullEvents();

    void onSceneChanged();

    void setOverlayDevice(ThorQ::VR::OVRTrackedDevice device);

    const QMatrix4x4& getOffsetForControllerRole(vr::ETrackedControllerRole role) const;
    const QMatrix4x4& getOffsetForTrackedDevice(ThorQ::VR::OVRTrackedDevice device) const;
    const QMatrix4x4& getOffsetForInputSource(ThorQ::VR::OVRInputSource source) const;
private:
	bool m_isVisible;
	float m_alpha;
	float m_width;
    float m_curvature;
    QColor m_tint;

	// Widget
	QGraphicsProxyWidget* m_proxyWidget;
    QTimer* m_updateLogicTimer;

    // Overlay stuff
    ThorQ::VR::OVROverlay m_overlay;
    QMatrix4x4 m_overlayOffset;
    QMatrix4x4 m_overlayDeviceOffsetL;
    QMatrix4x4 m_overlayDeviceOffsetR;
    QMatrix4x4 m_overlayDeviceOffsetC;

    // Controller stuff
    ThorQ::VR::OVRTrackedDevice m_mouseDevice;
    ThorQ::VR::OVRTrackedDevice m_overlayDevice;
    vr::ETrackedControllerRole m_mouseControllerRole;
    vr::ETrackedControllerRole m_overlayControllerRole;

    // Action set stuff
    vr::VRActiveActionSet_t m_activeActionSet;
    ThorQ::VR::OVRActionSet m_actionSet;
    ThorQ::VR::OVRAction    m_actionHaptics;
    ThorQ::VR::OVRAction    m_actionInteract;
    ThorQ::VR::OVRAction    m_actionShowOverlay;
    ThorQ::VR::OVRAction    m_actionProxSensor;

    ThorQ::VR::OVRInputSource m_hmd;
    ThorQ::VR::OVRInputSource m_controllerLeft;
    ThorQ::VR::OVRInputSource m_controllerRight;

    // Graphics
    QGraphicsScene* m_scene;
    QOpenGLContext* m_glContext;
    QOffscreenSurface* m_surface;
    QOpenGLFramebufferObject* m_frameBuffer;

    // Input handling
    QPointF m_lastMousePoint;
    Qt::MouseButtons m_lastMouseButtons;
};
}

#endif // OPENVROVERLAYCONTROLLER_H

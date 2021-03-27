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

#include "ovrdevice.h"
#include "ovroverlay.h"
#include "ovraction.h"
#include "ovractionset.h"

namespace ThorQ {
namespace VR {
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

    enum class EHand : std::int8_t
    {
        Invalid = -1,
        Left,
        Right,
        Center
    };

	/**
	 * @brief widget
	 * @return
	 */
	QWidget* widget() const;

	/**
	 * @brief isVisible
	 * @return
	 */
	bool isVisible() const;

	/**
	 * @brief width
	 * @return
	 */
	float width() const;

	/**
	 * @brief alpha
	 * @return
	 */
	float alpha() const;

	/**
	 * @brief tint
	 * @return
	 */
    QColor tint() const;
signals:
    /**
	 * @brief vrQuit
     */
	void vrQuit();

	/**
	 * @brief widgetChanged
	 * @param widget
	 */
	void widgetChanged(QWidget* widget);

	/**
	 * @brief isVisibleChanged
	 * @param visible
	 */
    void isVisibleChanged(bool visible);

	/**
	 * @brief widthChanged
	 * @param width
	 */
	void widthChanged(float width);

	/**
	 * @brief alphaChanged
	 * @param alpha
	 */
	void alphaChanged(float alpha);

	/**
	 * @brief tintChanged
	 * @param color
	 */
    void tintChanged(const QColor& color);
public slots:
    /**
	 * @brief setWidget
     * @param widget
     */
	bool setWidget(QWidget* widget);

	/**
	 * @brief setIsVisible
	 * @param isVisible
	 */
    void setIsVisible(bool isVisible);

    /**
	 * @brief setWidth
     * @param width
     */
	void setWidth(float width);

    /**
	 * @brief setAlpha
     * @param alpha
     */
	void setAlpha(float alpha);

    /**
	 * @brief setTint
     * @param color
     */
    void setTint(const QColor& color);

    bool openBindingUI();
protected:
    void halt();

	void update();

    bool pullEvents();

    void onSceneChanged();

    void setOverlayDevice(vr::TrackedDeviceIndex_t deviceIndex);

    EHand getHandForSource(ThorQ::VR::OVRDevice device);
    const QMatrix4x4& getOffsetForHand(EHand hand);
    const QMatrix4x4& getOffsetForDevice(ThorQ::VR::OVRDevice device);
    ThorQ::VR::OVRDevice getOriginForHand(EHand hand);
    vr::TrackedDeviceIndex_t getDeviceForSource(vr::VRInputValueHandle_t source);
private:
	bool m_isVisible;
	float m_alpha;
	float m_width;
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
    vr::TrackedDeviceIndex_t m_overlayDevice;

    // Controller stuff
    EHand m_mouseHand;
    EHand m_overlayHand;
    vr::TrackedDeviceIndex_t m_mouseDeviceIndex;

    // Action set stuff
    vr::VRActiveActionSet_t m_activeActionSet;
    ThorQ::VR::OVRActionSet m_actionSet;
    ThorQ::VR::OVRAction    m_actionHaptics;
    ThorQ::VR::OVRAction    m_actionInteract;
    ThorQ::VR::OVRAction    m_actionShowOverlay;
    ThorQ::VR::OVRAction    m_actionProxSensor;

    ThorQ::VR::OVRDevice m_hmd;
    ThorQ::VR::OVRDevice m_controllerLeft;
    ThorQ::VR::OVRDevice m_controllerRight;

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
}

#endif // OPENVROVERLAYCONTROLLER_H

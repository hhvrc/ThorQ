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

namespace ThorQ {
namespace VR {
bool IsSteamVRInstalled();
bool IsSteamVRRunning();
bool IsHmdPresent();

bool Initialize();
void Shutdown();

bool IsManifestInstalled();
bool CreateManifest();
bool InstallManifest();
bool RemoveManifest();
}
}

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
	 * @brief init
     * @return
     */
	bool init();

    /**
	 * @brief shutdown
     */
	void shutdown();

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

    bool triggerHapticFeedback(EHand hand, float secondsFromNow, float amplitude, float frequency, float duration );

    bool openBindingUI();
protected:
	void update();

    bool pullEvents();

	bool createOverlay();
	void onSceneChanged();
    bool overlayTransform();

    void setOverlayDevice(vr::TrackedDeviceIndex_t deviceIndex);
    void setOverlayOffset(const QMatrix4x4& offset);



    EHand getHandForSource(vr::VRInputValueHandle_t source);
    const QMatrix4x4& getOffsetForHand(EHand hand);
    const QMatrix4x4& getOffsetForSource(vr::VRInputValueHandle_t source);
    vr::VRInputValueHandle_t getOriginForHand(EHand hand);
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
    vr::VROverlayHandle_t    m_overlayHandle;
    vr::HmdMatrix34_t        m_overlayOffset;
    vr::TrackedDeviceIndex_t m_overlayDevice;
    QMatrix4x4 m_overlayDeviceOffsetL;
    QMatrix4x4 m_overlayDeviceOffsetR;
    QMatrix4x4 m_overlayDeviceOffsetC;

    // Controller stuff
    EHand m_mouseHand;
    EHand m_overlayHand;
    vr::TrackedDeviceIndex_t m_mouseDeviceIndex;

    // Action set stuff
    vr::VRActiveActionSet_t  m_activeActionSet;
    vr::VRActionSetHandle_t  m_handleActionSet;
    vr::VRActionHandle_t     m_handleActionHapticsLeft;
    vr::VRActionHandle_t     m_handleActionHapticsRight;
    vr::VRActionHandle_t     m_handleActionInteract;
    vr::VRActionHandle_t     m_handleActionShowOverlay;
    vr::VRActionHandle_t     m_handleActionProxSensor;

    vr::VRInputValueHandle_t m_sourceHMD;
    vr::VRInputValueHandle_t m_sourceControllerLeft;
    vr::VRInputValueHandle_t m_sourceControllerRight;

	// Graphics
	QGraphicsScene *m_scene;
	QOpenGLContext *m_glContext;
	QOffscreenSurface *m_surface;
	QOpenGLFramebufferObject *m_frameBuffer;

    // Input handling
    QPointF m_lastMousePoint;
    Qt::MouseButtons m_lastMouseButtons;
};


#endif // OPENVROVERLAYCONTROLLER_H

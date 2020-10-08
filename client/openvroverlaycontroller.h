#ifndef OPENVROVERLAYCONTROLLER_H
#define OPENVROVERLAYCONTROLLER_H

#ifdef _WIN32
#pragma once
#endif

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

/**
 * @brief The OpenVROverlayController class
 */
class OpenVROverlayController : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(OpenVROverlayController)
public:
	/**
	 * @brief Checks if SteamVR is installed
	 * @return Returns if SteamVR is installed
	 */
	static bool IsSteamVRInstalled();

	/**
	 * @brief Checks if SteamVR is running
	 * @return Returns if SteamVR is running
	 */
	static bool IsSteamVRRunning();

	/**
	 * @brief Checks if a VR headset is connected to the computer
	 * @return Returns if a VR headset is connected to the computer
	 */
    static bool IsHmdPresent();
public:
	OpenVROverlayController(QObject* parent = nullptr);
	~OpenVROverlayController() override;

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

	/**
	 * @brief visibilityTimeout
	 * @return
	 */
	int visibilityTimeout() const;

	/**
	 * @brief visibilityTimeoutEnabled
	 * @return
	 */
	bool visibilityTimeoutEnabled() const;
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
	 * @brief visibilityTimeoutChanged
	 * @param timeout
	 */
	void visibilityTimeoutChanged(int timeout);

	/**
	 * @brief visibilityTimeoutEnabledChanged
	 * @param enabled
	 */
	void visibilityTimeoutEnabledChanged(bool enabled);

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
	 * @brief setVisibilityTimeout
	 * @param timeout
	 */
	void setVisibilityTimeout(int timeout);

	/**
	 * @brief setVisibilityTimeoutEnabled
	 * @param isEnabled
	 */
	void setVisibilityTimeoutEnabled(bool isEnabled);

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
protected:
	void visibilityTimeoutExpired();

	void update();

	bool createOverlay();
	void onSceneChanged();
    void overlayTransform();

    void setPriController(vr::TrackedDeviceIndex_t index);
	bool isOculus(vr::TrackedDeviceIndex_t index) const;
private:
	bool m_isVisible;
	float m_alpha;
	float m_width;
	QColor m_tint;

	bool m_timeoutEnabled;

	// Widget
	QGraphicsProxyWidget* m_proxyWidget;
	QTimer* m_updateLogicTimer;
	QTimer* m_visibilityTimer;

	// Overlay stuff
	vr::IVRSystem* m_vrSystem;
	vr::IVRInput* m_vrInput;
	vr::IVROverlay* m_vrOverlay;
	vr::IVRSettings* m_vrSettings;
	vr::VROverlayHandle_t m_overlay;
    vr::HmdVector2_t m_windowSize;

    // Controller stuff
    vr::TrackedDeviceIndex_t m_controller_pri;
    vr::TrackedDeviceIndex_t m_controller_sec;

	// Overlay offset
    QMatrix4x4* m_overlay_offset;
	QMatrix4x4  m_overlay_offset_L;
	QMatrix4x4  m_overlay_offset_R;
	QMatrix4x4  m_overlay_offset_U;

	// Graphics
	QGraphicsScene *m_scene;
	QOpenGLContext *m_glContext;
	QOffscreenSurface *m_surface;
	QOpenGLFramebufferObject *m_frameBuffer;

    // Input handling
    QPointF m_lastMousePoint;
    Qt::MouseButtons m_lastMouseButtons;

	std::string m_vrManifestPath;
};


#endif // OPENVROVERLAYCONTROLLER_H

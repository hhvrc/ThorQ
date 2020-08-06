//====== Copyright Valve Corporation, All rights reserved. =======

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
     * @brief IsSteamVRRunning
     * @return
     */
	static bool IsSteamVRRunning();

    /**
     * @brief IsHmdPresent
     * @return
     */
    static bool IsHmdPresent();
public:
    /**
     * @brief OpenVROverlayController
     * @param parent
     */
	OpenVROverlayController(QObject* parent = nullptr);

    /**
     * @brief ~OpenVROverlayController
     */
	~OpenVROverlayController() override;

    /**
     * @brief GetWidget
     * @return
     */
	QWidget* GetWidget() const;

    /**
     * @brief GetIsVisible
     * @return
     */
	bool GetIsVisible() const;

    /**
     * @brief GetWidth
     * @return
     */
	float GetWidth() const;

    /**
     * @brief GetAlpha
     * @return
     */
	float GetAlpha() const;

    /**
     * @brief GetTint
     * @return
     */
	QColor GetTint() const;
signals:
    /**
     * @brief VrExited
     */
	void VrExited();

    /**
     * @brief WidgetChanged
     * @param widget
     */
	void WidgetChanged(QWidget* widget);

    /**
     * @brief IsVisibleChanged
     * @param visible
     */
	void IsVisibleChanged(bool visible);

    /**
     * @brief WidthChanged
     * @param width
     */
	void WidthChanged(float width);

    /**
     * @brief AlphaChanged
     * @param alpha
     */
	void AlphaChanged(float alpha);

    /**
     * @brief TintChanged
     * @param color
     */
	void TintChanged(const QColor& color);
public slots:
    /**
     * @brief Init
     * @return
     */
	bool Init();

    /**
     * @brief Shutdown
     */
	void Shutdown();

    /**
     * @brief SetWidget
     * @param widget
     */
	void SetWidget(QWidget* widget);

    /**
     * @brief SetIsVisible
     * @param visible
     */
	void SetIsVisible(bool visible);

    /**
     * @brief ToggleIsVisible
     */
	void ToggleIsVisible();

    /**
     * @brief SetIsVisibleTimeout
     * @param enabled
     * @param msecs
     */
	void SetIsVisibleTimeout(bool enabled, int msecs);

    /**
     * @brief SetWidth
     * @param width
     */
	void SetWidth(float width);

    /**
     * @brief SetAlpha
     * @param alpha
     */
	void SetAlpha(float alpha);

    /**
     * @brief SetTint
     * @param color
     */
	void SetTint(const QColor& color);
protected:
    /**
     * @brief ConnectToVRRuntime
     * @return
     */
    bool ConnectToVRRuntime();

    /**
     * @brief DisconnectFromVRRuntime
     */
    void DisconnectFromVRRuntime();

    /**
     * @brief SetTrackedDevice
     * @param index
     */
	void SetTrackedDevice(vr::TrackedDeviceIndex_t index);

    /**
     * @brief GetTrackedDevice
     * @return
     */
	vr::TrackedDeviceIndex_t GetTrackedDevice() const;

    /**
     * @brief PollEvents
     */
	void PollEvents();

    /**
     * @brief SetOverlayResolution
     * @param width
     * @param height
     */
	void SetOverlayResolution(int width, int height);

    /**
     * @brief OverlayCreate
     */
	void OverlayCreate();

    /**
     * @brief OverlayInit
     */
	void OverlayInit();

    /**
     * @brief OverlayProcess
     */
	void OverlayProcess();

    /**
     * @brief OverlayDraw
     */
	void OverlayDraw();

    /**
     * @brief OverlayTransform
     */
	void OverlayTransform();
private:
	bool m_isInitialized;
	bool m_isVisible;
	float m_alpha;
	float m_width;
	QColor m_tint;

	// Widget
	QWidget *m_widget;
	QTimer *m_pumpEventsTimer;
	QTimer* m_visibilityTimer;

	// Overlay stuff
	vr::IVRSystem* m_ivrSystem;
	vr::VROverlayHandle_t m_handle;
    vr::HmdMatrix34_t* m_deviceOffset;
    vr::HmdMatrix34_t m_L_deviceOffset;
    vr::HmdMatrix34_t m_R_deviceOffset;
    vr::HmdVector2_t m_windowSize;
	vr::TrackedDeviceIndex_t m_deviceIndex;

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

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

class OpenVROverlayController : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(OpenVROverlayController)
public:
    static bool IsSteamVRRunning();
    static bool IsHmdPresent();
    static OpenVROverlayController* SharedInstance();
public:
    OpenVROverlayController(QObject* parent = nullptr);
    ~OpenVROverlayController() override;

    QWidget* GetWidget() const;
    bool GetIsVisible() const;
    float GetWidth() const;
    float GetAlpha() const;
    QColor GetTint() const;
signals:
    void VrExited();

    void WidgetChanged(QWidget* widget);
    void IsVisibleChanged(bool visible);
    void WidthChanged(float width);
    void AlphaChanged(float alpha);
    void TintChanged(const QColor& color);
public slots:
    bool Init();
    void Shutdown();

    void SetWidget(QWidget* widget);
    void SetIsVisible(bool visible);
    void ToggleIsVisible();
    void SetIsVisibleTimeout(bool enabled, int msecs);
    void SetWidth(float width);
    void SetAlpha(float alpha);
    void SetTint(const QColor& color);
protected:
    bool ConnectToVRRuntime();
    void DisconnectFromVRRuntime();

    void SetTrackedDevice(vr::TrackedDeviceIndex_t index);
    vr::TrackedDeviceIndex_t GetTrackedDevice() const;

    void PollEvents();

    void SetOverlayResolution(int width, int height);

    void OverlayCreate();
    void OverlayInit();
    void OverlayProcess();
    void OverlayDraw();
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
    vr::HmdMatrix34_t m_deviceOffset;
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

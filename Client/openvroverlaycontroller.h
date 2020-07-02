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

public:
    static bool IsSteamVRRunning();
    static bool IsHmdPresent();
    static OpenVROverlayController *SharedInstance();

    OpenVROverlayController();
    ~OpenVROverlayController() override;

    bool Init();
    void Shutdown();

    void SetWidget( QWidget* pWidget );
    QWidget* GetWidget() const;

    void SetTint(const QColor& color);
    QColor GetTint() const;

    void SetAlpha(float alpha);
    float GetAlpha() const;

    void SetWidth(float meters);
    float GetWidth() const;
signals:
    void VrExited();
public slots:
    vr::EVRInitError ConnectToVRRuntime();
    void DisconnectFromVRRuntime();

    void PollEvents();
    vr::EVROverlayError DrawOverlay(bool show = true, float size = 1.f, float alpha = 0.9f);

protected:

private:

    // OPENVR VARIABLES
    vr::IVRSystem* m_system;

    // Overlay stuff
    vr::VROverlayHandle_t m_handle;
    vr::HmdMatrix34_t m_deviceOffset;
    vr::TrackedDeviceIndex_t m_deviceIndex;

    // HANDLERS

    // QT VARIABLES

    // Visibility
    bool m_isVisible;
    QElapsedTimer m_visibleTimeout;

    // Graphics
    QGraphicsScene *m_scene;
    QOpenGLContext *m_glContext;
    QOffscreenSurface *m_surface;
    QOpenGLFramebufferObject *m_frameBuffer;

    // Event loop
    QTimer *m_pumpEventsTimer;

    // Widget
    QWidget *m_widget;

    // Input handling
    QPointF m_lastMousePoint;
    Qt::MouseButtons m_lastMouseButtons;
};


#endif // OPENVROVERLAYCONTROLLER_H

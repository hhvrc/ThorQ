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

    bool m_isVisible;
    float m_alpha;
    float m_width;
    QColor m_tint;

public:
    static bool IsSteamVRRunning();
    static bool IsHmdPresent();
    static OpenVROverlayController *SharedInstance();
public:
    OpenVROverlayController();
    ~OpenVROverlayController() override;

    void SetWidget( QWidget* pWidget );
    QWidget* GetWidget() const;

    void SetIsVisible(bool show);
    bool GetIsVisible();

    void SetWidth(float meters);
    float GetWidth() const;

    void SetAlpha(float alpha);
    float GetAlpha() const;

    void SetTint(const QColor& color);
    QColor GetTint() const;
signals:
    void VrExited();
public slots:
    bool Init();
    void Shutdown();
protected:
    vr::EVRInitError ConnectToVRRuntime();
    void DisconnectFromVRRuntime();

    void PollEvents();

    void OverlayCreate();
    void OverlayInit();
    void OverlayProcess();
    void OverlayDraw();
    void OverlayTransform();
private:
    // Widget
    QWidget *m_widget;
    QTimer *m_pumpEventsTimer;
    QElapsedTimer m_visibleTimeout;

    // OPENVR VARIABLES
    vr::IVRSystem* m_system;

    // Overlay stuff
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

//====== Copyright Valve Corporation, All rights reserved. =======


#include "openvroverlaycontroller.h"


#include <QOpenGLFramebufferObjectFormat>
#include <QOpenGLPaintDevice>
#include <QPainter>
#include <QtWidgets/QWidget>
#include <QMouseEvent>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsEllipseItem>
#include <QCursor>

using namespace vr;

bool OpenVROverlayController::IsSteamVRRunning()
{
    // TODO: Look for process with name "SteamVR"
    return false;
}

bool OpenVROverlayController::IsHmdPresent()
{
    return vr::VR_IsHmdPresent();
}

OpenVROverlayController* s_pSharedVRController = nullptr;

OpenVROverlayController *OpenVROverlayController::SharedInstance()
{
    if (s_pSharedVRController == nullptr)
    {
        s_pSharedVRController = new OpenVROverlayController();
    }
    return s_pSharedVRController;
}

OpenVROverlayController::OpenVROverlayController()
    : QObject()
    , m_handle(k_ulOverlayHandleInvalid)
    , m_deviceIndex(k_unTrackedDeviceIndexInvalid)
    , m_scene( nullptr )
    , m_glContext( nullptr )
    , m_surface ( nullptr )
    , m_frameBuffer( nullptr )
    , m_pumpEventsTimer( nullptr )
    , m_widget( nullptr )
    , m_lastMouseButtons( 0 )
{
}

OpenVROverlayController::~OpenVROverlayController()
{
}

bool OpenVROverlayController::Init()
{
    QSurfaceFormat format;
    format.setMajorVersion( 4 );
    format.setMinorVersion( 1 );
    format.setProfile( QSurfaceFormat::CompatibilityProfile );

    m_glContext = new QOpenGLContext();
    m_glContext->setFormat( format );
    if(!m_glContext->create())
        return false;

    // create an offscreen surface to attach the context and FBO to
    m_surface = new QOffscreenSurface();
    m_surface->create();
    m_glContext->makeCurrent( m_surface );

    m_scene = new QGraphicsScene();

    // Loading the OpenVR Runtime
    {
        EVRInitError err = ConnectToVRRuntime();

        if (err != VRInitError_None || vr::VRCompositor() == nullptr || vr::VROverlay() == nullptr)
            return false;
    }

    // Initialize overlay
    DrawOverlay(false);

    m_pumpEventsTimer = new QTimer( this );
    connect(m_pumpEventsTimer, &QTimer::timeout, this, &OpenVROverlayController::PollEvents);
    m_pumpEventsTimer->setInterval( 20 );
    m_pumpEventsTimer->start();

    return true;
}

void OpenVROverlayController::Shutdown()
{
    DisconnectFromVRRuntime();

    //delete m_scene;
    //delete m_frameBuffer;
    //delete m_surface;

    if(m_glContext != nullptr)
    {
        delete m_glContext;
        m_glContext = nullptr;
    }
}

void OpenVROverlayController::SetWidget( QWidget* widget )
{
    if( m_scene )
    {
        // all of the mouse handling stuff requires that the widget be at 0,0
        widget->move( 0, 0 );
        m_scene->addWidget( widget );
    }
    m_widget = widget;

    if (m_frameBuffer != nullptr)
        delete m_frameBuffer;

    m_frameBuffer = new QOpenGLFramebufferObject( widget->width(), widget->height(), GL_TEXTURE_2D );

    if( vr::VROverlay() )
    {
        vr::HmdVector2_t vecWindowSize =
        {
            (float)widget->width(),
            (float)widget->height()
        };
        vr::VROverlay()->SetOverlayMouseScale( m_handle, &vecWindowSize );
    }

    DrawOverlay(true);
}

QWidget* OpenVROverlayController::GetWidget() const
{
    return m_widget;
}

void OpenVROverlayController::SetTint(const QColor& color)
{
    if (!color.isValid())
        return;

    vr::VROverlay()->SetOverlayColor(m_handle, color.redF(), color.greenF(), color.blueF());
}

QColor OpenVROverlayController::GetTint() const
{
    float r, g, b;
    vr::VROverlay()->GetOverlayColor(m_handle, &r, &g, &b);

    QColor color;
    color.setRedF(r);
    color.setGreenF(g);
    color.setBlueF(b);
    return color;
}

void OpenVROverlayController::SetAlpha(float alpha)
{
    vr::VROverlay()->SetOverlayAlpha(m_handle, alpha);
}

float OpenVROverlayController::GetAlpha() const
{
    float alpha;
    vr::VROverlay()->GetOverlayAlpha(m_handle, &alpha);
    return alpha;
}

void OpenVROverlayController::SetWidth(float meters)
{
    vr::VROverlay()->SetOverlayWidthInMeters(m_handle, meters);
}

float OpenVROverlayController::GetWidth() const
{
    float meters;
    vr::VROverlay()->GetOverlayWidthInMeters(m_handle, &meters);
    return meters;
}

EVRInitError OpenVROverlayController::ConnectToVRRuntime()
{
    EVRInitError err = VRInitError_None;
    m_system = vr::VR_Init( &err, vr::VRApplication_Overlay );
    return err;
}

void OpenVROverlayController::DisconnectFromVRRuntime()
{
    vr::VR_Shutdown();
}

void OpenVROverlayController::PollEvents()
{
    if( !vr::VRSystem() )
        return;

    vr::VREvent_t vrEvent;
    while( vr::VROverlay()->PollNextOverlayEvent( m_handle, &vrEvent, sizeof( vrEvent )  ) )
    {
        switch( vrEvent.eventType )
        {
        case vr::VREvent_MouseMove:
        {
            QPointF ptNewMouse( vrEvent.data.mouse.x, vrEvent.data.mouse.y );
            QPoint ptGlobal = ptNewMouse.toPoint();
            QGraphicsSceneMouseEvent mouseEvent( QEvent::GraphicsSceneMouseMove );
            mouseEvent.setWidget( NULL );
            mouseEvent.setPos( ptNewMouse );
            mouseEvent.setScenePos( ptGlobal );
            mouseEvent.setScreenPos( ptGlobal );
            mouseEvent.setLastPos( m_lastMousePoint );
            mouseEvent.setLastScenePos( m_widget->mapToGlobal( m_lastMousePoint.toPoint() ) );
            mouseEvent.setLastScreenPos( m_widget->mapToGlobal( m_lastMousePoint.toPoint() ) );
            mouseEvent.setButtons( m_lastMouseButtons );
            mouseEvent.setButton( Qt::NoButton );
            mouseEvent.setModifiers( 0 );
            mouseEvent.setAccepted( false );

            m_lastMousePoint = ptNewMouse;
            QApplication::sendEvent( m_scene, &mouseEvent );

            // Fixme
            DrawOverlay(true);
        }
            break;

        case vr::VREvent_MouseButtonDown:
        {
            Qt::MouseButton button = vrEvent.data.mouse.button == vr::VRMouseButton_Right ? Qt::RightButton : Qt::LeftButton;

            m_lastMouseButtons |= button;

            QPoint ptGlobal = m_lastMousePoint.toPoint();
            QGraphicsSceneMouseEvent mouseEvent( QEvent::GraphicsSceneMousePress );
            mouseEvent.setWidget( NULL );
            mouseEvent.setPos( m_lastMousePoint );
            mouseEvent.setButtonDownPos( button, m_lastMousePoint );
            mouseEvent.setButtonDownScenePos( button, ptGlobal);
            mouseEvent.setButtonDownScreenPos( button, ptGlobal );
            mouseEvent.setScenePos( ptGlobal );
            mouseEvent.setScreenPos( ptGlobal );
            mouseEvent.setLastPos( m_lastMousePoint );
            mouseEvent.setLastScenePos( ptGlobal );
            mouseEvent.setLastScreenPos( ptGlobal );
            mouseEvent.setButtons( m_lastMouseButtons );
            mouseEvent.setButton( button );
            mouseEvent.setModifiers( 0 );
            mouseEvent.setAccepted( false );

            QApplication::sendEvent( m_scene, &mouseEvent );
        }
            break;

        case vr::VREvent_MouseButtonUp:
        {
            Qt::MouseButton button = vrEvent.data.mouse.button == vr::VRMouseButton_Right ? Qt::RightButton : Qt::LeftButton;
            m_lastMouseButtons &= ~button;

            QPoint ptGlobal = m_lastMousePoint.toPoint();
            QGraphicsSceneMouseEvent mouseEvent( QEvent::GraphicsSceneMouseRelease );
            mouseEvent.setWidget( NULL );
            mouseEvent.setPos( m_lastMousePoint );
            mouseEvent.setScenePos( ptGlobal );
            mouseEvent.setScreenPos( ptGlobal );
            mouseEvent.setLastPos( m_lastMousePoint );
            mouseEvent.setLastScenePos( ptGlobal );
            mouseEvent.setLastScreenPos( ptGlobal );
            mouseEvent.setButtons( m_lastMouseButtons );
            mouseEvent.setButton( button );
            mouseEvent.setModifiers( 0 );
            mouseEvent.setAccepted( false );

            QApplication::sendEvent(  m_scene, &mouseEvent );
        }
            break;

        case vr::VREvent_ButtonPress:
        {
            vr::VRControllerState_t state;
            m_system->GetControllerState(vrEvent.trackedDeviceIndex, &state, sizeof( state ));
            bool gripPushed = (state.ulButtonPressed & vr::ButtonMaskFromId(vr::EVRButtonId::k_EButton_Grip)) != 0;
            // Move overlay if true and focused on overlay
            qDebug() << ">w<" << gripPushed;

            DrawOverlay(false);
        }
            break;

        case vr::VREvent_ButtonTouch:
            qDebug() << "UwU";
            break;

        case vr::VREvent_OverlayShown:
        {
            m_widget->repaint();
        }
            break;

        case vr::VREvent_Quit:
            QApplication::exit();
            break;
        }
    }
}

EVROverlayError OpenVROverlayController::DrawOverlay(bool show, float size, float alpha)
{
    EVROverlayError err = VROverlayError_None;

    // Find or create the overlay
    if (m_handle == 0)
    {
        err = VROverlay()->FindOverlay("ThorQ", &m_handle);
        if (err != VROverlayError_None)
        {
            if (err != VROverlayError_UnknownOverlay)
                return err;

            err = VROverlay()->CreateOverlay("ThorQ", "ThorQ", &m_handle);
            if (err != VROverlayError_None)
                return err;

            err = VROverlay()->SetOverlayAlpha(m_handle, alpha);
            if (err != VROverlayError_None)
                return err;

            err = VROverlay()->SetOverlayWidthInMeters(m_handle, size);
            if (err != VROverlayError_None)
                return err;

            err = VROverlay()->SetOverlayInputMethod(m_handle, VROverlayInputMethod_Mouse);
            if (err != VROverlayError_None)
                return err;

            err = vr::VROverlay()->SetOverlayFlag(m_handle, VROverlayFlags::VROverlayFlags_NoDashboardTab, true);
            if (err != VROverlayError_None)
                return err;

            err = vr::VROverlay()->SetOverlayFlag(m_handle, VROverlayFlags::VROverlayFlags_MakeOverlaysInteractiveIfVisible, true);
            if (err != VROverlayError_None)
                return err;
        }
    }

    // Offset overlay from device (most likely a controller)
    if (m_deviceIndex != k_unTrackedDeviceIndexInvalid)
    {
        err = VROverlay()->SetOverlayTransformTrackedDeviceRelative(m_handle, m_deviceIndex, &m_deviceOffset);
        if (err != VROverlayError_None)
            return err;
    }

    if (show)
    {
        m_glContext->makeCurrent(m_surface);
        m_frameBuffer->bind();

        QOpenGLPaintDevice device(m_frameBuffer->size());
        QPainter painter(&device);

        m_scene->render(&painter);

        m_frameBuffer->release();

        GLuint glTexture = m_frameBuffer->texture();
        if (glTexture != 0)
        {
            vr::Texture_t texture = {(void*)(uintptr_t)glTexture, vr::TextureType_OpenGL, vr::ColorSpace_Auto };
            vr::VROverlay()->SetOverlayTexture( m_handle, &texture );
        }

        if (!m_isVisible)
        {
            err = VROverlay()->ShowOverlay(m_handle);
            if (err != VROverlayError_None)
                return err;

            m_isVisible = true;
        }
    }
    else if (m_isVisible)
    {
        err = VROverlay()->HideOverlay(m_handle);
        if (err != VROverlayError_None)
            return err;

        m_isVisible = false;
    }

    return err;
}

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

void Dbg(vr::VROverlayError err, int line)
{
    if (err != vr::VROverlayError_None)
        qDebug() << "Error:" << err << "line:"<< line;
}

QMatrix4x4 ToQMatrix(const vr::HmdMatrix34_t& mat)
{
    return QMatrix4x4(
                mat.m[0][0], mat.m[0][1], mat.m[0][2], mat.m[0][3],
                mat.m[1][0], mat.m[1][1], mat.m[1][2], mat.m[1][3],
                mat.m[2][0], mat.m[2][1], mat.m[2][2], mat.m[2][3],
                0.f,         0.f,         0.f,         1.f
            );
}
vr::HmdMatrix34_t ToHmdMatrix34(const QMatrix4x4& mat)
{
    vr::HmdMatrix34_t ret;
    for (int i = 0; i < 3; i++)
    {
        QVector4D row = mat.row(i);
        ret.m[i][0] = row.x();
        ret.m[i][1] = row.y();
        ret.m[i][2] = row.z();
        ret.m[i][3] = row.w();
    }
    return ret;
}

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

OpenVROverlayController::OpenVROverlayController(QObject* parent)
    : QObject(parent)
    , m_widget( nullptr )
    , m_pumpEventsTimer(new QTimer(this))
    , m_visibilityTimer(new QTimer(this))
    , m_system( nullptr )
    , m_handle(vr::k_ulOverlayHandleInvalid)
    , m_deviceOffset()
    , m_deviceIndex(vr::k_unTrackedDeviceIndexInvalid)
    , m_scene( nullptr )
    , m_glContext( nullptr )
    , m_surface ( nullptr )
    , m_frameBuffer( nullptr )
    , m_lastMousePoint()
    , m_lastMouseButtons( 0 )
{
    connect(m_pumpEventsTimer, &QTimer::timeout, this, &OpenVROverlayController::PollEvents);
    connect(m_visibilityTimer, &QTimer::timeout, [](){ qDebug() << "Hide!"; });

    m_pumpEventsTimer->setInterval(20);

    m_visibilityTimer->setInterval(5000);
    m_visibilityTimer->setSingleShot(true);
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
        vr::EVRInitError err = ConnectToVRRuntime();

        if (err != vr::VRInitError_None || vr::VRCompositor() == nullptr || vr::VROverlay() == nullptr)
            return false;
    }

    // Initialize overlay
    OverlayProcess();

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

    OverlayProcess();
}
QWidget* OpenVROverlayController::GetWidget() const
{
    return m_widget;
}

vr::EVRInitError OpenVROverlayController::ConnectToVRRuntime()
{
    vr::EVRInitError err = vr::VRInitError_None;
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

    SetIsVisible(true);

    vr::VREvent_t event;
    while (vr::VRSystem()->PollNextEvent(&event, sizeof(event)))
    {
        switch(event.eventType)
        {
        case vr::VREvent_ButtonPress:
        {
            vr::VRControllerState_t state;
            m_system->GetControllerState(event.trackedDeviceIndex, &state, sizeof( state ));
            bool pushed = (state.ulButtonPressed & vr::ButtonMaskFromId(vr::EVRButtonId::k_EButton_ApplicationMenu)) != 0;


            if (pushed)
            {
                qDebug() << "Show!";
                m_visibilityTimer->start();
                m_deviceIndex = event.trackedDeviceIndex;
                OverlayProcess();
            }
        }
            break;

        case vr::VREvent_OverlayShown:
        {
            m_widget->repaint();
        }
            break;

        case vr::VREvent_Quit:
            QApplication::exit();
            break;

        case vr::VREvent_MouseMove:
        case vr::VREvent_MouseButtonDown:
        case vr::VREvent_MouseButtonUp:
            qDebug() << "Please dont say that this captures these events...";
            break;
        }
    }

    while(vr::VROverlay()->PollNextOverlayEvent(m_handle, &event, sizeof(event)))
    {
        switch(event.eventType)
        {
        case vr::VREvent_MouseMove:
        {
            qDebug() << "MouseMove!";
            QPointF ptNewMouse( event.data.mouse.x, event.data.mouse.y );
            QPoint ptGlobal = ptNewMouse.toPoint();
            QGraphicsSceneMouseEvent mouseEvent( QEvent::GraphicsSceneMouseMove );
            mouseEvent.setWidget( nullptr );
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
            OverlayProcess();
        }
            break;

        case vr::VREvent_MouseButtonDown:
        {
            qDebug() << "MouseDown!";
            Qt::MouseButton button = event.data.mouse.button == vr::VRMouseButton_Right ? Qt::RightButton : Qt::LeftButton;

            m_lastMouseButtons |= button;

            QPoint ptGlobal = m_lastMousePoint.toPoint();
            QGraphicsSceneMouseEvent mouseEvent( QEvent::GraphicsSceneMousePress );
            mouseEvent.setWidget( nullptr );
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
            qDebug() << "MouseUp!";
            Qt::MouseButton button = event.data.mouse.button == vr::VRMouseButton_Right ? Qt::RightButton : Qt::LeftButton;
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
        }
    }
}

void OpenVROverlayController::OverlayCreate()
{
    vr::EVROverlayError err = vr::VROverlayError_None;

    qDebug() << "Find";
    err = vr::VROverlay()->FindOverlay("ThorQ", &m_handle);
    if (err != vr::VROverlayError_None)
    {
        if (err != vr::VROverlayError_UnknownOverlay)
        {
            Dbg(err, __LINE__);
            return;
        }

        qDebug() << "Create";
        Dbg(vr::VROverlay()->CreateOverlay("ThorQ", "ThorQ", &m_handle), __LINE__);

        OverlayInit();
    }
}
void OpenVROverlayController::OverlayInit()
{
    if (m_handle == 0)
        return;

    // Alpha
    m_alpha = 1.f;
    Dbg(vr::VROverlay()->SetOverlayAlpha(m_handle, m_alpha), __LINE__);

    // Tint
    m_tint = QColor(0, 0, 0, 255);
    Dbg(vr::VROverlay()->SetOverlayColor(m_handle, m_tint.redF(), m_tint.greenF(), m_tint.blueF()), __LINE__);

    // Visibility
    m_isVisible = false;
    Dbg(vr::VROverlay()->HideOverlay(m_handle), __LINE__);

    // Flags
    Dbg(vr::VROverlay()->SetOverlayInputMethod(m_handle, vr::VROverlayInputMethod_Mouse), __LINE__);
    Dbg(vr::VROverlay()->SetOverlayFlag(m_handle, vr::VROverlayFlags_VisibleInDashboard, false), __LINE__);

    // I want this, but it blocks user input :c
    //Dbg(vr::VROverlay()->SetOverlayFlag(m_handle, vr::VROverlayFlags_MakeOverlaysInteractiveIfVisible, true), __LINE__);

    m_visibilityTimer->start();
}
void OpenVROverlayController::OverlayProcess()
{
    // Find or create the overlay
    if (m_handle == 0)
        OverlayCreate();

    // Offset overlay from device (most likely a controller)
    OverlayTransform();

    if (m_isVisible)
        OverlayDraw();
}
void OpenVROverlayController::OverlayDraw()
{
    if (m_handle == 0)
    {
        qDebug() << "Handle is invalid" << __LINE__;
        return;
    }
    if (m_frameBuffer == nullptr)
    {
        qDebug() << "Framebuffer is nullptr" << __LINE__;
        return;
    }

    qDebug() << "Draw";


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
        Dbg(vr::VROverlay()->SetOverlayTexture( m_handle, &texture), __LINE__);
    }
}
void OpenVROverlayController::OverlayTransform()
{
    if (m_handle == 0)
    {
        qDebug() << "Handle is invalid" << __LINE__;
        return;
    }
    if (m_deviceIndex == vr::k_unTrackedDeviceIndexInvalid)
    {
        qDebug() << "Device is invalid" << __LINE__;
        return;
    }
    qDebug() << "Reposition";

    // Calculate offset
    QMatrix4x4 mat;
    mat.translate(0,0,0);
    mat.rotate(-90, 1, 0);
    m_deviceOffset = ToHmdMatrix34(mat);

    // Position
    Dbg(vr::VROverlay()->SetOverlayTransformTrackedDeviceRelative(m_handle, m_deviceIndex, &m_deviceOffset), __LINE__);
}

void OpenVROverlayController::SetIsVisible(bool show)
{
    if (m_handle == 0)
        return;

    if (show && !m_isVisible)
    {
        qDebug() << "Show";
        Dbg(vr::VROverlay()->ShowOverlay(m_handle), __LINE__);
        m_isVisible = true;
    }
    else if (!show && m_isVisible)
    {
        qDebug() << "Hide";
        Dbg(vr::VROverlay()->HideOverlay(m_handle), __LINE__);
        m_isVisible = false;
    }
}
bool OpenVROverlayController::GetIsVisible()
{
    return m_isVisible;
}

void OpenVROverlayController::SetWidth(float width)
{
    if (m_handle == 0)
    {
        qDebug() << "Handle is invalid" << __LINE__;
        return;
    }

    if (m_width != width)
    {
        m_width = width;
        qDebug() << "Width" << m_width;
        Dbg(vr::VROverlay()->SetOverlayWidthInMeters(m_handle, m_width), __LINE__);
    }
}
float OpenVROverlayController::GetWidth() const
{
    return m_width;
}

void OpenVROverlayController::SetAlpha(float alpha)
{
    if (m_handle == 0)
    {
        qDebug() << "Handle is invalid" << __LINE__;
        return;
    }

    if (m_alpha != alpha)
    {
        m_alpha = alpha;
        qDebug() << "Alpha" << alpha;
        Dbg(vr::VROverlay()->SetOverlayAlpha(m_handle, alpha), __LINE__);
    }
}
float OpenVROverlayController::GetAlpha() const
{
    return m_alpha;
}

void OpenVROverlayController::SetTint(const QColor& tint)
{
    if (m_handle == 0)
        return;
    if (!tint.isValid())
        return;

    if (m_tint != tint)
    {
        m_tint = tint;
        qDebug() << "Tint" << tint;
        Dbg(vr::VROverlay()->SetOverlayColor(m_handle, m_tint.redF(), m_tint.greenF(), m_tint.blueF()), __LINE__);
    }
}
QColor OpenVROverlayController::GetTint() const
{
    return m_tint;
}


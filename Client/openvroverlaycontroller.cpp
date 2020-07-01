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

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
OpenVROverlayController* s_pSharedVRController = nullptr;

OpenVROverlayController *OpenVROverlayController::SharedInstance()
{
	if (s_pSharedVRController == nullptr)
	{
		s_pSharedVRController = new OpenVROverlayController();
	}
	return s_pSharedVRController;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
OpenVROverlayController::OpenVROverlayController()
	: QObject()
	, m_strVRDriver( "No Driver" )
	, m_strVRDisplay( "No Display" )
	, m_hmdError( vr::VRInitError_None )
	, m_compositorError( vr::VRInitError_None )
	, m_overlayError( vr::VRInitError_None )
	, m_overlayHandle( vr::k_ulOverlayHandleInvalid )
	, m_openGLContext( nullptr )
	, m_scene( nullptr )
	, m_frameBuffer( nullptr )
	, m_vrSurface ( nullptr )
	, m_pumpEventsTimer( nullptr )
	, m_widget( nullptr )
	, m_lastMouseButtons( 0 )
{
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
OpenVROverlayController::~OpenVROverlayController()
{
}


//-----------------------------------------------------------------------------
// Purpose: Helper to get a string from a tracked device property and turn it
//			into a QString
//-----------------------------------------------------------------------------
QString GetTrackedDeviceString( vr::IVRSystem *pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop )
{
	char buf[128];
	vr::TrackedPropertyError err;
	pHmd->GetStringTrackedDeviceProperty( unDevice, prop, buf, sizeof( buf ), &err );
	if( err != vr::TrackedProp_Success )
	{
		return QString( "Error Getting String: " ) + pHmd->GetPropErrorNameFromEnum( err );
	}
	else
	{
		return buf;
	}
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool OpenVROverlayController::Init(const QString& name)
{
	bool bSuccess = true;

	m_strName = name;

	QSurfaceFormat format;
	format.setMajorVersion( 4 );
	format.setMinorVersion( 1 );
	format.setProfile( QSurfaceFormat::CompatibilityProfile );

	m_openGLContext = new QOpenGLContext();
	m_openGLContext->setFormat( format );
	bSuccess = m_openGLContext->create();
	if( !bSuccess )
		return false;

	// create an offscreen surface to attach the context and FBO to
	m_vrSurface = new QOffscreenSurface();
	m_vrSurface->create();
	m_openGLContext->makeCurrent( m_vrSurface );

	m_scene = new QGraphicsScene();
	connect(m_scene, &QGraphicsScene::changed, this, &OpenVROverlayController::OnSceneChanged);

	// Loading the OpenVR Runtime
	bSuccess = ConnectToVRRuntime();

	bSuccess = bSuccess && vr::VRCompositor() != nullptr;

	if( vr::VROverlay() )
	{
		std::string sKey = std::string( "thorq." ) + m_strName.toStdString();
		vr::VROverlayError overlayError = vr::VROverlay()->CreateOverlay( sKey.c_str(), m_strName.toStdString().c_str(), &m_overlayHandle );
		bSuccess = bSuccess && overlayError == vr::VROverlayError_None;
	}

	if( bSuccess )
	{
		vr::VROverlay()->SetOverlayWidthInMeters( m_overlayHandle, 1.5f );
		vr::VROverlay()->SetOverlayInputMethod( m_overlayHandle, vr::VROverlayInputMethod_Mouse );
		vr::VROverlay()->SetOverlayFlag(m_overlayHandle, VROverlayFlags::VROverlayFlags_NoDashboardTab, true);

		m_pumpEventsTimer = new QTimer( this );
		connect(m_pumpEventsTimer, &QTimer::timeout, this, &OpenVROverlayController::OnTimeoutPumpEvents);
		m_pumpEventsTimer->setInterval( 20 );
		m_pumpEventsTimer->start();

	}
	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void OpenVROverlayController::Shutdown()
{
	DisconnectFromVRRuntime();

	delete m_scene;
	delete m_frameBuffer;
	delete m_vrSurface;

	if( m_openGLContext )
	{
		//		m_pOpenGLContext->destroy();
		delete m_openGLContext;
		m_openGLContext = NULL;
	}
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void OpenVROverlayController::OnSceneChanged( const QList<QRectF>& )
{
	// skip rendering if the overlay isn't visible
	if( ( m_overlayHandle == k_ulOverlayHandleInvalid ) || !vr::VROverlay() || !vr::VROverlay()->IsOverlayVisible( m_overlayHandle ) )
		return;

	m_openGLContext->makeCurrent( m_vrSurface );
	m_frameBuffer->bind();

	QOpenGLPaintDevice device( m_frameBuffer->size() );
	QPainter painter( &device );

	m_scene->render( &painter );

	m_frameBuffer->release();

	GLuint unTexture = m_frameBuffer->texture();
	if( unTexture != 0 )
	{
		vr::Texture_t texture = {(void*)(uintptr_t)unTexture, vr::TextureType_OpenGL, vr::ColorSpace_Auto };
		vr::VROverlay()->SetOverlayTexture( m_overlayHandle, &texture );
	}
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void OpenVROverlayController::OnTimeoutPumpEvents()
{
	if( !vr::VRSystem() )
		return;

	vr::VREvent_t vrEvent;
	while( vr::VROverlay()->PollNextOverlayEvent( m_overlayHandle, &vrEvent, sizeof( vrEvent )  ) )
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

			OnSceneChanged( QList<QRectF>() );
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
			m_VRSystem->GetControllerState(vrEvent.trackedDeviceIndex, &state, sizeof( state ));
			bool gripPushed = (state.ulButtonPressed & vr::ButtonMaskFromId(vr::EVRButtonId::k_EButton_Grip)) != 0;
			// Move overlay if true and focused on overlay
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
		}
	}

}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
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
		vr::VROverlay()->SetOverlayMouseScale( m_overlayHandle, &vecWindowSize );
	}

}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
QWidget* OpenVROverlayController::GetWidget() const
{
	return m_widget;
}

void OpenVROverlayController::SetTint(const QColor& color)
{
	if (!color.isValid())
		return;

	vr::VROverlay()->SetOverlayColor(m_overlayHandle, color.redF(), color.greenF(), color.blueF());
}

QColor OpenVROverlayController::GetTint() const
{
	float r, g, b;
	vr::VROverlay()->GetOverlayColor(m_overlayHandle, &r, &g, &b);

	QColor color;
	color.setRedF(r);
	color.setGreenF(g);
	color.setBlueF(b);
	return color;
}

void OpenVROverlayController::SetAlpha(float alpha)
{
	vr::VROverlay()->SetOverlayAlpha(m_overlayHandle, alpha);
}

float OpenVROverlayController::GetAlpha() const
{
	float alpha;
	vr::VROverlay()->GetOverlayAlpha(m_overlayHandle, &alpha);
	return alpha;
}

void OpenVROverlayController::SetWidth(float meters)
{
	vr::VROverlay()->SetOverlayWidthInMeters(m_overlayHandle, meters);
}

float OpenVROverlayController::GetWidth() const
{
	float meters;
	vr::VROverlay()->GetOverlayWidthInMeters(m_overlayHandle, &meters);
	return meters;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool OpenVROverlayController::ConnectToVRRuntime()
{
	m_hmdError = vr::VRInitError_None;
	m_VRSystem = vr::VR_Init( &m_hmdError, vr::VRApplication_Overlay );

	if ( m_hmdError != vr::VRInitError_None )
	{
		m_strVRDriver = "No Driver";
		m_strVRDisplay = "No Display";
		return false;
	}

	m_strVRDriver = GetTrackedDeviceString(m_VRSystem, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String);
	m_strVRDisplay = GetTrackedDeviceString(m_VRSystem, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String);

	return true;
}


void OpenVROverlayController::DisconnectFromVRRuntime()
{
	vr::VR_Shutdown();
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
QString OpenVROverlayController::GetVRDriverString()
{
	return m_strVRDriver;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
QString OpenVROverlayController::GetVRDisplayString()
{
	return m_strVRDisplay;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool OpenVROverlayController::BHMDAvailable()
{
	return vr::VRSystem() != nullptr;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
IVRSystem* OpenVROverlayController::GetVRSystem()
{
	return m_VRSystem;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------

vr::HmdError OpenVROverlayController::GetLastHmdError()
{
	return m_hmdError;
}

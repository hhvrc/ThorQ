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

inline void ToQMatrix(const vr::HmdMatrix34_t& mat, QMatrix4x4& out)
{
    for (int i = 0; i < 3; i++)
        out.setRow(i, QVector4D(mat.m[i][0], mat.m[i][1], mat.m[i][2], mat.m[i][3]));
}
inline void ToHmdMatrix34(const QMatrix4x4& mat, vr::HmdMatrix34_t& out)
{
	for (int i = 0; i < 3; i++)
	{
		QVector4D row = mat.row(i);
        out.m[i][0] = row.x();
        out.m[i][1] = row.y();
        out.m[i][2] = row.z();
        out.m[i][3] = row.w();
    }
}
constexpr float deg2rad = (float)M_PI / 180.f;
constexpr float rad2deg = 180.f / (float)M_PI;

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
	, m_isInitialized(false)
	, m_widget(nullptr)
	, m_pumpEventsTimer(new QTimer(this))
	, m_visibilityTimer(new QTimer(this))
	, m_ivrSystem(nullptr)
	, m_handle(vr::k_ulOverlayHandleInvalid)
	, m_deviceOffset()
    , m_L_deviceOffset()
    , m_R_deviceOffset()
	, m_deviceIndex(vr::k_unTrackedDeviceIndexInvalid)
	, m_scene(nullptr)
	, m_glContext(nullptr)
	, m_surface(nullptr)
	, m_frameBuffer(nullptr)
	, m_lastMousePoint()
	, m_lastMouseButtons(0)
{
	connect(m_pumpEventsTimer, &QTimer::timeout, this, &OpenVROverlayController::PollEvents);
	connect(m_visibilityTimer, &QTimer::timeout, [this](){ SetIsVisible(false); });

	m_pumpEventsTimer->setInterval(20);

	m_visibilityTimer->setInterval(5000);
	m_visibilityTimer->setSingleShot(true);

	// Calculate offset
    {
        QMatrix4x4 mat;
        mat.scale(0.25f);
        mat.rotate(90.f, -90.f, 90.f);
        mat.translate(-0.07f, -0.05f, 0.06f);
        mat.optimize();
        ToHmdMatrix34(mat, m_L_deviceOffset);
    }
    {
        QMatrix4x4 mat;
        mat.scale(0.25f);
        mat.rotate(-90.f, 90.f, 90.f);
        mat.translate(0.4f, -0.05f, 0.06f);
        mat.optimize();
        ToHmdMatrix34(mat, m_R_deviceOffset);
    }
    m_deviceOffset = &m_L_deviceOffset;
}

OpenVROverlayController::~OpenVROverlayController()
{
}

bool OpenVROverlayController::Init()
{
	if (m_isInitialized)
		return true;

	QSurfaceFormat format;
	format.setMajorVersion(4);
	format.setMinorVersion(1);
	format.setProfile(QSurfaceFormat::CompatibilityProfile);

	m_glContext = new QOpenGLContext();
	m_glContext->setFormat(format);
	if(!m_glContext->create())
		return false;

	// create an offscreen surface to attach the context and FBO to
	m_surface = new QOffscreenSurface();
	m_surface->create();
	m_glContext->makeCurrent(m_surface);

	m_scene = new QGraphicsScene();
	connect(m_scene, &QGraphicsScene::changed, this, &OpenVROverlayController::OverlayDraw);

	// Loading the OpenVR Runtime
	if (!ConnectToVRRuntime() || vr::VRCompositor() == nullptr || vr::VROverlay() == nullptr)
		return false;

	OverlayCreate();

	m_pumpEventsTimer->start();

	m_isInitialized = true;
	return true;
}
void OpenVROverlayController::Shutdown()
{
	if (!m_isInitialized)
		return;
	m_isInitialized = false;

	m_visibilityTimer->stop();
	m_pumpEventsTimer->stop();

	if (m_frameBuffer != nullptr)
	{
		delete m_frameBuffer;
		m_frameBuffer = nullptr;
	}

	DisconnectFromVRRuntime();

	m_handle = vr::k_ulOverlayHandleInvalid;

	if (m_scene != nullptr)
	{
		delete m_scene;
		m_scene = nullptr;
	}

	if (m_surface != nullptr)
	{
		delete m_surface;
		m_surface = nullptr;
	}

	if(m_glContext != nullptr)
	{
		delete m_glContext;
		m_glContext = nullptr;
	}
}

bool OpenVROverlayController::ConnectToVRRuntime()
{
	vr::EVRInitError err = vr::VRInitError_None;
	m_ivrSystem = vr::VR_Init(&err, vr::VRApplication_Overlay);
	return err == vr::VRInitError_None;
}
void OpenVROverlayController::DisconnectFromVRRuntime()
{
	vr::VR_Shutdown();
}

void OpenVROverlayController::SetWidget(QWidget* widget)
{
	if (!m_isInitialized)
		return;

	if (m_widget != widget)
	{
		m_widget = widget;

		// all of the mouse handling stuff requires that the widget be at 0,0
		m_widget->move(0, 0);

		m_scene->addWidget(m_widget);

		SetOverlayResolution(m_widget->width(), m_widget->height());

		OverlayProcess();
		emit WidgetChanged(widget);
	}
}
QWidget* OpenVROverlayController::GetWidget() const
{
	return m_widget;
}

void OpenVROverlayController::SetIsVisible(bool visible)
{
	if (m_handle == vr::k_ulOverlayHandleInvalid)
		return;

	if (m_isVisible != visible)
	{
		m_isVisible = visible;

		if (visible) {
			qDebug() << "Show";
			Dbg(vr::VROverlay()->ShowOverlay(m_handle), __LINE__);
		} else {
			qDebug() << "Hide";
			Dbg(vr::VROverlay()->HideOverlay(m_handle), __LINE__);
		}

		emit IsVisibleChanged(visible);
	}
}
bool OpenVROverlayController::GetIsVisible() const
{
	return m_isVisible;
}

void OpenVROverlayController::SetWidth(float width)
{
	if (m_handle == vr::k_ulOverlayHandleInvalid)
		return;

	if (m_width != width)
	{
		m_width = width;
		Dbg(vr::VROverlay()->SetOverlayWidthInMeters(m_handle, m_width), __LINE__);
		emit WidthChanged(width);
	}
}
float OpenVROverlayController::GetWidth() const
{
	return m_width;
}

void OpenVROverlayController::SetAlpha(float alpha)
{
	if (m_handle == vr::k_ulOverlayHandleInvalid)
		return;

	if (m_alpha != alpha)
	{
		m_alpha = alpha;
		Dbg(vr::VROverlay()->SetOverlayAlpha(m_handle, alpha), __LINE__);
		emit AlphaChanged(alpha);
	}
}
float OpenVROverlayController::GetAlpha() const
{
	return m_alpha;
}

void OpenVROverlayController::SetTint(const QColor& color)
{
	if (m_handle == vr::k_ulOverlayHandleInvalid)
		return;

	if (!color.isValid())
		return;

	if (m_tint != color)
	{
		m_tint = color;
		Dbg(vr::VROverlay()->SetOverlayColor(m_handle, m_tint.redF(), m_tint.greenF(), m_tint.blueF()), __LINE__);
		emit TintChanged(color);
	}
}
QColor OpenVROverlayController::GetTint() const
{
	return m_tint;
}

void OpenVROverlayController::ToggleIsVisible()
{
	SetIsVisible(!m_isVisible);
}
void OpenVROverlayController::SetIsVisibleTimeout(bool enabled, int msec)
{

}


void OpenVROverlayController::SetTrackedDevice(vr::TrackedDeviceIndex_t index)
{
	if (m_deviceIndex != index)
	{
		m_deviceIndex = index;
		OverlayTransform();
		emit
	}
}

vr::TrackedDeviceIndex_t OpenVROverlayController::GetTrackedDevice() const
{
	return m_deviceIndex;
}

void OpenVROverlayController::PollEvents()
{
	if(vr::VRSystem() == nullptr)
		return;

	vr::VREvent_t event;
	while (vr::VRSystem()->PollNextEvent(&event, sizeof(event)))
	{
		switch(event.eventType)
		{
		case vr::VREvent_ButtonPress:
		{
            vr::ETrackedDeviceClass devClass = vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex);

            if (devClass == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
            {
                bool isOculus = false;

                {
                    std::string buffer;
                    buffer.resize(256);
                    vr::ETrackedPropertyError err;
                    vr::VRSystem()->GetStringTrackedDeviceProperty(event.trackedDeviceIndex, vr::ETrackedDeviceProperty::Prop_TrackingSystemName_String, buffer.data(), 256, &err);
                    std::transform(buffer.begin(), buffer.end(), buffer.begin(), ::tolower);
                    isOculus = buffer.find("oculus") != std::string::npos;
                }

                // Oculus : B/Y, Bit 1, Mask 2
                // Oculus : A/X, Bit 7, Mask 128
                // Vive : Menu, Bit 1, Mask 2,
                // Vive : Grip, Bit 2, Mask 4
                vr::VRControllerState_t state;
                vr::VRSystem()->GetControllerState(event.trackedDeviceIndex, &state, sizeof(state));

                if ((state.ulButtonPressed & (isOculus ? 0x2 : 0x4)) != 0)
                {
                    if (GetIsVisible())
                    {
                        if (GetTrackedDevice() == event.trackedDeviceIndex)
                        {
                            SetIsVisible(false);
                            m_visibilityTimer->stop();
                        }
                        else
                        {
                            SetTrackedDevice(event.trackedDeviceIndex);
                        }
                    }
                    else
                    {
                        SetTrackedDevice(event.trackedDeviceIndex);
                        SetIsVisible(true);
                        m_visibilityTimer->start();
                    }
                }
            }
            break;
        }
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
			QPointF ptNewMouse(event.data.mouse.x, event.data.mouse.y);
			QPoint ptGlobal = ptNewMouse.toPoint();
			QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMouseMove);
			mouseEvent.setWidget(nullptr);
			mouseEvent.setPos(ptNewMouse);
			mouseEvent.setScenePos(ptGlobal);
			mouseEvent.setScreenPos(ptGlobal);
			mouseEvent.setLastPos(m_lastMousePoint);
			mouseEvent.setLastScenePos(m_widget->mapToGlobal(m_lastMousePoint.toPoint()));
			mouseEvent.setLastScreenPos(m_widget->mapToGlobal(m_lastMousePoint.toPoint()));
			mouseEvent.setButtons(m_lastMouseButtons);
			mouseEvent.setButton(Qt::NoButton);
			mouseEvent.setModifiers(0);
			mouseEvent.setAccepted(false);

			m_lastMousePoint = ptNewMouse;
			QApplication::sendEvent(m_scene, &mouseEvent);

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
			QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMousePress);
			mouseEvent.setWidget(nullptr);
			mouseEvent.setPos(m_lastMousePoint);
			mouseEvent.setButtonDownPos(button, m_lastMousePoint);
			mouseEvent.setButtonDownScenePos(button, ptGlobal);
			mouseEvent.setButtonDownScreenPos(button, ptGlobal);
			mouseEvent.setScenePos(ptGlobal);
			mouseEvent.setScreenPos(ptGlobal);
			mouseEvent.setLastPos(m_lastMousePoint);
			mouseEvent.setLastScenePos(ptGlobal);
			mouseEvent.setLastScreenPos(ptGlobal);
			mouseEvent.setButtons(m_lastMouseButtons);
			mouseEvent.setButton(button);
			mouseEvent.setModifiers(0);
			mouseEvent.setAccepted(false);

			QApplication::sendEvent(m_scene, &mouseEvent);
		}
			break;

		case vr::VREvent_MouseButtonUp:
		{
			qDebug() << "MouseUp!";
			Qt::MouseButton button = event.data.mouse.button == vr::VRMouseButton_Right ? Qt::RightButton : Qt::LeftButton;
			m_lastMouseButtons &= ~button;

			QPoint ptGlobal = m_lastMousePoint.toPoint();
			QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMouseRelease);
			mouseEvent.setWidget(NULL);
			mouseEvent.setPos(m_lastMousePoint);
			mouseEvent.setScenePos(ptGlobal);
			mouseEvent.setScreenPos(ptGlobal);
			mouseEvent.setLastPos(m_lastMousePoint);
			mouseEvent.setLastScenePos(ptGlobal);
			mouseEvent.setLastScreenPos(ptGlobal);
			mouseEvent.setButtons(m_lastMouseButtons);
			mouseEvent.setButton(button);
			mouseEvent.setModifiers(0);
			mouseEvent.setAccepted(false);

			QApplication::sendEvent(m_scene, &mouseEvent);
		}
			break;
		}
	}
}

void OpenVROverlayController::SetOverlayResolution(int width, int height)
{
	QOpenGLFramebufferObject* oldBuff = m_frameBuffer;

	m_frameBuffer = new QOpenGLFramebufferObject(m_widget->width(), m_widget->height(), GL_TEXTURE_2D);

	if (oldBuff != nullptr)
		delete oldBuff;

	vr::HmdVector2_t vecWindowSize = { float(width), float(height) };
	vr::VROverlay()->SetOverlayMouseScale(m_handle, &vecWindowSize);
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
	if (m_handle == vr::k_ulOverlayHandleInvalid)
		return;

	// Alpha
	m_alpha = 0.9f;
	Dbg(vr::VROverlay()->SetOverlayAlpha(m_handle, m_alpha), __LINE__);

	// Tint
	// Makes overlay black for some reason?
	//m_tint = QColor(0, 0, 0, 0);
	//Dbg(vr::VROverlay()->SetOverlayColor(m_handle, m_tint.redF(), m_tint.greenF(), m_tint.blueF()), __LINE__);

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
	if (m_handle == vr::k_ulOverlayHandleInvalid)
		OverlayCreate();

	// Offset overlay from device (most likely a controller)
	OverlayTransform();

	if (m_isVisible)
		OverlayDraw();
}

void OpenVROverlayController::OverlayDraw()
{
	if (m_handle == vr::k_ulOverlayHandleInvalid)
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
		Dbg(vr::VROverlay()->SetOverlayTexture(m_handle, &texture), __LINE__);
	}
}

void OpenVROverlayController::OverlayTransform()
{
	if (m_handle == vr::k_ulOverlayHandleInvalid)
	{
		qDebug() << "Handle is invalid" << __LINE__;
		return;
	}
	if (m_deviceIndex == vr::k_unTrackedDeviceIndexInvalid)
	{
		qDebug() << "Device is invalid" << __LINE__;
		return;
	}

	qDebug() << "Position";

    switch (vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(m_deviceIndex))
    {
    case vr::ETrackedControllerRole::TrackedControllerRole_LeftHand:
        m_deviceOffset = &m_L_deviceOffset;
        break;
    case vr::ETrackedControllerRole::TrackedControllerRole_RightHand:
        m_deviceOffset = &m_R_deviceOffset;
        break;
    default:
        return;
    }

	// Position
    Dbg(vr::VROverlay()->SetOverlayTransformTrackedDeviceRelative(m_handle, m_deviceIndex, m_deviceOffset), __LINE__);
}

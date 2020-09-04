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
#include <QDebug>
#include <QGraphicsProxyWidget>

#include "constants.h"

inline void ToQMatrix(const vr::HmdMatrix34_t& mat, QMatrix4x4& out)
{
    for (int i = 0; i < 3; i++)
		out.setColumn(i, QVector4D(mat.m[i][0], mat.m[i][1], mat.m[i][2], mat.m[i][3]));
}
inline void ToHmdMatrix34(const QMatrix4x4& mat, vr::HmdMatrix34_t& out)
{
	for (int i = 0; i < 3; i++)
	{
		QVector4D col = mat.column(i);
		out.m[i][0] = col.x();
		out.m[i][1] = col.y();
		out.m[i][2] = col.z();
		out.m[i][3] = col.w();
    }
}
// TODO: unused
constexpr float deg2rad = (float)M_PI / 180.f;
constexpr float rad2deg = 180.f / (float)M_PI;

// Static functions
bool OpenVROverlayController::IsSteamVRInstalled()
{
	return vr::VR_IsRuntimeInstalled();
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


OpenVROverlayController::OpenVROverlayController(QObject* parent)
	: QObject(parent)
	, m_isVisible(false)
	, m_alpha(0.f)
	, m_width(0.f)
	, m_tint()
	, m_timeoutEnabled(true)
	, m_proxyWidget(nullptr)
	, m_updateLogicTimer(new QTimer(this))
	, m_visibilityTimer(new QTimer(this))
	, m_system(nullptr)
	, m_overlay(vr::k_ulOverlayHandleInvalid)
    , m_windowSize()
    , m_controller_pri(vr::k_unTrackedDeviceIndexInvalid)
    , m_controller_sec(vr::k_unTrackedDeviceIndexInvalid)
    , m_overlay_offset(&m_overlay_offset_R)
	, m_overlay_offset_L()
	, m_overlay_offset_R()
	, m_overlay_offset_U()
	, m_scene(nullptr)
	, m_glContext(nullptr)
	, m_surface(nullptr)
	, m_frameBuffer(nullptr)
	, m_lastMousePoint()
	, m_lastMouseButtons(Qt::NoButton)
{
	connect(m_updateLogicTimer, &QTimer::timeout, this, &OpenVROverlayController::update);
	connect(m_visibilityTimer, &QTimer::timeout, this, &OpenVROverlayController::visibilityTimeoutExpired);

	m_updateLogicTimer->setInterval(20);

	m_visibilityTimer->setInterval(5000);
	m_visibilityTimer->setSingleShot(true);

	// Calculate offsets

	// Left controller
	m_overlay_offset_L.scale(0.25f);
    m_overlay_offset_L.rotate(90.f, 1.f, 0.f, 0.f);
    m_overlay_offset_L.translate(-0.4f, -0.05f, 0.06f);
	m_overlay_offset_L.optimize();

	// Right controller
	m_overlay_offset_R.scale(0.25f);
    m_overlay_offset_R.rotate(90.f, 1.f, 0.f, 0.f);
    m_overlay_offset_R.translate(0.4f, -0.05f, 0.06f);
	m_overlay_offset_R.optimize();

	// Unidirectional controller
	m_overlay_offset_U.scale(0.25f);
    m_overlay_offset_U.rotate(90.f, 1.f, 0.f, 0.f);
    m_overlay_offset_U.translate(0.f, -0.05f, 0.06f);
	m_overlay_offset_U.optimize();
}

OpenVROverlayController::~OpenVROverlayController()
{
	shutdown();
	delete m_frameBuffer;
}

bool OpenVROverlayController::init()
{
	if (m_system == nullptr)
	{
        vr::EVRInitError initErr;

        m_system = vr::VR_Init(&initErr, vr::VRApplication_Overlay);

		if (m_system == nullptr)
		{
			// TODO: handle err

			return false;
		}

        // Allow overlay to be interractable within vr
        vr::EVRSettingsError settingsError;
        vr::VRSettings()->SetBool(vr::k_pch_SteamVR_Section, vr::k_pch_SteamVR_AllowGlobalActionSetPriority, true, &settingsError);
        if (settingsError != vr::VRSettingsError_None)
            qDebug() << tr("Failed to enable global ActionSet priority, error:") << settingsError;

        /*
        vr::VRActionHandle_t actionHandle;
        vr::VRInput()->GetActionHandle(THORQ_APPLICATION_NAME, &actionHandle);

        vr::VRActiveActionSet_t actionSet;
        actionSet.ulActionSet = actionHandle;
        actionSet.nPriority = vr::k_nActionSetOverlayGlobalPriorityMin + 1;
        actionSet.ulRestrictedToDevice = vr::k_ulInvalidInputValueHandle;

        vr::VRInput()->UpdateActionState(&actionSet, sizeof(actionSet), 1);*/
	}

	if (!createOverlay())
	{
		return false;
	}

	if (m_glContext == nullptr)
	{
		QSurfaceFormat format;
		format.setMajorVersion(4);
		format.setMinorVersion(1);
        format.setProfile(QSurfaceFormat::CompatibilityProfile);

		m_glContext = new QOpenGLContext(this);
		m_glContext->setFormat(format);
		if(!m_glContext->create())
		{
			delete m_glContext;
			m_glContext = nullptr;

			return false;
		}
	}

	if (m_surface == nullptr)
	{
		// create an offscreen surface to attach the context and FBO to
		m_surface = new QOffscreenSurface();
		m_surface->create();
		m_glContext->makeCurrent(m_surface);
	}

	if (m_scene == nullptr)
	{
		m_scene = new QGraphicsScene(this);
		connect(m_scene, &QGraphicsScene::changed, this, &OpenVROverlayController::onSceneChanged);
	}

	if (!m_updateLogicTimer->isActive())
	{
		m_updateLogicTimer->start();
	}

	return true;
}
void OpenVROverlayController::shutdown()
{
	qDebug() << tr("Stopping timers");
	m_visibilityTimer->stop();
	m_updateLogicTimer->stop();

	m_scene->deleteLater();
	m_scene = nullptr;
	m_proxyWidget = nullptr;

	delete m_surface;
	m_surface = nullptr;

	m_glContext->deleteLater();
	m_glContext = nullptr;

	if (m_system != nullptr)
    {
        // Revert global settings
        vr::VRSettings()->SetBool(vr::k_pch_SteamVR_Section, vr::k_pch_SteamVR_AllowGlobalActionSetPriority, true);

		qDebug() << tr("Disconnecting VR runtime");
		m_system = nullptr;
		vr::VR_Shutdown();
	}
}

bool OpenVROverlayController::setWidget(QWidget* widget)
{
	if (m_scene == nullptr)
		return false;

	if (m_proxyWidget != nullptr)
	{
		if (m_proxyWidget->widget() != widget)
		{
			m_scene->removeItem(m_proxyWidget);
			delete m_proxyWidget;
		}
		else
		{
			return true;
		}
	}

    // Fixed some fuckery
    widget->resize(widget->sizeHint());

	// all of the mouse handling stuff requires that the widget be at 0,0
    widget->move(0, 0);

    m_proxyWidget = m_scene->addWidget(widget);

	delete m_frameBuffer;
	m_frameBuffer = new QOpenGLFramebufferObject(widget->size(), GL_TEXTURE_2D);

	emit widgetChanged(widget);

	return true;
}
QWidget* OpenVROverlayController::widget() const
{
	if (m_proxyWidget != nullptr)
		return m_proxyWidget->widget();
	return nullptr;
}

void OpenVROverlayController::setIsVisible(bool visible)
{
	if (m_overlay == vr::k_ulOverlayHandleInvalid)
		return;

	if (m_isVisible != visible)
	{
		m_isVisible = visible;

		vr::EVROverlayError err = vr::VROverlayError_None;

		if (visible)
		{
			qDebug() << tr("Show overlay");

			err = vr::VROverlay()->ShowOverlay(m_overlay);

			if (err != vr::VROverlayError_None)
			{
				qDebug() << tr("Error showing overlay:") << err;
				return;
			}

		}
		else
		{
			qDebug() << tr("Hide overlay");

			err = vr::VROverlay()->HideOverlay(m_overlay);

			if (err != vr::VROverlayError_None)
			{
				qDebug() << tr("Error hiding overlay:") << err;
				return;
			}
		}

		emit isVisibleChanged(visible);
	}
}
bool OpenVROverlayController::isVisible() const
{
	return m_isVisible;
}

void OpenVROverlayController::setWidth(float width)
{
	if (m_overlay == vr::k_ulOverlayHandleInvalid)
		return;

	if (m_width != width)
	{
		m_width = width;

		vr::EVROverlayError err = vr::VROverlay()->SetOverlayWidthInMeters(m_overlay, m_width);

		if (err != vr::VROverlayError_None)
		{
			qDebug() << tr("Error setting overlay width:") << err;
			return;
		}

		emit widthChanged(width);
	}
}
float OpenVROverlayController::width() const
{
	return m_width;
}

void OpenVROverlayController::setAlpha(float alpha)
{
	if (m_overlay == vr::k_ulOverlayHandleInvalid)
		return;

	if (m_alpha != alpha)
	{
		m_alpha = alpha;

		vr::EVROverlayError err = vr::VROverlay()->SetOverlayAlpha(m_overlay, alpha);

		if (err != vr::VROverlayError_None)
		{
			qDebug() << tr("Error setting overlay alpha:") << err;
			return;
		}

		emit alphaChanged(alpha);
	}
}
float OpenVROverlayController::alpha() const
{
	return m_alpha;
}

void OpenVROverlayController::setTint(const QColor& tint)
{
	if (m_overlay == vr::k_ulOverlayHandleInvalid)
		return;

	if (!tint.isValid())
		return;

	if (m_tint != tint)
	{
		m_tint = tint;

		vr::EVROverlayError err = vr::VROverlay()->SetOverlayColor(m_overlay, m_tint.redF(), m_tint.greenF(), m_tint.blueF());

		if (err != vr::VROverlayError_None)
		{
			qDebug() << tr("Error setting overlay tint:") << err;
			return;
		}

		emit tintChanged(tint);
	}
}
QColor OpenVROverlayController::tint() const
{
	return m_tint;
}

void OpenVROverlayController::setVisibilityTimeout(int timeout)
{
	if (m_visibilityTimer->interval() != timeout)
	{
		if (m_visibilityTimer->isActive())
		{
			m_visibilityTimer->stop();
			visibilityTimeoutExpired();
		}

		m_visibilityTimer->setInterval(timeout);

		emit visibilityTimeoutChanged(timeout);
	}
}
int OpenVROverlayController::visibilityTimeout() const
{
	return m_visibilityTimer->interval();
}

void OpenVROverlayController::setVisibilityTimeoutEnabled(bool enabled)
{
	if (m_timeoutEnabled != enabled)
	{
		m_timeoutEnabled = enabled;
		emit visibilityTimeoutEnabledChanged(enabled);
	}
}
bool OpenVROverlayController::visibilityTimeoutEnabled() const
{
	return m_timeoutEnabled;
}

void OpenVROverlayController::visibilityTimeoutExpired()
{
	if (m_timeoutEnabled)
	{
		setIsVisible(false);
	}
}

void OpenVROverlayController::update()
{
    if (m_proxyWidget == nullptr)
        return;

    init();

    if(vr::VRSystem() == nullptr)
        return;

    vr::VREvent_t event{};

    // Poll global events
    while (vr::VRSystem()->PollNextEvent(&event, sizeof(event)))
    {
        switch(event.eventType)
        {
            case vr::VREvent_ButtonPress:
            {
                vr::ETrackedDeviceClass devClass = vr::VRSystem()->GetTrackedDeviceClass(event.trackedDeviceIndex);

                if (devClass == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller)
                {
                    // Oculus: [31] ProximitySensor
                    // Oculus: [32] Joystick
                    // Oculus: [33] Trigger
                    // Oculus: [34] Grip
                    // Oculus: [07] A/X
                    // Oculus: [02] Grip
                    // Oculus: [01] B/Y

                    // Index:  [31] ProximitySensor
                    // Index:  [32] Joystick/Touchpad
                    // Index:  [33] Trigger
                    // Index:  [34] ????
                    // Index:  [07] ????
                    // Index:  [02] A/X/Grip
                    // Index:  [01] B/Y

                    // Vive : Menu, Bit 1, Mask 2,
                    // Vive : Grip, Bit 2, Mask 4
                    vr::VRControllerState_t state;
                    vr::VRSystem()->GetControllerState(event.trackedDeviceIndex, &state, sizeof(state));

                    if ((state.ulButtonPressed & (isOculus(event.trackedDeviceIndex) ? 0x2 : 0x4)) != 0)
                    {
                        //vr::VROverlay()->ComputeOverlayIntersection(m_overlay, vr::VROverlayIntersectionParams_t)
                        if (isVisible())
                        {
                            if (m_controller_pri == event.trackedDeviceIndex)
                            {
                                setIsVisible(false);
                                m_visibilityTimer->stop();
                            }
                            else
                            {
                                setPriController(event.trackedDeviceIndex);
                            }
                        }
                        else
                        {
                            setPriController(event.trackedDeviceIndex);
                            setIsVisible(true);
                            m_visibilityTimer->start();
                        }
                    }
                }
                break;
            }
            case vr::VREvent_OverlayShown:
            {
                m_proxyWidget->widget()->repaint();
                break;
            }
            case vr::VREvent_Quit:
            {
                emit vrQuit();
                shutdown();
                return;
            }
        }
    }

    // Poll overlay events
    while(vr::VROverlay()->PollNextOverlayEvent(m_overlay, &event, sizeof(event)))
    {
        switch(event.eventType)
        {
        case vr::VREvent_MouseMove:
        {
            qDebug() << tr("Mouse moved");
            QPointF ptNewMouse(event.data.mouse.x, event.data.mouse.y);
            QPoint ptGlobal = ptNewMouse.toPoint();
            QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMouseMove);
            mouseEvent.setWidget(nullptr);
            mouseEvent.setPos(ptNewMouse);
            mouseEvent.setScenePos(ptGlobal);
            mouseEvent.setScreenPos(ptGlobal);
            mouseEvent.setLastPos(m_lastMousePoint);
            mouseEvent.setLastScenePos(m_proxyWidget->widget()->mapToGlobal(m_lastMousePoint.toPoint()));
            mouseEvent.setLastScreenPos(m_proxyWidget->widget()->mapToGlobal(m_lastMousePoint.toPoint()));
            mouseEvent.setButtons(m_lastMouseButtons);
            mouseEvent.setButton(Qt::NoButton);
            mouseEvent.setModifiers(Qt::NoModifier);
            mouseEvent.setAccepted(false);

            m_lastMousePoint = ptNewMouse;
            QApplication::sendEvent(m_scene, &mouseEvent);
            break;
        }
        case vr::VREvent_MouseButtonDown:
        {
            qDebug() << tr("Mouse press");
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
            mouseEvent.setModifiers(Qt::NoModifier);
            mouseEvent.setAccepted(false);

            QApplication::sendEvent(m_scene, &mouseEvent);
            break;
        }
        case vr::VREvent_MouseButtonUp:
        {
            qDebug() << tr("Mouse release");
            Qt::MouseButton button = event.data.mouse.button == vr::VRMouseButton_Right ? Qt::RightButton : Qt::LeftButton;
            m_lastMouseButtons &= ~button;

            QPoint ptGlobal = m_lastMousePoint.toPoint();
            QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMouseRelease);
            mouseEvent.setWidget(nullptr);
            mouseEvent.setPos(m_lastMousePoint);
            mouseEvent.setScenePos(ptGlobal);
            mouseEvent.setScreenPos(ptGlobal);
            mouseEvent.setLastPos(m_lastMousePoint);
            mouseEvent.setLastScenePos(ptGlobal);
            mouseEvent.setLastScreenPos(ptGlobal);
            mouseEvent.setButtons(m_lastMouseButtons);
            mouseEvent.setButton(button);
            mouseEvent.setModifiers(Qt::NoModifier);
            mouseEvent.setAccepted(false);

            QApplication::sendEvent(m_scene, &mouseEvent);
            break;
        }
        }
    }
}

// Requires: vr::VROverlay()
bool OpenVROverlayController::createOverlay()
{
	if (m_overlay != vr::k_ulOverlayHandleInvalid)
	{
        return true;
	}

	qDebug() << tr("Finding overlay");
	vr::EVROverlayError err = vr::VROverlay()->FindOverlay(THORQ_APPLICATION_NAME, &m_overlay);
	if (err == vr::VROverlayError_None)
	{
		qDebug() << tr("Found overlay");
		return true;
	}
	else if (err != vr::VROverlayError_UnknownOverlay)
	{
		m_overlay = vr::k_ulOverlayHandleInvalid;
		qDebug() << tr("Error finding overlay:") << err;
		return false;
	}



	qDebug() << tr("Creating overlay");
	err = vr::VROverlay()->CreateOverlay(THORQ_APPLICATION_NAME, THORQ_APPLICATION_NAME, &m_overlay);
	if (err != vr::VROverlayError_None)
	{
		m_overlay = vr::k_ulOverlayHandleInvalid;
		qDebug() << tr("Error creating overlay:") << err;
		return false;
	}



	// Alpha
	m_alpha = 0.9f;
	err = vr::VROverlay()->SetOverlayAlpha(m_overlay, m_alpha);
	if (err != vr::VROverlayError_None)
		qDebug() << tr("Error setting overlay alpha:") << err;

	// Visibility
	m_isVisible = false;
	err = vr::VROverlay()->HideOverlay(m_overlay);
	if (err != vr::VROverlayError_None)
		qDebug() << tr("Error setting overlay visibility:") << err;

	// Input method
	err = vr::VROverlay()->SetOverlayInputMethod(m_overlay, vr::VROverlayInputMethod_Mouse);
	if (err != vr::VROverlayError_None)
        qDebug() << tr("Error setting overlay input method:") << err;

    // Interaction
    //err = vr::VROverlay()->SetOverlayFlag(m_overlay, vr::VROverlayFlags_MakeOverlaysInteractiveIfVisible, true);
    //if (err != vr::VROverlayError_None)
    //    qDebug() << tr("Error setting overlay input method:") << err;

	return true;
}

// Requires: m_glContext, m_surface, m_frameBuffer, m_scene, vr::VROverlay(), m_overlay
void OpenVROverlayController::onSceneChanged()
{
	if (!createOverlay() || m_frameBuffer == nullptr)
	{
		return;
    }

    qDebug() << tr("Drawing overlay");

    QWidget* widget = m_proxyWidget->widget();
    QSize newSize = widget->sizeHint();

    if (m_frameBuffer->size() != newSize)
    {
        widget->resize(newSize);
        widget->move(0, 0);

        delete m_frameBuffer;
        m_frameBuffer = new QOpenGLFramebufferObject(newSize, GL_TEXTURE_2D);
    }

    m_glContext->makeCurrent(m_surface);
    m_frameBuffer->bind();

    qDebug() << "Size:" << m_frameBuffer->width() << m_frameBuffer->height();

    QOpenGLPaintDevice device(m_frameBuffer->size());
    QPainter painter(&device);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::HighQualityAntialiasing);

	m_scene->render(&painter);

	m_frameBuffer->release();

	// Get framebuffer id
	GLuint glTexture = m_frameBuffer->texture();
	if (glTexture != 0)
	{
		vr::Texture_t texture;

		texture.handle = (void*)(std::uintptr_t)glTexture;
		texture.eType = vr::TextureType_OpenGL;
		texture.eColorSpace = vr::ColorSpace_Auto;

		// Give framebuffer id to OpenVR
		vr::EVROverlayError err = vr::VROverlay()->SetOverlayTexture(m_overlay, &texture);
		if (err != vr::VROverlayError_None)
			qDebug() << tr("Error setting overlay texture:") << err;
	}
}

// Requires: vr::VROverlay(), m_overlay, m_primary_controller_index, m_overlay_offset
void OpenVROverlayController::overlayTransform()
{
	if (!createOverlay())
	{
		return;
	}

	qDebug() << tr("Transforming overlay");

	vr::HmdMatrix34_t offset;

    ToHmdMatrix34(*m_overlay_offset, offset);

	// Position
    vr::EVROverlayError err = vr::VROverlay()->SetOverlayTransformTrackedDeviceRelative(m_overlay, m_controller_pri, &offset);

	if (err != vr::VROverlayError_None)
        qDebug() << tr("Error transforming overlay:") << err;
}

void OpenVROverlayController::setPriController(vr::TrackedDeviceIndex_t index)
{
    if (m_controller_pri == index)
        return;

    switch (m_system->GetControllerRoleForTrackedDeviceIndex(index))
    {
    case vr::TrackedControllerRole_LeftHand:
        m_overlay_offset = &m_overlay_offset_L;
        break;
    case vr::TrackedControllerRole_RightHand:
        m_overlay_offset = &m_overlay_offset_R;
        break;
    case vr::TrackedControllerRole_Invalid:
        m_overlay_offset = &m_overlay_offset_U;
        break;
    default:
        return;
    }

    m_controller_sec = m_controller_pri;
    m_controller_pri = index;

    overlayTransform();
}

bool OpenVROverlayController::isOculus(vr::TrackedDeviceIndex_t index) const
{
	std::string buffer;
	buffer.resize(256);
	vr::ETrackedPropertyError err;
    vr::VRSystem()->GetStringTrackedDeviceProperty(index, vr::ETrackedDeviceProperty::Prop_TrackingSystemName_String, buffer.data(), 256, &err);

	// TODO: handle err

	std::transform(buffer.begin(), buffer.end(), buffer.begin(), ::tolower);
	return buffer.find("oculus") != std::string::npos;
}

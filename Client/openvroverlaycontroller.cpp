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
#include <QDir>
#include <QFile>
#include <QApplication>
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
	, m_vrSystem(nullptr)
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
    QDir dir(QApplication::applicationDirPath());

	dir.mkdir("action_manifest");
	if (!dir.cd("action_manifest"))
	{
		qWarning() << "Cant create folder for vr actions!";
		exit(EXIT_FAILURE);
	}

	QDirIterator it(":/action_manifest/");
	while (it.hasNext()) {
		QString internalName = it.next();
		if (!dir.exists(it.fileName()))
		{
			QFile::copy(internalName, dir.filePath(it.fileName()));
		}
    }

    m_vrManifestPath = dir.filePath("manifest.json").toStdString();

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
	if (m_vrSystem == nullptr)
	{
        vr::EVRInitError initErr;

        m_vrSystem = vr::VR_Init(&initErr, vr::VRApplication_Overlay);

		if (m_vrSystem == nullptr)
		{
            qWarning() << tr("Failed to initialize OpenVR:") << vr::VR_GetVRInitErrorAsEnglishDescription(initErr);
			return false;
		}

		m_vrInput = vr::VRInput();
		m_vrOverlay = vr::VROverlay();
        m_vrSettings = vr::VRSettings();

        // Allow overlay to be interractable within vr
        vr::EVRSettingsError settingsError;
		m_vrSettings->SetBool(vr::k_pch_SteamVR_Section, vr::k_pch_SteamVR_AllowGlobalActionSetPriority, true, &settingsError);
        if (settingsError != vr::VRSettingsError_None)
        {
			qWarning() << tr("Failed to enable global ActionSet priority:") << m_vrSettings->GetSettingsErrorNameFromEnum(settingsError);
            return false;
		}

        qDebug() << "Setting vrmanifest:" << m_vrManifestPath.c_str();

        vr::EVRInputError inputError = m_vrInput->SetActionManifestPath(m_vrManifestPath.c_str());
        if (inputError != vr::VRInputError_None)
        {
            qWarning() << tr("Failed to set VrManifest file:") << inputError;
            return false;
        }

        // Set priority
		vr::VRActionSetHandle_t actionSetHandle;
        inputError = m_vrInput->GetActionSetHandle("ui", &actionSetHandle);
        if (inputError != vr::VRInputError_None)
        {
            qWarning() << tr("Failed to get actionSetHandle:") << inputError;
            return false;
        }

		vr::VRActiveActionSet_t actionSet;
		actionSet.nPriority = vr::k_nActionSetOverlayGlobalPriorityMin + 1;
		actionSet.ulSecondaryActionSet = vr::k_ulInvalidInputValueHandle;
		actionSet.ulActionSet = actionSetHandle;

        inputError = m_vrInput->UpdateActionState(&actionSet, sizeof(actionSet), 1);
        if (inputError != vr::VRInputError_None)
        {
            qWarning() << tr("Failed to set actionSet:") << inputError;
            return false;
        }
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

	if (m_vrSystem != nullptr)
    {
        // Revert global settings
		m_vrSettings->SetBool(vr::k_pch_SteamVR_Section, vr::k_pch_SteamVR_AllowGlobalActionSetPriority, true);

		qDebug() << tr("Disconnecting VR runtime");
		m_vrSystem = nullptr;
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

			err = m_vrOverlay->ShowOverlay(m_overlay);

			if (err != vr::VROverlayError_None)
			{
				qWarning() << tr("Error showing overlay:") << m_vrOverlay->GetOverlayErrorNameFromEnum(err);
				return;
			}

		}
		else
		{
			qDebug() << tr("Hide overlay");

			err = m_vrOverlay->HideOverlay(m_overlay);

			if (err != vr::VROverlayError_None)
			{
				qWarning() << tr("Error hiding overlay:") << m_vrOverlay->GetOverlayErrorNameFromEnum(err);
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

		vr::EVROverlayError err = m_vrOverlay->SetOverlayWidthInMeters(m_overlay, m_width);

		if (err != vr::VROverlayError_None)
		{
			qWarning() << tr("Error setting overlay width:") << m_vrOverlay->GetOverlayErrorNameFromEnum(err);
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

		vr::EVROverlayError err = m_vrOverlay->SetOverlayAlpha(m_overlay, alpha);

		if (err != vr::VROverlayError_None)
		{
			qWarning() << tr("Error setting overlay alpha:") << m_vrOverlay->GetOverlayErrorNameFromEnum(err);
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

		vr::EVROverlayError err = m_vrOverlay->SetOverlayColor(m_overlay, m_tint.redF(), m_tint.greenF(), m_tint.blueF());

		if (err != vr::VROverlayError_None)
		{
			qWarning() << tr("Error setting overlay tint:") << m_vrOverlay->GetOverlayErrorNameFromEnum(err);
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
	while (m_vrSystem->PollNextEvent(&event, sizeof(event)))
    {
        switch(event.eventType)
        {
            case vr::VREvent_TrackedDeviceUserInteractionStarted:
            {
                qDebug() << "Start!";
                break;
            }
            case vr::VREvent_TrackedDeviceUserInteractionEnded:
            {
                qDebug() << "End!";
                break;
            }
            case vr::VREvent_ButtonPress:
            {
				vr::ETrackedDeviceClass devClass = m_vrSystem->GetTrackedDeviceClass(event.trackedDeviceIndex);

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
                    if ((event.data.controller.button & (isOculus(event.trackedDeviceIndex) ? 0x7 : 0x1)) != 0)
                    {
						//m_vrOverlay->ComputeOverlayIntersection(m_overlay, vr::VROverlayIntersectionParams_t)
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
	while(m_vrOverlay->PollNextOverlayEvent(m_overlay, &event, sizeof(event)))
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

// Requires: m_vrOverlay
bool OpenVROverlayController::createOverlay()
{
	if (m_overlay != vr::k_ulOverlayHandleInvalid)
	{
        return true;
	}

	qDebug() << tr("Finding overlay");
	vr::EVROverlayError err = m_vrOverlay->FindOverlay(THORQ_APPLICATION_NAME, &m_overlay);
	if (err == vr::VROverlayError_None)
	{
		qDebug() << tr("Found overlay");
		return true;
	}
	else if (err != vr::VROverlayError_UnknownOverlay)
	{
		m_overlay = vr::k_ulOverlayHandleInvalid;
		qWarning() << tr("Error finding overlay:") << m_vrOverlay->GetOverlayErrorNameFromEnum(err);
		return false;
	}



	qDebug() << tr("Creating overlay");
	err = m_vrOverlay->CreateOverlay(THORQ_APPLICATION_NAME, THORQ_APPLICATION_NAME, &m_overlay);
	if (err != vr::VROverlayError_None)
	{
		m_overlay = vr::k_ulOverlayHandleInvalid;
		qWarning() << tr("Error creating overlay:") << m_vrOverlay->GetOverlayErrorNameFromEnum(err);
		return false;
	}



	// Alpha
	m_alpha = 0.9f;
	err = m_vrOverlay->SetOverlayAlpha(m_overlay, m_alpha);
	if (err != vr::VROverlayError_None)
		qWarning() << tr("Error setting overlay alpha:") << m_vrOverlay->GetOverlayErrorNameFromEnum(err);

	// Visibility
	m_isVisible = false;
	err = m_vrOverlay->HideOverlay(m_overlay);
	if (err != vr::VROverlayError_None)
		qWarning() << tr("Error setting overlay visibility:") << m_vrOverlay->GetOverlayErrorNameFromEnum(err);

	// Input method
	err = m_vrOverlay->SetOverlayInputMethod(m_overlay, vr::VROverlayInputMethod_Mouse);
	if (err != vr::VROverlayError_None)
		qWarning() << tr("Error setting overlay input method:") << m_vrOverlay->GetOverlayErrorNameFromEnum(err);

    // Interaction
	err = m_vrOverlay->SetOverlayFlag(m_overlay, vr::VROverlayFlags_MakeOverlaysInteractiveIfVisible, true);
    if (err != vr::VROverlayError_None)
		qWarning() << tr("Error setting overlay input method:") << m_vrOverlay->GetOverlayErrorNameFromEnum(err);

	return true;
}

// Requires: m_glContext, m_surface, m_frameBuffer, m_scene, m_vrOverlay, m_overlay
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
		vr::EVROverlayError err = m_vrOverlay->SetOverlayTexture(m_overlay, &texture);
		if (err != vr::VROverlayError_None)
			qWarning() << tr("Error setting overlay texture:") << m_vrOverlay->GetOverlayErrorNameFromEnum(err);
	}
}

// Requires: m_vrOverlay, m_overlay, m_primary_controller_index, m_overlay_offset
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
	vr::EVROverlayError err = m_vrOverlay->SetOverlayTransformTrackedDeviceRelative(m_overlay, m_controller_pri, &offset);

	if (err != vr::VROverlayError_None)
		qWarning() << tr("Error transforming overlay:") << m_vrOverlay->GetOverlayErrorNameFromEnum(err);
}

void OpenVROverlayController::setPriController(vr::TrackedDeviceIndex_t index)
{
    if (m_controller_pri == index)
	{
        return;
	}

	switch (m_vrSystem->GetControllerRoleForTrackedDeviceIndex(index))
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
	char buffer[256];
	memset(buffer, 0, 256);

	vr::ETrackedPropertyError err;
	m_vrSystem->GetStringTrackedDeviceProperty(index, vr::ETrackedDeviceProperty::Prop_TrackingSystemName_String, buffer, 256, &err);

	if (err != vr::TrackedProp_Success)
	{
		qWarning() << tr("Error getting tracking system name:") << m_vrSystem->GetPropErrorNameFromEnum(err);
		return false;
	}

	std::transform(buffer, buffer + 256, buffer, ::tolower);

	return strstr(buffer, "oculus") != nullptr;
}

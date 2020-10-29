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

	, m_proxyWidget(nullptr)
    , m_updateLogicTimer(new QTimer(this))

    , m_apiSystem(nullptr)
    , m_apiInput(nullptr)
    , m_apiOverlay(nullptr)
    , m_apiSettings(nullptr)

    , m_overlayHandle(vr::k_ulOverlayHandleInvalid)
    , m_overlayDevice(vr::k_unTrackedDeviceIndexInvalid)
    , m_overlayDeviceOffsetL()
    , m_overlayDeviceOffsetR()
    , m_overlayDeviceOffsetC()

    , m_mouseHand(EHand::Invalid)
    , m_overlayHand(EHand::Invalid)
    , m_mouseDeviceIndex(vr::k_unTrackedDeviceIndexInvalid)

    , m_activeActionSet()
    , m_handleActionSet(vr::k_ulInvalidActionSetHandle)
    , m_handleActionHapticsLeft(vr::k_ulInvalidActionHandle)
    , m_handleActionHapticsRight(vr::k_ulInvalidActionHandle)
    , m_handleActionInteract(vr::k_ulInvalidActionHandle)
    , m_handleActionShowOverlay(vr::k_ulInvalidActionHandle)
    , m_handleActionProxSensor(vr::k_ulInvalidActionHandle)
    , m_sourceHMD(vr::k_ulInvalidInputValueHandle)
    , m_sourceControllerLeft(vr::k_ulInvalidInputValueHandle)
    , m_sourceControllerRight(vr::k_ulInvalidInputValueHandle)

	, m_scene(nullptr)
	, m_glContext(nullptr)
	, m_surface(nullptr)
	, m_frameBuffer(nullptr)

	, m_lastMousePoint()
	, m_lastMouseButtons(Qt::NoButton)

    , m_vrManifestPath()
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
    m_updateLogicTimer->setInterval(20);

    // Left controller offset
    m_overlayDeviceOffsetL.scale(0.25f);
    m_overlayDeviceOffsetL.rotate(90.f, 1.f, 0.f, 0.f);
    m_overlayDeviceOffsetL.translate(-0.4f, -0.05f, 0.06f);
    m_overlayDeviceOffsetL.optimize();

    // Right controller offset
    m_overlayDeviceOffsetR.scale(0.25f);
    m_overlayDeviceOffsetR.rotate(90.f, 1.f, 0.f, 0.f);
    m_overlayDeviceOffsetR.translate(0.4f, -0.05f, 0.06f);
    m_overlayDeviceOffsetR.optimize();

    // Unidirectional controller offset
    m_overlayDeviceOffsetC.scale(0.25f);
    m_overlayDeviceOffsetC.rotate(90.f, 1.f, 0.f, 0.f);
    m_overlayDeviceOffsetC.translate(0.f, -0.05f, 0.06f);
    m_overlayDeviceOffsetC.optimize();
}

OpenVROverlayController::~OpenVROverlayController()
{
	shutdown();
	delete m_frameBuffer;
}

bool OpenVROverlayController::init()
{
    if (m_apiSystem == nullptr)
	{
        vr::EVRInitError initErr;

        m_apiSystem = vr::VR_Init(&initErr, vr::VRApplication_Overlay);

        if (m_apiSystem == nullptr)
		{
            qWarning() << tr("Failed to initialize OpenVR:") << vr::VR_GetVRInitErrorAsEnglishDescription(initErr);
			return false;
		}

        m_apiInput = vr::VRInput();
        m_apiOverlay = vr::VROverlay();
        m_apiSettings = vr::VRSettings();

        // Allow overlay to be interractable within vr (sets a setting in steamVR)
        vr::EVRSettingsError settingsError;
        m_apiSettings->SetBool(vr::k_pch_SteamVR_Section, vr::k_pch_SteamVR_AllowGlobalActionSetPriority, true, &settingsError);
        if (settingsError != vr::VRSettingsError_None)
        {
            qWarning() << tr("Failed to enable global ActionSet priority:") << m_apiSettings->GetSettingsErrorNameFromEnum(settingsError);
            return false;
		}

        qDebug() << "Setting vrmanifest:" << m_vrManifestPath.c_str();

        // Set manifest path
        vr::EVRInputError inputError = m_apiInput->SetActionManifestPath(m_vrManifestPath.c_str());
        if (inputError != vr::VRInputError_None)
        {
            qWarning() << tr("Failed to set VrManifest file:") << inputError;
            return false;
        }

        // Get set handle
        inputError = m_apiInput->GetActionSetHandle("/actions/ui", &m_handleActionSet);
        if (inputError != vr::VRInputError_None)
        {
            qWarning() << tr("Failed to get actionSetHandle:") << inputError;
            return false;
        }

        // Get sources
        vr::VRInput()->GetInputSourceHandle("/user/head", &m_sourceHMD);
        if (inputError != vr::VRInputError_None)
        {
            qWarning() << tr("Failed to get head action handle:") << inputError;
            return false;
        }
        vr::VRInput()->GetInputSourceHandle("/user/hand/left", &m_sourceControllerLeft);
        if (inputError != vr::VRInputError_None)
        {
            qWarning() << tr("Failed to get hand_left action handle:") << inputError;
            return false;
        }
        vr::VRInput()->GetInputSourceHandle("/user/hand/right", &m_sourceControllerRight);
        if (inputError != vr::VRInputError_None)
        {
            qWarning() << tr("Failed to get hand_right action handle:") << inputError;
            return false;
        }

        // Get action handles
        vr::VRInput()->GetActionHandle("/actions/ui/out/haptics_left", &m_handleActionHapticsLeft);
        if (inputError != vr::VRInputError_None)
        {
            qWarning() << tr("Failed to get haptics_left action handle:") << inputError;
            return false;
        }
        vr::VRInput()->GetActionHandle("/actions/ui/out/haptics_right", &m_handleActionHapticsRight);
        if (inputError != vr::VRInputError_None)
        {
            qWarning() << tr("Failed to get haptics_right action handle:") << inputError;
            return false;
        }
        vr::VRInput()->GetActionHandle("/actions/ui/in/interact", &m_handleActionInteract);
        if (inputError != vr::VRInputError_None)
        {
            qWarning() << tr("Failed to get interact action handle:") << inputError;
            return false;
        }
        vr::VRInput()->GetActionHandle("/actions/ui/in/show_overlay", &m_handleActionShowOverlay);
        if (inputError != vr::VRInputError_None)
        {
            qWarning() << tr("Failed to get show_overlay action handle:") << inputError;
            return false;
        }
        vr::VRInput()->GetActionHandle("/actions/ui/in/prox_sensor", &m_handleActionShowOverlay);
        if (inputError != vr::VRInputError_None)
        {
            qWarning() << tr("Failed to get show_overlay action handle:") << inputError;
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
	m_updateLogicTimer->stop();

	m_scene->deleteLater();
	m_scene = nullptr;
	m_proxyWidget = nullptr;

	delete m_surface;
	m_surface = nullptr;

	m_glContext->deleteLater();
	m_glContext = nullptr;

    if (m_apiSystem != nullptr)
    {
        // Revert global settings
        m_apiSettings->SetBool(vr::k_pch_SteamVR_Section, vr::k_pch_SteamVR_AllowGlobalActionSetPriority, true);

		qDebug() << tr("Disconnecting VR runtime");
        m_apiSystem = nullptr;
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
    if (m_overlayHandle == vr::k_ulOverlayHandleInvalid)
		return;

	if (m_isVisible != visible)
	{
		m_isVisible = visible;

		vr::EVROverlayError err = vr::VROverlayError_None;

		if (visible)
		{
			qDebug() << tr("Show overlay");

            err = m_apiOverlay->ShowOverlay(m_overlayHandle);

			if (err != vr::VROverlayError_None)
			{
                qWarning() << tr("Error showing overlay:") << m_apiOverlay->GetOverlayErrorNameFromEnum(err);
				return;
			}

		}
		else
		{
			qDebug() << tr("Hide overlay");

            err = m_apiOverlay->HideOverlay(m_overlayHandle);

			if (err != vr::VROverlayError_None)
			{
                qWarning() << tr("Error hiding overlay:") << m_apiOverlay->GetOverlayErrorNameFromEnum(err);
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
    if (m_overlayHandle == vr::k_ulOverlayHandleInvalid)
		return;

	if (m_width != width)
	{
		m_width = width;

        vr::EVROverlayError err = m_apiOverlay->SetOverlayWidthInMeters(m_overlayHandle, m_width);

		if (err != vr::VROverlayError_None)
		{
            qWarning() << tr("Error setting overlay width:") << m_apiOverlay->GetOverlayErrorNameFromEnum(err);
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
    if (m_overlayHandle == vr::k_ulOverlayHandleInvalid)
		return;

	if (m_alpha != alpha)
	{
		m_alpha = alpha;

        vr::EVROverlayError err = m_apiOverlay->SetOverlayAlpha(m_overlayHandle, alpha);

		if (err != vr::VROverlayError_None)
		{
            qWarning() << tr("Error setting overlay alpha:") << m_apiOverlay->GetOverlayErrorNameFromEnum(err);
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
    if (m_overlayHandle == vr::k_ulOverlayHandleInvalid)
		return;

	if (!tint.isValid())
		return;

	if (m_tint != tint)
	{
		m_tint = tint;

        vr::EVROverlayError err = m_apiOverlay->SetOverlayColor(m_overlayHandle, m_tint.redF(), m_tint.greenF(), m_tint.blueF());

		if (err != vr::VROverlayError_None)
		{
            qWarning() << tr("Error setting overlay tint:") << m_apiOverlay->GetOverlayErrorNameFromEnum(err);
			return;
		}

		emit tintChanged(tint);
    }
}

bool OpenVROverlayController::openBindingUI()
{
    return m_apiInput->ShowActionOrigins(m_handleActionSet, m_handleActionInteract) == vr::VRInputError_None;
}

QColor OpenVROverlayController::tint() const
{
    return m_tint;
}

bool OpenVROverlayController::triggerHapticFeedback(OpenVROverlayController::EHand hand, float secondsFromNow, float amplitude, float frequency, float duration)
{
    vr::VRActionHandle_t action;
    vr::VRInputValueHandle_t origin;

    if (hand == EHand::Left)
    {
        action = m_handleActionHapticsLeft;
        origin = m_sourceControllerLeft;
    }
    else if (hand == EHand::Right)
    {
        action = m_handleActionHapticsRight;
        origin = m_sourceControllerRight;
    }
    else
    {
        return false;
    }

    vr::EVRInputError err = vr::VRInput()->TriggerHapticVibrationAction(action, secondsFromNow, duration, frequency, amplitude, origin);

    if (err == vr::VRInputError_None)
    {
        return true;
    }

    qWarning() << tr("Error triggering haptic:") << err;

    return false;
}

void OpenVROverlayController::update()
{
    init();

    if (!pullEvents())
    {
        qDebug() << "Error pulling events!";
        return;
    }

    vr::InputDigitalActionData_t interactionState, showOverlayState;
    m_apiInput->GetDigitalActionData( m_handleActionInteract, &interactionState, sizeof(vr::InputDigitalActionData_t), vr::k_ulInvalidActionHandle );
    m_apiInput->GetDigitalActionData( m_handleActionShowOverlay, &showOverlayState, sizeof(vr::InputDigitalActionData_t), vr::k_ulInvalidActionHandle );

    if ( showOverlayState.bChanged )
    {
        if ( showOverlayState.bActive )
        {
            auto device = getDeviceForSource( showOverlayState.activeOrigin );

            if ( device != vr::k_unTrackedDeviceIndexInvalid )
            {
                setOverlayDevice( device );
                setOverlayOffset( getOffsetForSource( showOverlayState.activeOrigin ) );
                setIsVisible( true );
            }
        }
        else
        {
            setIsVisible( false );
        }
    }

    if (interactionState.bChanged)
    {
        if (interactionState.bActive)
        {
            triggerHapticFeedback(getHandForSource(interactionState.activeOrigin), 0.f, 1.f, 500.f, 1.f);
        }
        else
        {
            triggerHapticFeedback(getHandForSource(interactionState.activeOrigin), 0.f, 1.f, 250.f, 0.5f);
        }
    }

    /*
    vr::InputAnalogActionData_t headData, leftHandData, rightHandData;
    m_vrInput->GetAnalogActionData(m_handleHead, &headData, sizeof(vr::InputAnalogActionData_t), m_handleHead);
    m_vrInput->GetAnalogActionData(m_handleHandLeft, &leftHandData, sizeof(vr::InputAnalogActionData_t), m_handleHandLeft);
    m_vrInput->GetAnalogActionData(m_handleHandRight, &rightHandData, sizeof(vr::InputAnalogActionData_t), m_handleHandRight);
    */

    // Links to check out:
    //
    // https://github.com/ValveSoftware/openvr/wiki/IVROverlay::HandleControllerOverlayInteractionAsMouse
    // https://github.com/aardvarkxr/aardvark/blob/57ab9a76a186ea8150f02393f1c210bfcb5ff931/data/aardvark.vrmanifest



    vr::VREvent_t event{};

    // Poll global events
    while (m_apiSystem->PollNextEvent(&event, sizeof(event)))
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
    while(m_apiOverlay->PollNextOverlayEvent(m_overlayHandle, &event, sizeof(event)))
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

bool OpenVROverlayController::pullEvents()
{
    m_activeActionSet.ulActionSet = m_handleActionSet;
    m_activeActionSet.ulRestrictedToDevice = vr::k_ulInvalidInputValueHandle;
    m_activeActionSet.ulSecondaryActionSet = vr::k_ulInvalidInputValueHandle;
    m_activeActionSet.nPriority = vr::k_nActionSetOverlayGlobalPriorityMin + 1;

    return m_apiInput->UpdateActionState(&m_activeActionSet, sizeof(m_activeActionSet), 1) == vr::VRInputError_None;
}

// Requires: m_vrOverlay
bool OpenVROverlayController::createOverlay()
{
    if (m_overlayHandle != vr::k_ulOverlayHandleInvalid)
	{
        return true;
	}

	qDebug() << tr("Finding overlay");
    vr::EVROverlayError err = m_apiOverlay->FindOverlay(THORQ_APPLICATION_NAME, &m_overlayHandle);
	if (err == vr::VROverlayError_None)
	{
		qDebug() << tr("Found overlay");
		return true;
	}
	else if (err != vr::VROverlayError_UnknownOverlay)
	{
        m_overlayHandle = vr::k_ulOverlayHandleInvalid;
        qWarning() << tr("Error finding overlay:") << m_apiOverlay->GetOverlayErrorNameFromEnum(err);
		return false;
	}

	qDebug() << tr("Creating overlay");
    err = m_apiOverlay->CreateOverlay(THORQ_APPLICATION_NAME, THORQ_APPLICATION_NAME, &m_overlayHandle);
	if (err != vr::VROverlayError_None)
	{
        m_overlayHandle = vr::k_ulOverlayHandleInvalid;
        qWarning() << tr("Error creating overlay:") << m_apiOverlay->GetOverlayErrorNameFromEnum(err);
		return false;
	}

	// Alpha
	m_alpha = 0.9f;
    err = m_apiOverlay->SetOverlayAlpha(m_overlayHandle, m_alpha);
	if (err != vr::VROverlayError_None)
        qWarning() << tr("Error setting overlay alpha:") << m_apiOverlay->GetOverlayErrorNameFromEnum(err);

	// Visibility
	m_isVisible = false;
    err = m_apiOverlay->HideOverlay(m_overlayHandle);
	if (err != vr::VROverlayError_None)
        qWarning() << tr("Error setting overlay visibility:") << m_apiOverlay->GetOverlayErrorNameFromEnum(err);

    // Set input method to simulate a mouse
    err = m_apiOverlay->SetOverlayInputMethod(m_overlayHandle, vr::VROverlayInputMethod_Mouse);
	if (err != vr::VROverlayError_None)
        qWarning() << tr("Error setting overlay input method:") << m_apiOverlay->GetOverlayErrorNameFromEnum(err);

    // Enable Interaction
    err = m_apiOverlay->SetOverlayFlag(m_overlayHandle, vr::VROverlayFlags_MakeOverlaysInteractiveIfVisible, true);
    if (err != vr::VROverlayError_None)
        qWarning() << tr("Error setting overlay input method:") << m_apiOverlay->GetOverlayErrorNameFromEnum(err);

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
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

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
        vr::EVROverlayError err = m_apiOverlay->SetOverlayTexture(m_overlayHandle, &texture);
		if (err != vr::VROverlayError_None)
            qWarning() << tr("Error setting overlay texture:") << m_apiOverlay->GetOverlayErrorNameFromEnum(err);
	}
}

// Requires: m_vrOverlay, m_overlay, m_primary_controller_index, m_overlay_offset
bool OpenVROverlayController::overlayTransform()
{
    if (createOverlay())
    {
        qDebug() << tr("Transforming overlay");

        // Position
        vr::EVROverlayError err = m_apiOverlay->SetOverlayTransformTrackedDeviceRelative(m_overlayHandle, m_overlayDevice, &m_overlayOffset);

        if (err == vr::VROverlayError_None)
        {
            return true;
        }

        qWarning() << tr("Error transforming overlay:") << m_apiOverlay->GetOverlayErrorNameFromEnum(err);
    }

    return false;
}

void OpenVROverlayController::setOverlayDevice(vr::TrackedDeviceIndex_t deviceIndex)
{
    // Set current overlay device as mouse device, and set new device as overlay device
    m_mouseDeviceIndex = m_overlayDevice;
    m_overlayDevice = deviceIndex;
}
void OpenVROverlayController::setOverlayOffset(const QMatrix4x4 &offset)
{
    ToHmdMatrix34(offset, m_overlayOffset);
}

OpenVROverlayController::EHand OpenVROverlayController::getHandForSource(vr::VRInputValueHandle_t source)
{
    if (source == m_sourceControllerLeft)
    {
        return EHand::Left;
    }
    else
    if (source == m_sourceControllerRight)
    {
        return EHand::Right;
    }

    return EHand::Invalid;
}

const QMatrix4x4& OpenVROverlayController::getOffsetForHand(OpenVROverlayController::EHand hand)
{
    switch (hand) {
    case EHand::Left:
        return m_overlayDeviceOffsetL;
    case EHand::Right:
        return m_overlayDeviceOffsetR;
    default:
        return m_overlayDeviceOffsetC;
    }
}

const QMatrix4x4 &OpenVROverlayController::getOffsetForSource(vr::VRInputValueHandle_t source)
{
    if (source == m_sourceControllerLeft)
    {
        return m_overlayDeviceOffsetL;
    }
    else if (source == m_sourceControllerRight)
    {
        return m_overlayDeviceOffsetR;
    }
    else
    {
        return m_overlayDeviceOffsetC;
    }
}

vr::VRInputValueHandle_t OpenVROverlayController::getOriginForHand(OpenVROverlayController::EHand hand)
{
    switch (hand) {
    case EHand::Left:
        return m_sourceControllerLeft;
    case EHand::Right:
        return m_sourceControllerRight;
    default:
        return vr::k_ulInvalidInputValueHandle;
    }
}

vr::TrackedDeviceIndex_t OpenVROverlayController::getDeviceForSource(vr::VRInputValueHandle_t source)
{
    vr::InputOriginInfo_t sourceInfo;
    vr::EVRInputError err = m_apiInput->GetOriginTrackedDeviceInfo(source, &sourceInfo, sizeof(vr::InputOriginInfo_t));

    if (err != vr::VRInputError_None)
    {
        return vr::TrackedControllerRole_Invalid;
    }

    return sourceInfo.trackedDeviceIndex;
}

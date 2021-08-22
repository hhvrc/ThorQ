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

#include "fmt/core.h"

#include "constants.h"

#ifdef _WIN32
#define THORQ_MAX_PATH_LEN MAX_PATH
#else
#define THORQ_MAX_PATH_LEN PATH_MAX
#endif

// Static functions
bool ThorQ::VR::IsSteamVRInstalled()
{
	return vr::VR_IsRuntimeInstalled();
}

bool ThorQ::VR::IsSteamVRRunning()
{
	// TODO: Look for process with name "SteamVR"
	return false;
}

bool ThorQ::VR::IsHmdPresent()
{
    return vr::VR_IsHmdPresent();
}

bool ThorQ::VR::Initialize()
{
    vr::EVRInitError initErr = vr::VRInitError_None;

    // Initialize OpenVR
    vr::VR_Init(&initErr, vr::VRApplication_Overlay);

    if (initErr != vr::VRInitError_None) {
        fmt::print(stderr, "OpenVR error: {}\n", vr::VR_GetVRInitErrorAsEnglishDescription(initErr));
        return false;
    }
    else if (!vr::VR_IsInterfaceVersionValid(vr::IVRSystem_Version)) {
        ThorQ::VR::Shutdown();
        fmt::print(stderr, "OpenVR error: Outdated IVRSystem_Version\n");
        return false;
    }
    else if (!vr::VR_IsInterfaceVersionValid(vr::IVRApplications_Version)) {
        ThorQ::VR::Shutdown();
        fmt::print(stderr, "OpenVR error: Outdated IVRApplications_Version\n");
        return false;
    }
    else if (!vr::VR_IsInterfaceVersionValid(vr::IVRSettings_Version)) {
        ThorQ::VR::Shutdown();
        fmt::print(stderr, "OpenVR error: Outdated IVRSettings_Version\n");
        return false;
    }
    else if (!vr::VR_IsInterfaceVersionValid(vr::IVROverlay_Version)) {
        ThorQ::VR::Shutdown();
        fmt::print(stderr, "OpenVR error: Outdated IVROverlay_Version\n");
        return false;
    }
    else if (!vr::VR_IsInterfaceVersionValid(vr::IVRInput_Version)) {
        ThorQ::VR::Shutdown();
        fmt::print(stderr, "OpenVR error: Outdated IVRInput_Version\n");
        return false;
    }

    // Allow global actionset priority
    if (!ThorQ::VR::OVRSettings::AllowGlobalActionSetPriority(true)) {
        ThorQ::VR::Shutdown();
        return false;
    }

    QDir dir = QDir(QApplication::applicationDirPath());

    QString manifestPath = dir.filePath("thorq.vrmanifest");
    if (!QFile::exists(manifestPath) && !QFile::copy(":/vr/data/thorq.vrmanifest", manifestPath)) {
        return false;
    }

    QString actionsPath = dir.filePath("actions.json");
    if (!QFile::exists(actionsPath) && !QFile::copy(":/vr/data/actions.json", actionsPath)) {
        return false;
    }

    QString bannerPath = dir.filePath("vr-banner.png");
    if (!QFile::exists(bannerPath) && !QFile::copy(":/vr/data/banner.png", bannerPath)) {
        return false;
    }

    dir.mkdir("bindings");
    if (!dir.cd("bindings")) {
        fmt::print(stderr, "Cant create folder for vr bindings!\n");
        return false;
    }

    QDirIterator it(":/vr/data/bindings/");
    while (it.hasNext()) {
        QString internalName = it.next();

        QString externalName = dir.filePath(it.fileName());
        if (!QFile::exists(externalName)) {
            QFile::copy(internalName, externalName);
        }
    }

    if (vr::VRInput()->SetActionManifestPath(actionsPath.toStdString().c_str()) != vr::VRInputError_None) {
        fmt::print(stderr, "Failed to set action manifest path!\n");
        return false;
    }

    return true;
}

void ThorQ::VR::Shutdown()
{
    // Revert global actionset priority
    ThorQ::VR::OVRSettings::AllowGlobalActionSetPriority(false);

    fmt::print("Disconnecting VR runtime\n");
    vr::VR_Shutdown();
}


bool ThorQ::VR::IsManifestInstalled()
{
    return vr::VRApplications()->IsApplicationInstalled(OPENVR_APPLICATION_KEY);
}

bool ThorQ::VR::InstallManifest()
{
    QDir dir = QDir(QApplication::applicationDirPath());
    std::string manifestPath = dir.filePath("thorq.vrmanifest").toStdString();

    vr::EVRApplicationError err = vr::VRApplications()->AddApplicationManifest(manifestPath.c_str());
    if (err != vr::VRApplicationError_None)
    {
        fmt::print(stderr, "Failed to add manifest: {}\n", vr::VRApplications()->GetApplicationsErrorNameFromEnum(err));
        return false;
    }

    vr::VRApplications()->SetApplicationAutoLaunch(OPENVR_APPLICATION_KEY, true);
    if (err != vr::VRApplicationError_None)
    {
        fmt::print(stderr, "Failed to set autolaunch: {}\n", vr::VRApplications()->GetApplicationsErrorNameFromEnum(err));
        return false;
    }

    return true;
}

bool ThorQ::VR::RemoveManifest()
{
    vr::EVRApplicationError err;

    std::string directory;
    directory.resize(THORQ_MAX_PATH_LEN);

    // Get directory of manifest
    std::uint32_t len = vr::VRApplications()->GetApplicationPropertyString(OPENVR_APPLICATION_KEY, vr::VRApplicationProperty_WorkingDirectory_String, directory.data(), THORQ_MAX_PATH_LEN, &err);
    if (err != vr::VRApplicationError_None)
    {
        fmt::print(stderr, "Failed to get old working dir, skipping removal: {}\n", vr::VRApplications()->GetApplicationsErrorNameFromEnum(err));
        return false;
    }

    directory.resize(len - 1);
    directory += "/thorq.vrmanifest";

    // Remove manifest
    err = vr::VRApplications()->RemoveApplicationManifest(directory.c_str());
    if (err != vr::VRApplicationError_None)
    {
        fmt::print(stderr, "Failed to remove menifest: {}\n", vr::VRApplications()->GetApplicationsErrorNameFromEnum(err));
        return false;
    }

    return true;
}

ThorQ::VR::OpenVROverlayController* s_pSharedVRController = nullptr;


ThorQ::VR::OpenVROverlayController::OpenVROverlayController(QObject* parent)
	: QObject(parent)
	, m_isVisible(false)
    , m_alpha(0.9f)
	, m_width(0.f)
    , m_tint()

	, m_proxyWidget(nullptr)
    , m_updateLogicTimer(new QTimer(this))

    , m_overlay(OPENVR_APPLICATION_KEY, OPENVR_APPLICATION_NAME)

    , m_overlayDeviceOffsetL()
    , m_overlayDeviceOffsetR()
    , m_overlayDeviceOffsetC()

    , m_mouseDevice(vr::k_unTrackedDeviceIndexInvalid)
    , m_overlayDevice(vr::k_unTrackedDeviceIndexInvalid)
    , m_mouseControllerRole(vr::TrackedControllerRole_Invalid)
    , m_overlayControllerRole(vr::TrackedControllerRole_Invalid)

    , m_activeActionSet()
    , m_actionSet("/actions/thorq")
    , m_actionHaptics("/actions/thorq/out/haptic")
    , m_actionInteract("/actions/thorq/in/interact")
    , m_actionShowOverlay("/actions/thorq/in/show_overlay")
    , m_actionProxSensor("/actions/thorq/in/prox_sensor")
    , m_hmd("/user/head")
    , m_controllerLeft("/user/hand/left")
    , m_controllerRight("/user/hand/right")

	, m_scene(nullptr)
	, m_glContext(nullptr)
	, m_surface(nullptr)
	, m_frameBuffer(nullptr)

	, m_lastMousePoint()
    , m_lastMouseButtons(Qt::NoButton)
{
    // Overlay stuff
    m_overlay.setAlpha(m_alpha);
    m_overlay.setVisibility(m_isVisible);
    m_overlay.setInputMethod(OVROverlay::InputMethod::Mouse);

    connect(m_updateLogicTimer, &QTimer::timeout, this, &OpenVROverlayController::update);
    m_updateLogicTimer->setInterval(20);

    // Left controller offset
    m_overlayDeviceOffsetL.scale(0.25f);
    m_overlayDeviceOffsetL.rotate(90.f, 1.f, 0.f, 0.f);
    m_overlayDeviceOffsetL.translate(-0.4f, -0.5f, 0.06f);
    m_overlayDeviceOffsetL.optimize();

    // Right controller offset
    m_overlayDeviceOffsetR.scale(0.25f);
    m_overlayDeviceOffsetR.rotate(90.f, 1.f, 0.f, 0.f);
    m_overlayDeviceOffsetR.translate(0.4f, -0.5f, 0.06f);
    m_overlayDeviceOffsetR.optimize();

    // Unidirectional controller offset
    m_overlayDeviceOffsetC.scale(0.25f);
    m_overlayDeviceOffsetC.rotate(90.f, 1.f, 0.f, 0.f);
    m_overlayDeviceOffsetC.translate(0.f, -0.5f, 0.06f);
    m_overlayDeviceOffsetC.optimize();

    // Init m_glContext
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
        return;
    }

    // Init m_surface
    // create an offscreen surface to attach the context and FBO to
    m_surface = new QOffscreenSurface(nullptr, this);
    m_surface->create();
    m_glContext->makeCurrent(m_surface);

    // Init m_scene
    m_scene = new QGraphicsScene(this);
    connect(m_scene, &QGraphicsScene::changed, this, &OpenVROverlayController::onSceneChanged);

    // Start m_updateLogicTimer
    if (!m_updateLogicTimer->isActive())
    {
        m_updateLogicTimer->start();
    }
}

ThorQ::VR::OpenVROverlayController::~OpenVROverlayController()
{
    fmt::print("Stopping timers\n");
    m_updateLogicTimer->stop();

    delete m_frameBuffer;
}

bool ThorQ::VR::OpenVROverlayController::isValid() const
{
    return m_actionSet.isValid()         &&
           m_actionHaptics.isValid()     &&
           m_actionInteract.isValid()    &&
           m_actionShowOverlay.isValid() &&
           m_actionProxSensor.isValid()  &&
           m_glContext != nullptr;
}

bool ThorQ::VR::OpenVROverlayController::setWidget(QWidget* widget)
{
	if (m_scene == nullptr)
    {
		return false;
    }

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
    //widget->resize(widget->sizeHint());

	// all of the mouse handling stuff requires that the widget be at 0,0
    widget->move(0, 0);

    m_proxyWidget = m_scene->addWidget(widget);

	delete m_frameBuffer;
	m_frameBuffer = new QOpenGLFramebufferObject(widget->size(), GL_TEXTURE_2D);

	emit widgetChanged(widget);

	return true;
}
QWidget* ThorQ::VR::OpenVROverlayController::widget() const
{
	if (m_proxyWidget != nullptr)
		return m_proxyWidget->widget();
	return nullptr;
}

void ThorQ::VR::OpenVROverlayController::setIsVisible(bool visible)
{
    if (m_isVisible != visible && m_overlay.setVisibility(visible)) {
        m_isVisible = visible;
        emit isVisibleChanged(visible);
    }
}
bool ThorQ::VR::OpenVROverlayController::isVisible() const
{
	return m_isVisible;
}

void ThorQ::VR::OpenVROverlayController::setWidth(float width)
{
    if (m_width != width && m_overlay.setWidth(width)) {
        m_width = width;
        emit widthChanged(width);
    }
}
float ThorQ::VR::OpenVROverlayController::width() const
{
	return m_width;
}

void ThorQ::VR::OpenVROverlayController::setAlpha(float alpha)
{
    if (m_alpha != alpha && m_overlay.setAlpha(alpha)) {
		m_alpha = alpha;
        emit alphaChanged(alpha);
	}
}
float ThorQ::VR::OpenVROverlayController::alpha() const
{
	return m_alpha;
}

void ThorQ::VR::OpenVROverlayController::setTint(const QColor& tint)
{
    if (tint.isValid() && m_tint != tint && m_overlay.setTint(tint.redF(), tint.greenF(), tint.blueF())) {
        m_tint = tint;
		emit tintChanged(tint);
    }
}

void ThorQ::VR::OpenVROverlayController::setCurvature(float curvature)
{
    if (m_curvature != m_curvature && m_overlay.setCurvature(curvature)) {
        m_curvature = curvature;
        emit curvatureChanged(curvature);
    }
}

bool ThorQ::VR::OpenVROverlayController::openBindingUI()
{
    return m_actionSet.openBindingUI(m_actionInteract);
}

void ThorQ::VR::OpenVROverlayController::halt()
{
    m_updateLogicTimer->stop();
    m_overlay.destroy();
}

QColor ThorQ::VR::OpenVROverlayController::tint() const
{
    return m_tint;
}

float ThorQ::VR::OpenVROverlayController::curvature() const
{
    return m_curvature;
}

void ThorQ::VR::OpenVROverlayController::update()
{
    if (!pullEvents()) {
        fmt::print(stderr, "Error pulling events!");
        return;
    }

    vr::InputDigitalActionData_t interactionState, showOverlayState, proxSensorState;
    m_actionInteract.getDigitalData(interactionState);
    m_actionShowOverlay.getDigitalData(showOverlayState);
    m_actionProxSensor.getDigitalData(proxSensorState);

    if (proxSensorState.bActive && proxSensorState.bChanged) {
        fmt::print("Proximity sensor: {}\n", proxSensorState.bState);
    }

    if (/*showOverlayState.bActive && */showOverlayState.bChanged)
    {
        fmt::print("show overlay changed: {}\n", showOverlayState.bChanged);
        if (showOverlayState.bState)
        {
            ThorQ::VR::OVRInputSource activeOrigin(showOverlayState.activeOrigin);

            auto device = activeOrigin.trackedDevice();

            if (device != vr::k_unTrackedDeviceIndexInvalid)
            {
                setOverlayDevice(device);
                setIsVisible(true);
            }
        }
        else
        {
            setIsVisible(false);
        }
    }

    if (/*interactionState.bActive && */interactionState.bChanged)
    {
        fmt::print("interaction changed: {}\n", interactionState.bState);
        if (interactionState.bState)
        {
            m_actionHaptics.triggerHapticFeedback(0.f, 1.f, 500.f, 1.f, interactionState.activeOrigin);
        }
        else
        {
            m_actionHaptics.triggerHapticFeedback(0.f, 1.f, 250.f, 0.5f, interactionState.activeOrigin);;
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
    // https://github.com/aardvarkxr/aardvark/blob/master/src/avrenderer/vrmanager.cpp
    // https://github.com/aardvarkxr/aardvark/blob/master/data/aardvark.vrmanifest


    vr::VREvent_t event{};

    // Poll global events
    while (vr::VRSystem()->PollNextEvent(&event, sizeof(event)))
    {
        switch(event.eventType)
        {
            case vr::VREvent_Input_ActionManifestLoadFailed:
            {
                fmt::print("Manifest failed to load\n");
                break;
            }
            case vr::VREvent_Input_ActionManifestReloaded:
            {
                fmt::print("Manifest reloaded\n");
                break;
            }
            case vr::VREvent_Input_BindingLoadFailed:
            {
                fmt::print("Binding failed to load\n");
                break;
            }
            case vr::VREvent_Input_BindingLoadSuccessful:
            {
                fmt::print("Binding loaded\n");
                break;
            }
            case vr::VREvent_TrackedDeviceDeactivated:
            {
                fmt::print("Tracked device deactivated\n");
                break;
            }
            case vr::VREvent_TrackedDeviceUpdated:
            {
                fmt::print("Tracked device update\n");
                break;
            }
            case vr::VREvent_TrackedDeviceUserInteractionStarted:
            {
                fmt::print("Interaction started\n");
                break;
            }
            case vr::VREvent_TrackedDeviceUserInteractionEnded:
            {
                fmt::print("Interaction ended\n");
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
                halt();
                return;
            }
        }
    }

    // Poll overlay events
    while(m_overlay.pollEvent(event)) {
        switch(event.eventType)
        {
        case vr::VREvent_MouseMove:
        {
            fmt::print("Mouse move\n");
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
            fmt::print("Mouse press\n");
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
            fmt::print("Mouse release\n");
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

bool ThorQ::VR::OpenVROverlayController::pullEvents()
{
    m_activeActionSet.ulActionSet = m_actionSet.handle();
    m_activeActionSet.ulRestrictedToDevice = vr::k_ulInvalidInputValueHandle;
    m_activeActionSet.ulSecondaryActionSet = vr::k_ulInvalidInputValueHandle;
    m_activeActionSet.nPriority = vr::k_nActionSetOverlayGlobalPriorityMin + 1;

    vr::EVRInputError err = vr::VRInput()->UpdateActionState(&m_activeActionSet, sizeof(m_activeActionSet), 1);

    if (err != vr::VRInputError_None && err != vr::VRInputError_NoData)
    {
        fmt::print("Error updating action state: {}\n", err);
    }

    return true;
}

// Requires: m_glContext, m_surface, m_frameBuffer, m_scene, m_vrOverlay, m_overlay
void ThorQ::VR::OpenVROverlayController::onSceneChanged()
{
    if (m_proxyWidget == nullptr) {
        return;
    }

    QWidget* widget = m_proxyWidget->widget();
    QSize newSize = widget->sizeHint();

    if (m_frameBuffer->size() != newSize)
    {
        fmt::print("Resizing from [{}x{}] to [{}x{}]\n", m_frameBuffer->width(), m_frameBuffer->height(), newSize.width(), newSize.height());

        widget->resize(newSize);
        widget->move(0, 0);

        delete m_frameBuffer;
        m_frameBuffer = new QOpenGLFramebufferObject(newSize, GL_TEXTURE_2D);
    }

    m_glContext->makeCurrent(m_surface);
    m_frameBuffer->bind();

    QOpenGLPaintDevice device(m_frameBuffer->size());
    QPainter painter(&device);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

	m_scene->render(&painter);

    m_overlay.setTexture(m_frameBuffer->release());
}

void ThorQ::VR::OpenVROverlayController::setOverlayDevice(ThorQ::VR::OVRTrackedDevice device)
{
    if (m_overlayDevice != device)
    {
        // Set current overlay device as mouse device, and set new device as overlay device
        m_mouseDevice = m_overlayDevice;
        m_overlayDevice = device;
        m_overlay.setDeviceRelativeTransform(device, getOffsetForTrackedDevice(device));
    }
}

const QMatrix4x4& ThorQ::VR::OpenVROverlayController::getOffsetForControllerRole(vr::ETrackedControllerRole role) const
{
    switch (role) {
    case vr::TrackedControllerRole_LeftHand:
        return m_overlayDeviceOffsetL;
    case vr::TrackedControllerRole_RightHand:
        return m_overlayDeviceOffsetR;
    default:
        return m_overlayDeviceOffsetC;
    }
}

const QMatrix4x4 &ThorQ::VR::OpenVROverlayController::getOffsetForTrackedDevice(ThorQ::VR::OVRTrackedDevice device) const
{
    return getOffsetForControllerRole(device.controllerRole());
}

const QMatrix4x4& ThorQ::VR::OpenVROverlayController::getOffsetForInputSource(ThorQ::VR::OVRInputSource source) const
{
    if (source == m_controllerLeft)
    {
        return m_overlayDeviceOffsetL;
    }
    else if (source == m_controllerRight)
    {
        return m_overlayDeviceOffsetR;
    }
    else
    {
        return m_overlayDeviceOffsetC;
    }
}

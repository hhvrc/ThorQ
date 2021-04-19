#include "declerations_qt.h"
#include "stylesheets.h"
#include "accountloginwidget.h"
#include "accountregisterwidget.h"
#include "accountrecoverwidget.h"
#include "mainwidget.h"
#include "collar/serial.h"
#include "apiclient.h"
#include "vr/openvroverlaycontroller.h"
#include "accountcontroller.h"

#include <constants.h>
#include <encoding.h>

#include <QApplication>
#include <QCoreApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QStyle>
#include <QLabel>
#include <QIcon>
#include <QSpinBox>
#include <QInputDialog>
#include <QRegExp>
#include <QDebug>
#include <QDir>
#include <QTranslator>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QPushButton>

#include <filesystem>

#define COMTEST 1

std::uint16_t i = 0;
int main(int argc, char** argv)
{
    // TODO: customize GUI
    // TODO: enable support for SteamVR
    QCoreApplication::addLibraryPath(QDir::currentPath().append("/libs/"));

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setApplicationName(THORQ_APPLICATION_NAME);
    QCoreApplication::setApplicationVersion(ThorQ::ClientVersion.toString().c_str());
    QCoreApplication::setOrganizationName(THORQ_ORGANIZATION_NAME);
    QCoreApplication::setOrganizationDomain(THORQ_ORGANIZATION_DOMAIN);

    QApplication app(argc, argv);

    // Test the presence and variant of settings
    QSettings settings;
    if (!settings.contains("server/hostname") ||
        !settings.contains("server/port")) {
        settings.setValue("server/hostname", THORQ_SERVER_HOSTNAME);
        settings.setValue("server/port", THORQ_SERVER_PORT);
    }

#if COMTEST
#else
    if (!ThorQ::VR::Initialize()) {
        qDebug() << "Failed to init VR";
        return EXIT_FAILURE;
    }

    if (ThorQ::VR::IsManifestInstalled()) {
        ThorQ::VR::RemoveManifest();
    }

    if (!ThorQ::VR::InstallManifest()) {
        qDebug() << "Failed to install VR manifest files";
        return EXIT_FAILURE;
    }
#endif
    app.setStyleSheet(ThorQ::StyleSheets::tryGetStylesheet("main"));
    app.setDesktopFileName(THORQ_APPLICATION_NAME);
    app.setWindowIcon(QIcon(":/shockGrey.ico"));
    //app.setQuitOnLastWindowClosed(false);
#if COMTEST

    ThorQ::ApiClient* apiClient = new ThorQ::ApiClient(&app);
    QTimer* reconnectTimer = new QTimer(&app);
    reconnectTimer->setInterval(5000);
    reconnectTimer->setSingleShot(true);
    QObject::connect(reconnectTimer, &QTimer::timeout, apiClient, &ThorQ::ApiClient::netConnect);
    QObject::connect(apiClient, &ThorQ::ApiClient::netDisconnected, [reconnectTimer](){ reconnectTimer->start(); });

    CollarSerial* ser = new CollarSerial(&app);

    QObject::connect(ser, &CollarSerial::FoundCollar, [ser](){
        ser->SendBeep(1);
        QThread::sleep(1);
        ser->SendBeep(1);
        QThread::sleep(1);
        ser->SendBeep(1);
        QThread::sleep(1);
        ser->SendBeep(1);
        QThread::sleep(1);
        ser->SendBeep(1);
    });

    ser->FindCollar();
    QObject::connect(apiClient, &ThorQ::ApiClient::gotShock, ser, &CollarSerial::SendShock);
    QObject::connect(apiClient, &ThorQ::ApiClient::gotVibrate, ser, &CollarSerial::SendVibration);
    QObject::connect(apiClient, &ThorQ::ApiClient::gotBeep, ser, &CollarSerial::SendBeep);

    QWidget mainWindow;
    mainWindow.setWindowTitle("ThorQ");
    mainWindow.setWindowFlags(Qt::MSWindowsFixedSizeDialogHint);
    mainWindow.setFixedSize(350, 450);
    mainWindow.setGeometry(
                QStyle::alignedRect(
                    Qt::LeftToRight,
                    Qt::AlignCenter,
                    mainWindow.size(),
                    QApplication::primaryScreen()->availableGeometry()
                )
            );

    QVBoxLayout* mainLayout = new QVBoxLayout(&mainWindow);

    auto accountController = apiClient->accountController();

    ThorQ::AccountLoginWidget* loginWidget = new ThorQ::AccountLoginWidget(accountController, &mainWindow);
    ThorQ::AccountRegisterWidget* registerWidget = new ThorQ::AccountRegisterWidget(accountController, &mainWindow);
    ThorQ::AccountRecoverWidget* recoverWidget = new ThorQ::AccountRecoverWidget(accountController, &mainWindow);
    QHBoxLayout* buttonsLayout = new QHBoxLayout();

    QPushButton* registerButton = new QPushButton(&mainWindow);
    QPushButton* recoverButton = new QPushButton(&mainWindow);

    registerWidget->hide();
    recoverWidget->hide();

    registerButton->setText("REGISTER");
    registerButton->setCursor(Qt::PointingHandCursor);

    recoverButton->setText("RECOVER");
    recoverButton->setCursor(Qt::PointingHandCursor);

    mainLayout->addWidget(loginWidget);
    mainLayout->addWidget(registerWidget);
    mainLayout->addWidget(recoverWidget);

    buttonsLayout->addWidget(registerButton);
    buttonsLayout->addWidget(recoverButton);

    mainLayout->addLayout(buttonsLayout);

    mainWindow.setLayout(mainLayout);

    QObject::connect(registerWidget, &ThorQ::AccountRegisterWidget::goBackButtonPressed, loginWidget, &QWidget::show);
    QObject::connect(registerWidget, &ThorQ::AccountRegisterWidget::goBackButtonPressed, recoverButton, &QWidget::show);
    QObject::connect(registerWidget, &ThorQ::AccountRegisterWidget::goBackButtonPressed, registerButton, &QWidget::show);

    QObject::connect(recoverWidget, &ThorQ::AccountRecoverWidget::goBackButtonPressed, loginWidget, &QWidget::show);
    QObject::connect(recoverWidget, &ThorQ::AccountRecoverWidget::goBackButtonPressed, recoverButton, &QWidget::show);
    QObject::connect(recoverWidget, &ThorQ::AccountRecoverWidget::goBackButtonPressed, registerButton, &QWidget::show);

    QObject::connect(registerButton, &QPushButton::pressed, loginWidget, &QWidget::hide);
    QObject::connect(registerButton, &QPushButton::pressed, recoverButton, &QWidget::hide);
    QObject::connect(registerButton, &QPushButton::pressed, registerButton, &QWidget::hide);
    QObject::connect(registerButton, &QPushButton::pressed, registerWidget, &QWidget::show);

    QObject::connect(recoverButton, &QPushButton::pressed, loginWidget, &QWidget::hide);
    QObject::connect(recoverButton, &QPushButton::pressed, recoverButton, &QWidget::hide);
    QObject::connect(recoverButton, &QPushButton::pressed, registerButton, &QWidget::hide);
    QObject::connect(recoverButton, &QPushButton::pressed, recoverWidget, &QWidget::show);

    ThorQ::MainWidget mainWidget;
    QGridLayout* l1 = new QGridLayout(&mainWidget);

    QSpinBox* shockVal = new QSpinBox(&mainWidget);
    QSpinBox* vibrateVal = new QSpinBox(&mainWidget);

    QPushButton* shock = new QPushButton(&mainWidget);
    shock->setText("Shock");

    QPushButton* vibrate = new QPushButton(&mainWidget);
    vibrate->setText("Vibrate");

    QPushButton* beep = new QPushButton(&mainWidget);
    beep->setText("Beep");

    shockVal->setMaximum(7);
    vibrateVal->setMaximum(7);

    l1->addWidget(shockVal, 1, 1);
    l1->addWidget(vibrateVal, 1, 3);

    l1->addWidget(shock, 3, 1);
    l1->addWidget(vibrate, 3, 3);
    l1->addWidget(beep, 3, 5);

    l1->setColumnMinimumWidth(15, 0);
    l1->setColumnMinimumWidth(15, 2);
    l1->setColumnMinimumWidth(15, 4);
    l1->setColumnMinimumWidth(15, 6);

    l1->setRowMinimumHeight(15, 0);
    l1->setRowMinimumHeight(15, 2);
    l1->setRowMinimumHeight(15, 4);
    l1->setRowMinimumHeight(15, 6);

    mainWidget.adjustSize();

    QObject::connect(vibrate, &QPushButton::clicked, [](){ qDebug() << "Clicked!"; });

    QObject::connect(accountController, &ThorQ::AccountController::loggedIn, &mainWindow, &QWidget::hide);
    QObject::connect(accountController, &ThorQ::AccountController::loggedIn, &mainWidget, &QWidget::show);
    QObject::connect(accountController, &ThorQ::AccountController::loggedIn, &mainWidget, &QWidget::show);
    QObject::connect(accountController, &ThorQ::AccountController::loggedOut, &mainWindow, &QWidget::show);
    QObject::connect(accountController, &ThorQ::AccountController::loggedOut, &mainWidget, &QWidget::hide);
    QObject::connect(accountController, &ThorQ::AccountController::loggedOut, &mainWidget, &QWidget::hide);

    QObject::connect(apiClient, &ThorQ::ApiClient::connectionStatusChanged, loginWidget, &ThorQ::AccountLoginWidget::setConnectionStatus);

    apiClient->netConnect();
    mainWindow.show();
#else
    QPixmap pix(":/uwu.png");
    QLabel lab;
    lab.setPixmap(pix);

    ThorQ::VR::OpenVROverlayController* ovr = new ThorQ::VR::OpenVROverlayController(&app);

    ovr->setWidget(&lab);
    QObject::connect(ovr, &ThorQ::VR::OpenVROverlayController::vrQuit, ovr, &QObject::deleteLater);
    QObject::connect(ovr, &ThorQ::VR::OpenVROverlayController::vrQuit, &app, &QApplication::quit);
#endif

    int retval = app.exec();

#if COMTEST
#else
    ThorQ::VR::Shutdown();
#endif
    return retval;
}

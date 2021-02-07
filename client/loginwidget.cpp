#include "loginwidget.h"

#include <QDebug>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QCoreApplication>

struct UiState {
    const char* text;
    const char* styleSheet;
    bool hasPingIndicator;
};

UiState uiConnectionStatusList[]
{
    { "● Error",                    "font-size: 16px; color: #FF0000", false }, // ConnectionStatus::Error
    { "● Offline",                  "font-size: 16px; color: #FF0000", false }, // ConnectionStatus::Disconnected
    { "● Connecting..."   ,         "font-size: 16px; color: #FFA500", false }, // ConnectionStatus::Connecting
    { "● Connected\n%1 ms",         "font-size: 16px; color: #00FF00", true  }, // ConnectionStatus::Connected
    { "● Disconnecting...",         "font-size: 16px; color: #FF0000", false }, // ConnectionStatus::Disconnecting
};

const char* uiStatusList[][2]
{

    { "● Requesting...\n%1 ms",     "font-size: 16px; color: #FFA500" }, // THORQ_STATE_CRYPTO_REQUESTED
    { "● Encrypting...\n%1 ms",     "font-size: 16px; color: #FFA500" }, // THORQ_STATE_CRYPTO_ESTABLISHING
    { "● Verifying...\n%1 ms",      "font-size: 16px; color: #FFA500" }, // THORQ_STATE_CRYPTO_VERIFYING
    { "● Encryped\n%1 ms",          "font-size: 16px; color: #00FF00" }, // THORQ_STATE_AUTH_NONE

    { "● Authenticating...\n%1 ms", "font-size: 16px; color: #FFA500" }, // THORQ_STATE_AUTH_HWID_CHECKING
    { "● Authenticated\n%1 ms",     "font-size: 16px; color: #00FF00" }, // THORQ_STATE_AUTH_OK

    { "● Logging out...\n%1 ms",    "font-size: 16px; color: #FFA500" }, // THORQ_STATE_LOGIN_LOGGINGOUT
    { "● Logging in...\n%1 ms",     "font-size: 16px; color: #FFA500" }, // THORQ_STATE_LOGIN_LOGGINGIN
    { "● Logged in\n%1 ms",         "font-size: 16px; color: #00FF00" }, // THORQ_STATE_LOGIN_LOGGEDIN
};

inline void setLabelText(QLabel* label, const UiState& state, unsigned int ping = 0)
{
    QString str = QCoreApplication::tr(state.text);

    label->setStyleSheet(state.styleSheet);
    if (state.hasPingIndicator) {
        label->setText(str.arg(ping));
    }
    else {
        label->setText(str);
    }
}

LoginWidget::LoginWidget(QWidget* parent)
	: QWidget(parent)
    , m_connectionStatus(ConnectionStatus::Error)
    , m_ping(0)
    , m_title(new QLabel(this))
    , m_onlineStatus(new QLabel(this))
    , m_textInput(new QLineEdit(this))
    , m_acceptButton(new QPushButton(this))
    , m_mainLayout(new QVBoxLayout(this))
    , m_headerLayout(new QHBoxLayout())
{
    setWindowTitle(tr("ThorQ Login"));

    m_title->setText("ThorQ");
    m_title->setStyleSheet("font-size: 72px; color: #FFFFFF");

    setLabelText(m_onlineStatus, uiConnectionStatusList[0]);

	m_headerLayout->addWidget(m_title);
    m_headerLayout->addWidget(m_onlineStatus);

	m_mainLayout->addLayout(m_headerLayout);
    m_mainLayout->addWidget(m_textInput);
    m_mainLayout->addWidget(m_acceptButton);
	setLayout(m_mainLayout);

	setFixedSize(m_mainLayout->geometry().size());
	setWindowFlags(Qt::MSWindowsFixedSizeDialogHint);

    QObject::connect(m_acceptButton, &QPushButton::clicked, [this]()
    {
        if (m_textInput->text().isEmpty())
            return;

        emit usernameEntered(m_textInput->text());
    });

    updateUiState();
}

LoginWidget::~LoginWidget()
{
	delete m_headerLayout;
}

void LoginWidget::setConnectionStatus(ConnectionStatus status)
{
    if (m_connectionStatus != status)
	{
        m_connectionStatus = status;
        setLabelText(m_onlineStatus, uiConnectionStatusList[(int)status], m_ping);
        updateUiState();
    }
}

void LoginWidget::setConnectionPing(uint ping)
{
	if (m_ping != ping)
	{
		m_ping = ping;
		updateUiPing();
    }
}

void LoginWidget::updateUiState()
{
    /*
    if (m_connectionStatus == THORQ_STATE_LOGIN_LOGGEDOUT)
    {
        m_textInput->setText("");
        m_textInput->show();

        m_acceptButton->setText(tr("Login"));
        m_acceptButton->show();

        adjustSize();
    }
    else
    {
        m_acceptButton->hide();
        m_textInput->hide();
        adjustSize();
    }

    if (m_connectionStatus < THORQ_STATE_LOGIN_LOGGEDIN)
	{
		show();
	}
	else
	{
		hide();
	}
    */
}

void LoginWidget::updateUiPing()
{
    if (m_connectionStatus == ConnectionStatus::Connected)
    {
        //m_onlineStatus->setText(tr(uiStatusList[m_connectionStatus][0]).arg(m_ping));
    }
}

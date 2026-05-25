#include "AppController.h"
#include <QApplication>
#include <QCoreApplication>
#include <QSettings>
#include <QIcon>
#include <QMenu>
#include <QPainter>
#include <QtConcurrent/QtConcurrentRun>
#include <QMetaObject>
#include <QUrl>
#include <QVariant>

AppController::AppController(QObject* parent)
    : QObject(parent)
    , m_payloadController(std::make_unique<PayloadController>())
{
    m_payloadController->SetLogCallback([this](const std::wstring& msg) {
        QString text = QString::fromStdWString(msg);
        QMetaObject::invokeMethod(this, "appendLogMessage", Qt::QueuedConnection, Q_ARG(QString, text));
    });

    m_payloadController->SetStatusCallback([this](int status) {
        QMetaObject::invokeMethod(this, [this, status]() {
            emit statusChanged(status);
        }, Qt::QueuedConnection);
    });

    connect(&m_statusTimer, &QTimer::timeout, this, &AppController::pollDeviceStatus);
    m_statusTimer.start(1000);

    connect(&m_injectWatcher, &QFutureWatcher<int>::finished, this, [this]() {
        int result = m_injectWatcher.result();
        appendLogMessage(tr("Payload injection finished with code %1").arg(result));
    });

    createTrayIcon();
    loadSettings();
    requestDeviceStatus();
}

AppController::~AppController() = default;

QString AppController::payloadPath() const
{
    return m_payloadPath;
}

QString AppController::theme() const
{
    return m_theme;
}

bool AppController::autoInject() const
{
    return m_autoInject;
}

bool AppController::minimizeToTray() const
{
    return m_minimizeToTray;
}

QStringList AppController::favorites() const
{
    return m_favorites;
}

QStringList AppController::logMessages() const
{
    return m_logMessages;
}

bool AppController::deviceConnected() const
{
    return m_deviceConnected;
}

void AppController::setTheme(const QString& themeName)
{
    if (themeName.isEmpty() || themeName == m_theme)
        return;

    m_theme = themeName;
    saveSettings();
    emit themeChanged();
}

void AppController::setAutoInject(bool enabled)
{
    if (enabled == m_autoInject)
        return;

    m_autoInject = enabled;
    saveSettings();
    emit autoInjectChanged();
}

void AppController::setMinimizeToTray(bool enabled)
{
    if (enabled == m_minimizeToTray)
        return;

    m_minimizeToTray = enabled;
    saveSettings();
    emit minimizeToTrayChanged();
}

void AppController::setPayloadPath(const QVariant& pathOrUrl)
{
    QString path;
    if (pathOrUrl.userType() == QMetaType::QUrl) {
        path = pathOrUrl.toUrl().toLocalFile();
    } else if (pathOrUrl.canConvert<QUrl>()) {
        QUrl url = pathOrUrl.toUrl();
        if (url.isLocalFile()) {
            path = url.toLocalFile();
        } else {
            path = pathOrUrl.toString();
        }
    } else {
        path = pathOrUrl.toString();
    }

    if (path == m_payloadPath)
        return;

    m_payloadPath = path;
    emit payloadPathChanged();
}

void AppController::injectPayload(const QString& payloadPath)
{
    QString path = payloadPath.isEmpty() ? m_payloadPath : payloadPath;
    if (path.isEmpty()) {
        appendLogMessage(tr("Please select a payload before injecting."));
        return;
    }

    updateFavorites(path);
    appendLogMessage(tr("Starting payload injection: %1").arg(path));
    m_injectWatcher.setFuture(QtConcurrent::run([this, path]() {
        return m_payloadController->InjectPayload(path.toStdWString());
    }));
}

void AppController::addFavorite(const QString& payloadPath)
{
    if (payloadPath.isEmpty()) {
        appendLogMessage(tr("Cannot add an empty payload path to favorites."));
        return;
    }

    updateFavorites(payloadPath);
    appendLogMessage(tr("Added to favorites: %1").arg(payloadPath));
}

void AppController::removeFavorite(const QString& payloadPath)
{
    if (m_favorites.removeAll(payloadPath) > 0) {
        saveSettings();
        emit favoritesChanged();
        appendLogMessage(tr("Removed favorite: %1").arg(payloadPath));
    }
}

void AppController::clearLogs()
{
    m_logMessages.clear();
    emit logMessagesChanged();
}

void AppController::requestDeviceStatus()
{
    pollDeviceStatus();
}

void AppController::showMainWindow()
{
    for (QWindow* window : qApp->topLevelWindows()) {
        if (!window)
            continue;

        window->showNormal();
        window->requestActivate();
    }
}

void AppController::pollDeviceStatus()
{
    int status = m_payloadController->GetRcmStatus();
    bool connected = (status == 0);

    if (connected != m_deviceConnected) {
        m_deviceConnected = connected;
        emit deviceConnectedChanged();
        appendLogMessage(connected ? tr("RCM device connected.") : tr("RCM device disconnected."));
    }

    emit statusChanged(status);

    if (m_autoInject && connected && !m_payloadPath.isEmpty() && m_lastStatus != 0) {
        injectPayload();
    }

    m_lastStatus = status;
}

void AppController::appendLogMessage(const QString& message)
{
    if (message.isEmpty())
        return;

    m_logMessages.append(message);
    if (m_logMessages.size() > 250)
        m_logMessages.remove(0, m_logMessages.size() - 250);

    emit logMessagesChanged();
    emit logMessage(message);
}

void AppController::loadSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.beginGroup("General");
    m_theme = settings.value("theme", "Dark").toString();
    m_autoInject = settings.value("autoInject", false).toBool();
    m_minimizeToTray = settings.value("minimizeToTray", false).toBool();
    m_favorites = settings.value("favorites").toStringList();
    settings.endGroup();

    if (m_favorites.size() > 5)
        m_favorites = m_favorites.mid(0, 5);

    emit themeChanged();
    emit autoInjectChanged();
    emit minimizeToTrayChanged();
    emit favoritesChanged();
    emit settingsLoaded();
}

void AppController::saveSettings() const
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.beginGroup("General");
    settings.setValue("theme", m_theme);
    settings.setValue("autoInject", m_autoInject);
    settings.setValue("minimizeToTray", m_minimizeToTray);
    settings.setValue("favorites", m_favorites);
    settings.endGroup();
    settings.sync();
}

void AppController::updateFavorites(const QString& payloadPath)
{
    if (payloadPath.isEmpty())
        return;

    m_favorites.removeAll(payloadPath);
    m_favorites.prepend(payloadPath);
    while (m_favorites.size() > 5)
        m_favorites.removeLast();

    saveSettings();
    emit favoritesChanged();
}

void AppController::createTrayIcon()
{
    if (m_trayIcon)
        return;

    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor("#2a82da"));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(4, 4, 24, 24);
    painter.setBrush(Qt::white);
    painter.drawRect(14, 10, 4, 12);
    painter.drawRect(14, 18, 12, 4);

    m_trayIcon = new QSystemTrayIcon(QIcon(pixmap), this);
    m_trayIconMenu = new QMenu();
    QAction* showAction = m_trayIconMenu->addAction(tr("Show"));
    QAction* exitAction = m_trayIconMenu->addAction(tr("Exit"));

    connect(showAction, &QAction::triggered, this, &AppController::showMainWindow);
    connect(exitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick)
            showMainWindow();
    });

    m_trayIcon->setContextMenu(m_trayIconMenu);
    m_trayIcon->setToolTip(tr("TegraRcmGUI"));
    m_trayIcon->show();
}

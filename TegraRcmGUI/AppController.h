#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QFutureWatcher>
#include <QSystemTrayIcon>
#include <QVariant>
#include <memory>
#include "PayloadController.h"

class AppController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString payloadPath READ payloadPath WRITE setPayloadPath NOTIFY payloadPathChanged)
    Q_PROPERTY(QString theme READ theme NOTIFY themeChanged)
    Q_PROPERTY(bool autoInject READ autoInject NOTIFY autoInjectChanged)
    Q_PROPERTY(bool minimizeToTray READ minimizeToTray NOTIFY minimizeToTrayChanged)
    Q_PROPERTY(QStringList favorites READ favorites NOTIFY favoritesChanged)
    Q_PROPERTY(QStringList logMessages READ logMessages NOTIFY logMessagesChanged)
    Q_PROPERTY(bool deviceConnected READ deviceConnected NOTIFY deviceConnectedChanged)

public:
    explicit AppController(QObject* parent = nullptr);
    ~AppController() override;

    QString payloadPath() const;
    QString theme() const;
    bool autoInject() const;
    bool minimizeToTray() const;
    QStringList favorites() const;
    QStringList logMessages() const;
    bool deviceConnected() const;

    Q_INVOKABLE void setTheme(const QString& themeName);
    Q_INVOKABLE void setAutoInject(bool enabled);
    Q_INVOKABLE void setMinimizeToTray(bool enabled);
    Q_INVOKABLE void setPayloadPath(const QVariant& pathOrUrl);
    Q_INVOKABLE void injectPayload(const QString& payloadPath = QString());
    Q_INVOKABLE void addFavorite(const QString& payloadPath);
    Q_INVOKABLE void removeFavorite(const QString& payloadPath);
    Q_INVOKABLE void clearLogs();
    Q_INVOKABLE void requestDeviceStatus();
    Q_INVOKABLE void showMainWindow();

signals:
    void payloadPathChanged();
    void themeChanged();
    void autoInjectChanged();
    void minimizeToTrayChanged();
    void favoritesChanged();
    void logMessagesChanged();
    void deviceConnectedChanged();
    void statusChanged(int statusCode);
    void logMessage(const QString& message);
    void settingsLoaded();

private slots:
    void pollDeviceStatus();
    void appendLogMessage(const QString& message);

private:
    void loadSettings();
    void saveSettings() const;
    void updateFavorites(const QString& payloadPath);
    void createTrayIcon();

    QStringList m_favorites;
    QStringList m_logMessages;
    QString m_payloadPath;
    QString m_theme;
    bool m_autoInject{false};
    bool m_minimizeToTray{false};
    bool m_deviceConnected{false};
    int m_lastStatus{-1};

    std::unique_ptr<PayloadController> m_payloadController;
    QTimer m_statusTimer;
    QFutureWatcher<int> m_injectWatcher;
    QSystemTrayIcon* m_trayIcon{nullptr};
    QMenu* m_trayIconMenu{nullptr};
};

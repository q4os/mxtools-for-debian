#pragma once

#include "actionmodel.h"

#include <QSettings>

class QCommandLineParser;

class Backend final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ActionModel *actions READ actions CONSTANT)
    Q_PROPERTY(QString distroTitle READ distroTitle CONSTANT)
    Q_PROPERTY(QString distroVersion READ distroVersion CONSTANT)
    Q_PROPERTY(QString debianVersion READ debianVersion CONSTANT)
    Q_PROPERTY(QString desktopVersion READ desktopVersion CONSTANT)
    Q_PROPERTY(QString supportedUntil READ supportedUntil CONSTANT)
    Q_PROPERTY(QString termsSummary READ termsSummary CONSTANT)
    Q_PROPERTY(QString liveUserInfo READ liveUserInfo CONSTANT)
    Q_PROPERTY(QUrl headerSource READ headerSource CONSTANT)
    Q_PROPERTY(QUrl logoSource READ logoSource CONSTANT)
    Q_PROPERTY(QString systemInfo READ systemInfo NOTIFY systemInfoChanged)
    Q_PROPERTY(bool autoStartup READ autoStartup WRITE setAutoStartup NOTIFY autoStartupChanged)
    Q_PROPERTY(bool liveSession READ liveSession CONSTANT)
    Q_PROPERTY(bool showLiveUserInfo READ showLiveUserInfo CONSTANT)
    Q_PROPERTY(bool startOnAbout READ startOnAbout CONSTANT)
    Q_PROPERTY(QString version READ version CONSTANT)

public:
    explicit Backend(const QCommandLineParser &parser, QObject *parent = nullptr);

    [[nodiscard]] ActionModel *actions();
    [[nodiscard]] QString distroTitle() const;
    [[nodiscard]] QString distroVersion() const;
    [[nodiscard]] QString debianVersion() const;
    [[nodiscard]] QString desktopVersion() const;
    [[nodiscard]] QString supportedUntil() const;
    [[nodiscard]] QString termsSummary() const;
    [[nodiscard]] QString liveUserInfo() const;
    [[nodiscard]] QUrl headerSource() const;
    [[nodiscard]] QUrl logoSource() const;
    [[nodiscard]] QString systemInfo() const;
    [[nodiscard]] bool autoStartup() const;
    [[nodiscard]] bool liveSession() const;
    [[nodiscard]] bool showLiveUserInfo() const;
    [[nodiscard]] bool startOnAbout() const;
    [[nodiscard]] QString version() const;

    void setAutoStartup(bool enabled);

    Q_INVOKABLE void activate(const QString &identifier);
    Q_INVOKABLE void requestSystemInfo();
    Q_INVOKABLE void openFullSystemReport();
    Q_INVOKABLE QString termsText();
    Q_INVOKABLE QString licenseHtml() const;
    Q_INVOKABLE QString changelogText() const;

signals:
    void systemInfoChanged();
    void autoStartupChanged();
    void errorOccurred(const QString &title, const QString &message);

private:
    struct ActionDefinition {
        WelcomeAction action;
        QString command;
        QString defaultCommand;
        QUrl defaultUrl;
        bool visible = true;
    };

    static QString runCommand(const QString &program, const QStringList &arguments, int timeoutMs = 10000);
    static QString commandOutput(const QString &command);
    static QUrl localSource(const QString &path);
    void loadConfiguration(bool testMode);
    void rebuildActions();
    void launchCommand(const QString &command);
    [[nodiscard]] const ActionDefinition *findAction(const QString &identifier) const;

    ActionModel m_actions;
    QVector<ActionDefinition> m_definitions;
    QSettings m_userSettings;
    QString m_distroTitle;
    QString m_distroVersion;
    QString m_debianVersion;
    QString m_desktopVersion;
    QString m_supportedUntil;
    QString m_termsSummary;
    QString m_liveUserInfo;
    QString m_systemInfo;
    QString m_termsCommand;
    QUrl m_headerSource;
    QUrl m_logoSource;
    bool m_autoStartup = false;
    bool m_liveSession = false;
    bool m_showLiveUserInfo = false;
    bool m_startOnAbout = false;
    bool m_isFluxbox = false;
};

#pragma once

#include <QProcess>

class QTextStream;

class Cmd : public QProcess
{
    Q_OBJECT
public:
    explicit Cmd(QObject *parent = nullptr);

    enum class QuietMode { No, Yes };
    enum class Elevation { No, Yes };

    static QString elevationTool();

    [[nodiscard]] QString getOut(const QString &cmd, QuietMode quiet = QuietMode::No, Elevation elevation = Elevation::No);
    [[nodiscard]] QString getOutAsRoot(const QString &cmd, QuietMode quiet = QuietMode::No);
    bool run(const QString &cmd, QuietMode quiet = QuietMode::No, Elevation elevation = Elevation::No);
    bool runAsRoot(const QString &cmd, QuietMode quiet = QuietMode::No);

signals:
    void done();

private:
    QString cmdStr;
    QString elevate;
    QString helper;
};

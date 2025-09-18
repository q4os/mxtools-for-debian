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
    [[nodiscard]] QString readAllOutput() const;
    bool run(const QString &cmd, QuietMode quiet = QuietMode::No, Elevation elevation = Elevation::No);
    bool runAsRoot(const QString &cmd, QuietMode quiet = QuietMode::No);
    [[nodiscard]] bool terminateAndKill();

signals:
    void done();
    void errorAvailable(const QString &err);
    void outputAvailable(const QString &out);

private:
    QString elevate;
    QString helper;
    QString out_buffer;
};

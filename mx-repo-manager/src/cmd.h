#pragma once

#include <QProcess>

enum struct Elevation { No, Yes };
enum struct QuietMode { No, Yes };

class QTextStream;

class Cmd : public QProcess
{
    Q_OBJECT
public:
    explicit Cmd(QObject *parent = nullptr);

    bool proc(const QString &cmd, const QStringList &args = {}, QString *output = nullptr,
              const QByteArray *input = nullptr, QuietMode quiet = QuietMode::No, Elevation elevation = Elevation::No);
    bool procAsRoot(const QString &cmd, const QStringList &args = {}, QString *output = nullptr,
                    const QByteArray *input = nullptr, QuietMode quiet = QuietMode::No);
    bool startDetachedAsRoot(const QString &cmd, const QStringList &args = {}, QuietMode quiet = QuietMode::No,
                             const QString &logFilePath = {}, QString *errorMessage = nullptr);
    bool run(const QString &cmd, QString *output = nullptr, const QByteArray *input = nullptr,
             QuietMode quiet = QuietMode::No);
    [[nodiscard]] QString getOut(const QString &cmd, QuietMode quiet = QuietMode::No);
    [[nodiscard]] QString getOutAsRoot(const QString &cmd, const QStringList &args = {},
                                       QuietMode quiet = QuietMode::No);
    [[nodiscard]] QString readAllOutput() const;
    static QString shellQuote(const QString &arg);

signals:
    void done();
    void errorAvailable(const QString &err);
    void outputAvailable(const QString &out);

private slots:
    void handleStandardError();
    void handleStandardOutput();

private:
    QString elevationCommand;
    QString helper;
    QString outBuffer;

    static constexpr int EXIT_CODE_COMMAND_NOT_FOUND = 127;
    static constexpr int EXIT_CODE_PERMISSION_DENIED = 126;

    bool helperProc(const QStringList &helperArgs, QString *output = nullptr, const QByteArray *input = nullptr,
                    QuietMode quiet = QuietMode::No);
    void handleElevationError(int helperExitCode);
};

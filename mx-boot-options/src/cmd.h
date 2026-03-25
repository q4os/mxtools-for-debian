#pragma once

#include <QProcess>

class QTextStream;

enum struct Elevation { No, Yes };
enum struct QuietMode { No, Yes };

class Cmd : public QProcess
{
    Q_OBJECT
public:
    explicit Cmd(QObject *parent = nullptr);
    bool proc(const QString &cmd, const QStringList &args = {}, QString *output = nullptr,
              const QByteArray *input = nullptr, QuietMode quiet = QuietMode::No, Elevation elevation = Elevation::No);
    bool procAsRoot(const QString &cmd, const QStringList &args = {}, QString *output = nullptr,
                    const QByteArray *input = nullptr, QuietMode quiet = QuietMode::No);
    bool procAsRootInTarget(const QString &rootPath, const QString &cmd, const QStringList &args = {},
                            QString *output = nullptr, const QByteArray *input = nullptr,
                            QuietMode quiet = QuietMode::No);
    bool run(const QString &cmd, QString *output = nullptr, const QByteArray *input = nullptr,
             QuietMode quiet = QuietMode::No);
    [[nodiscard]] QString getOut(const QString &cmd, QuietMode quiet = QuietMode::No);
    [[nodiscard]] QString getOutAsRoot(const QString &cmd, const QStringList &args = {},
                                       QuietMode quiet = QuietMode::No);
    [[nodiscard]] QString getOutAsRootInTarget(const QString &rootPath, const QString &cmd,
                                               const QStringList &args = {}, QuietMode quiet = QuietMode::No);
    [[nodiscard]] QString readFileAsRoot(const QString &path, QuietMode quiet = QuietMode::No,
                                         const QString &rootPath = {});
    [[nodiscard]] bool isPackageInstalledAsRoot(const QString &manager, const QString &package,
                                                const QString &rootPath = {}, QuietMode quiet = QuietMode::No);
    bool appendToFileAsRootIfMissing(const QString &path, const QString &needle, const QString &content,
                                     QuietMode quiet = QuietMode::No, const QString &rootPath = {});
    bool previewPlymouthAsRoot(QuietMode quiet = QuietMode::No);

signals:
    void done();
    void errorAvailable(const QString &err);
    void outputAvailable(const QString &out);

private slots:
    void handleStandardError();
    void handleStandardOutput();

private:
    QString outBuffer;
    QString elevationCommand;
    QString helper;
    static constexpr int EXIT_CODE_COMMAND_NOT_FOUND = 127;
    static constexpr int EXIT_CODE_PERMISSION_DENIED = 126;

    bool helperProc(const QStringList &helperArgs, QString *output = nullptr, const QByteArray *input = nullptr,
                    QuietMode quiet = QuietMode::No);
    [[nodiscard]] QStringList helperExecArgs(const QString &cmd, const QStringList &args,
                                             const QString &rootPath = {}) const;
    [[nodiscard]] static QStringList helperRootArgs(const QString &rootPath);
    void handleElevationError();
};

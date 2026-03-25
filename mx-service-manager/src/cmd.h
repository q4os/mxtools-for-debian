
#ifndef CMD_H
#define CMD_H

#include <QProcess>
#include <QStringList>

class Cmd : public QProcess
{
    Q_OBJECT
public:
    explicit Cmd(QObject *parent = nullptr);

    [[nodiscard]] QString getOut(const QString &cmd, bool quiet = false, bool waitForFinish = false);
    [[nodiscard]] QString getOutAsRoot(const QStringList &helperArgs, bool quiet = false,
                                       bool waitForFinish = false);
    bool run(const QString &cmd, bool quiet = false, bool waitForFinish = false);
    bool runAsRoot(const QStringList &helperArgs, bool quiet = false, bool waitForFinish = false);

signals:
    void done();

private:
    QString elevate;
    QString helper;
    static constexpr int EXIT_CODE_COMMAND_NOT_FOUND = 127;
    static constexpr int EXIT_CODE_PERMISSION_DENIED = 126;

    void handleElevationError();
};

#endif // CMD_H

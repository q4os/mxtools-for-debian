#ifndef CMD_H
#define CMD_H

#include <QProcess>

class QString;
class QTextStream;

class Cmd : public QProcess
{
    Q_OBJECT
public:
    Cmd(QObject *parent = nullptr);
    QString getCmdOut(const QString &cmd, bool quiet = false);
    bool run(const QString &cmd, QString &output, bool quiet = false);
    bool run(const QString &cmd, bool quiet = false);

signals:
    void done();
};

#endif // CMD_H

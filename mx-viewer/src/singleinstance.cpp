/**********************************************************************
 *
 **********************************************************************
 * Copyright (C) 2023-2026 MX Authors
 *
 * Authors: Adrian <adrian@mxlinux.org>
 *          MX Linux <http://mxlinux.org>
 *
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this package. If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/
#include "singleinstance.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLocalServer>
#include <QLocalSocket>
#include <QLockFile>
#include <QTimer>
#include <QUrl>

#include <memory>

#include <sys/stat.h>
#include <unistd.h>

namespace
{
constexpr int connectTimeoutMs {1000};
// Longer, since the running instance answers from its GUI thread.
constexpr int ackTimeoutMs {3000};
constexpr char ack[] {"ok\n"};
constexpr qint64 maxRequestSize {64 * 1024};
constexpr int requestTimeoutMs {5000};

// One running browser per display, so a link opened in a second graphical session of the same user
// stays in that session.
QString socketName()
{
    QString display = qEnvironmentVariable("WAYLAND_DISPLAY");
    if (display.isEmpty()) {
        display = qEnvironmentVariable("DISPLAY");
        // ":0" and ":0.0" name the same display.
        const qsizetype colon = display.lastIndexOf(QLatin1Char(':'));
        const qsizetype dot = colon < 0 ? -1 : display.indexOf(QLatin1Char('.'), colon);
        if (dot > colon) {
            display.truncate(dot);
        }
    }
    // A hash keeps distinct names apart ("wayland.0" and "wayland_0") and the path within sun_path.
    const QByteArray hash = QCryptographicHash::hash(display.toUtf8(), QCryptographicHash::Sha256).toHex().left(16);
    return QStringLiteral("mx-viewer-%1.sock").arg(QString::fromLatin1(hash));
}

// The socket goes only in a private runtime directory owned by this user (as XDG requires, mode 0700),
// never in a shared place like /tmp where another user could create it first. Empty means
// single-instance mode is off.
QString socketPath()
{
    QStringList candidates;
    const QString runtimeDir = qEnvironmentVariable("XDG_RUNTIME_DIR");
    if (!runtimeDir.isEmpty()) {
        candidates << runtimeDir;
    }
    candidates << QStringLiteral("/run/user/%1").arg(getuid());
    for (const QString &dir : std::as_const(candidates)) {
        struct stat info {};
        if (stat(QFile::encodeName(dir).constData(), &info) == 0 && S_ISDIR(info.st_mode)
            && info.st_uid == getuid() && (info.st_mode & 077) == 0) {
            return QDir(dir).filePath(socketName());
        }
    }
    return {};
}

void readRequest(QLocalSocket *socket, const std::function<void(const QString &)> &handler)
{
    if (!socket->canReadLine()) {
        if (socket->bytesAvailable() > maxRequestSize) {
            socket->abort();
        }
        return;
    }
    // One request per connection.
    const QString argument = QString::fromUtf8(socket->readLine(maxRequestSize)).trimmed();
    // Tell the sender the request was taken, so it can exit; disconnecting flushes this first.
    socket->write(ack);
    socket->disconnectFromServer();
    handler(argument);
}
} // namespace

bool SingleInstance::forward(const QString &argument)
{
    const QString path = socketPath();
    if (path.isEmpty()) {
        return false;
    }
    QLocalSocket socket;
    socket.connectToServer(path);
    if (!socket.waitForConnected(connectTimeoutMs)) {
        return false;
    }
    // The running instance has a different working directory, so send local paths (files or
    // directories, as displaySite() accepts) as absolute URLs.
    QString request = argument;
    const QFileInfo file(argument);
    if (!argument.isEmpty() && file.exists()) {
        request = QUrl::fromLocalFile(file.absoluteFilePath()).toString(QUrl::FullyEncoded);
    }
    request.remove(QLatin1Char('\n'));
    socket.write(request.toUtf8() + '\n');
    if (!socket.waitForBytesWritten(connectTimeoutMs)) {
        return false;
    }
    // A connection can succeed through the listen backlog even if the instance is hung, so only its
    // answer counts as delivered.
    while (!socket.canReadLine()) {
        if (!socket.waitForReadyRead(ackTimeoutMs)) {
            return false;
        }
    }
    return socket.readLine() == ack;
}

void SingleInstance::listen(QObject *parent, const std::function<void(const QString &)> &handler)
{
    const QString path = socketPath();
    if (path.isEmpty()) {
        return;
    }
    // With UserAccessOption, listen() renames its socket over an existing one, so it would silently take
    // the path from a live instance that was just slow to answer. The lock decides who serves; it is
    // held until exit; a lock left by a dead process is detected by its PID and taken over.
    static std::unique_ptr<QLockFile> lock;
    if (lock) {
        return;
    }
    auto candidate = std::make_unique<QLockFile>(path + QStringLiteral(".lock"));
    candidate->setStaleLockTime(0);
    if (!candidate->tryLock(0)) {
        return;
    }
    lock = std::move(candidate);
    // Holding the lock means any leftover socket file is from an instance that is gone.
    QLocalServer::removeServer(path);
    auto *server = new QLocalServer(parent);
    server->setSocketOptions(QLocalServer::UserAccessOption);
    if (!server->listen(path)) {
        delete server;
        lock.reset(); // Nobody is serving, so don't keep later launches out.
        return;
    }
    QObject::connect(server, &QLocalServer::newConnection, server, [server, handler] {
        while (QLocalSocket *socket = server->nextPendingConnection()) {
            QObject::connect(socket, &QLocalSocket::disconnected, socket, &QObject::deleteLater);
            // A client that connects and never finishes its request would otherwise stay open forever.
            QTimer::singleShot(requestTimeoutMs, socket, [socket] {
                socket->abort();
                socket->deleteLater();
            });
            QObject::connect(socket, &QLocalSocket::readyRead, socket, [socket, handler] { readRequest(socket, handler); });
            readRequest(socket, handler);
        }
    });
}

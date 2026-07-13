#pragma once

#include <QString>
#include <QStringList>

namespace CardUtils
{

struct PipeWireSink {
    QString name;        // internal pactl name (used for set-default-sink)
    QString description; // human-readable label
};

QStringList parseCards(const QString &procAsoundCards);
QString parseDefaultCard(const QString &asoundrc);
bool isPipewireRunning();
QList<PipeWireSink> parsePipewireSinks(const QString &paListSinksOutput);

} // namespace CardUtils

#pragma once

#include "cmd.h"

#include <QPlainTextEdit>
#include <QScrollBar>
#include <QTextBlock>
#include <QTextCursor>

namespace MainWindowHelpers
{
inline constexpr auto MxpiLibPath = "/usr/lib/mx-packageinstaller/mxpi-lib";

inline void appendFlatpakStatusMessage(QPlainTextEdit *outputBox, const QString &message)
{
    if (!outputBox) {
        return;
    }

    QTextCursor cursor = outputBox->textCursor();
    cursor.movePosition(QTextCursor::End);
    if (outputBox->document()->characterCount() > 1) {
        const QString lastLine = outputBox->document()->lastBlock().text();
        if (!lastLine.isEmpty()) {
            cursor.insertText("\n");
        }
    }
    cursor.insertText(message + "\n");
    outputBox->setTextCursor(cursor);
    outputBox->verticalScrollBar()->setValue(outputBox->verticalScrollBar()->maximum());
}

inline bool runScriptAsRoot(Cmd &cmd, const char *scriptPath, const QString &action, Cmd::QuietMode quiet)
{
    const QString path = QString::fromLatin1(scriptPath);
    return cmd.procScriptAsRoot(path, {action}, nullptr, nullptr, quiet);
}

inline bool runMxpiLibAsRoot(Cmd &cmd, const QString &action, Cmd::QuietMode quiet = Cmd::QuietMode::Yes)
{
    return runScriptAsRoot(cmd, MxpiLibPath, action, quiet);
}
} // namespace MainWindowHelpers

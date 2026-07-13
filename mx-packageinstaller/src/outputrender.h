/**********************************************************************
 *  outputrender.h
 **********************************************************************
 * Copyright (C) 2026 MX Authors
 *
 * Authors: Adrian
 *          MX Linux <http://mxlinux.org>
 *
 * This program is free software: you can redistribute it and/or modify
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

#pragma once

#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QScrollBar>
#include <QString>
#include <QStringList>
#include <QTextBlock>
#include <QTextCursor>

// Rendering helpers for the Output tab. Kept header-only so the carriage-return /
// progress handling can be exercised directly by unit tests without pulling in the
// whole MainWindow.
namespace OutputRender
{

// Strip the ANSI escape and query sequences that command-line tools emit when they
// believe they are writing to a terminal (colors, cursor moves, status reports).
inline QString sanitizeOutputForDisplay(const QString &output)
{
    static const QRegularExpression ansiEscape {R"(\x1B\[[0-9;?]*[A-Za-z])"};
    static const QRegularExpression ansiQuery {R"(\x1B\[[0-9;?]*n)"};
    QString cleanOutput = output;
    cleanOutput.remove(ansiEscape);
    cleanOutput.remove(ansiQuery);
    return cleanOutput;
}

// Minimal terminal-style renderer for a QPlainTextEdit. Tools like snap and flatpak
// redraw a progress line in place with a bare carriage return (no newline), once per
// poll. That output arrives split across many readyRead chunks, so the cursor state
// (which column the next character overwrites) has to persist across calls -- otherwise
// every redraw is appended as a new line and the box fills with hundreds of copies of
// the same line.
//
//   '\n'  finalizes the current line and starts a new one
//   '\r'  moves the cursor back to column 0 of the current line (overwrite in place)
//   else  writes at the cursor, overwriting existing characters or extending the line
class OutputRenderer
{
public:
    OutputRenderer() = default;
    explicit OutputRenderer(QPlainTextEdit *box)
        : outputBox(box)
    {
    }

    void setOutputBox(QPlainTextEdit *box)
    {
        outputBox = box;
        reset();
    }

    // Forget the in-progress line. Call after clearing the box or editing it directly
    // (the next append() also re-syncs from the document, so this is belt-and-braces).
    void reset()
    {
        currentLine.clear();
        cursorColumn = 0;
    }

    void append(const QString &output)
    {
        if (!outputBox || output.isEmpty()) {
            return;
        }

        // Re-sync with the document if something else edited the box since the last
        // append (status messages, a clear(), the core-install retry truncation). During
        // normal streaming the last block already equals currentLine, so this is a no-op
        // and the cursor column persists across chunks.
        const QString lastBlockText = outputBox->document()->lastBlock().text();
        if (lastBlockText != currentLine) {
            currentLine = lastBlockText;
            cursorColumn = currentLine.size();
        }

        QStringList completedLines;
        const int size = output.size();
        for (int i = 0; i < size; ++i) {
            const QChar ch = output.at(i);
            if (ch == QChar(0x1B)) { // ESC: consume an ANSI control sequence
                i = consumeEscape(output, i);
                continue;
            }
            if (ch == QLatin1Char('\n')) {
                completedLines.append(currentLine);
                currentLine.clear();
                cursorColumn = 0;
            } else if (ch == QLatin1Char('\r')) {
                cursorColumn = 0;
            } else {
                if (cursorColumn < currentLine.size()) {
                    currentLine[cursorColumn] = ch;
                } else {
                    currentLine.append(ch);
                }
                ++cursorColumn;
            }
        }

        // The document's last block still holds the previous currentLine. Replace it
        // with the lines completed in this chunk followed by the live current line.
        QTextCursor cursor(outputBox->document());
        cursor.movePosition(QTextCursor::End);
        cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        QString replacement;
        for (const QString &line : completedLines) {
            replacement += line;
            replacement += QLatin1Char('\n');
        }
        replacement += currentLine;
        cursor.insertText(replacement);

        outputBox->verticalScrollBar()->setValue(outputBox->verticalScrollBar()->maximum());
    }

private:
    // Handle the ANSI escape sequence starting at escIndex (output[escIndex] == ESC).
    // Honors "erase in line" (CSI K), which tools use to clear a progress line before
    // printing a shorter final line; everything else (colors, cursor show/hide, status
    // reports) is skipped. Returns the index of the last consumed character.
    int consumeEscape(const QString &output, int escIndex)
    {
        const int size = output.size();
        if (escIndex + 1 >= size || output.at(escIndex + 1) != QLatin1Char('[')) {
            return escIndex; // lone ESC or non-CSI escape: drop just the ESC
        }
        int j = escIndex + 2;
        while (j < size) {
            const QChar c = output.at(j);
            if ((c >= QLatin1Char('0') && c <= QLatin1Char('9')) || c == QLatin1Char(';')
                || c == QLatin1Char('?')) {
                ++j;
            } else {
                break;
            }
        }
        if (j >= size) {
            return size - 1; // sequence split across chunks: drop what we have
        }
        const QChar finalByte = output.at(j);
        const QString params = output.mid(escIndex + 2, j - (escIndex + 2));
        if (finalByte == QLatin1Char('K')) {
            if (params.isEmpty() || params == QLatin1String("0")) {
                currentLine.truncate(cursorColumn); // erase from cursor to end of line
            } else if (params == QLatin1String("2")) {
                currentLine.clear(); // erase the whole line
                cursorColumn = 0;
            }
        }
        return j;
    }

    QPlainTextEdit *outputBox = nullptr;
    QString currentLine;
    int cursorColumn = 0;
};

} // namespace OutputRender

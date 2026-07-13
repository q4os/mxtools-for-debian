#include <QPlainTextEdit>
#include <QtTest/QtTest>

#include "src/outputrender.h"

// Verifies the Output-tab rendering logic, in particular how carriage returns from
// terminal-style tools (flatpak, snap) are collapsed instead of stacking -- including
// when each progress redraw arrives as its own streamed chunk.
class TestOutputRender : public QObject
{
    Q_OBJECT

private:
    // Render all chunks through a single renderer (state persists across chunks, like
    // the live process output signal) and return the resulting plain text.
    static QString render(const QStringList &chunks)
    {
        QPlainTextEdit box;
        OutputRender::OutputRenderer renderer(&box);
        for (const QString &chunk : chunks) {
            renderer.append(chunk);
        }
        return box.toPlainText();
    }

private slots:
    void plainCrlfLines()
    {
        QCOMPARE(render({"one\r\ntwo\r\nthree\r\n"}), QString("one\ntwo\nthree\n"));
    }

    void plainLfLines()
    {
        QCOMPARE(render({"alpha\nbeta\n"}), QString("alpha\nbeta\n"));
    }

    void crlfSplitAcrossChunks()
    {
        // The "\r" and "\n" of a line ending can land in different read chunks.
        QCOMPARE(render({"first\r", "\nsecond\r", "\n"}), QString("first\nsecond\n"));
    }

    void snapProgressCollapsesInOneChunk()
    {
        const QString stream = "\rcore 10% 1MB/s\rcore 45% 1MB/s\rcore 99% 1MB/s\r"
                               "core 16-2.61 from Canonical installed\n";
        const QString result = render({stream});
        QCOMPARE(result, QString("core 16-2.61 from Canonical installed\n"));
        QVERIFY(!result.contains("10%"));
        QVERIFY(!result.contains("45%"));
    }

    void snapProgressEachRedrawSeparateChunk()
    {
        // This is the real-world case: snap redraws its progress line ~10x/second and
        // each redraw is delivered as its own chunk. They must collapse to one line, not
        // pile up. Leading carriage return per chunk.
        QStringList chunks;
        for (int pct = 0; pct <= 100; pct += 5) {
            chunks << QStringLiteral("\rDownload snap \"core\"        %1%").arg(pct);
        }
        // snap clears the (longer) progress line with CSI K before the final line.
        chunks << "\rcore installed\x1B[K\r\n";
        const QString result = render(chunks);
        QCOMPARE(result, QString("core installed\n"));
        QVERIFY(!result.contains("50%"));
        QVERIFY(!result.contains("95%"));
    }

    void snapProgressTrailingCarriageReturnChunks()
    {
        // Same redraws, but with the carriage return at the END of each chunk (the read
        // boundary can fall either side of the "\r"). State must persist so the next
        // chunk overwrites from column 0.
        const QString result = render({
            "Mount snap \"core\"   |\r",
            "Mount snap \"core\"   /\r",
            "Mount snap \"core\"   -\r",
            "Mount snap \"core\" done\r\n",
        });
        QCOMPARE(result, QString("Mount snap \"core\" done\n"));
    }

    void snapProgressWithAnsiAndChunking()
    {
        const QStringList chunks {
            "\x1B[?25l\rDownloading core 12%\x1B[K",
            "\rDownloading core 58%\x1B[K",
            "\rDownloading core 100%\x1B[K",
            "\rcore installed\x1B[K\r\n\x1B[?25h",
        };
        QCOMPARE(render(chunks), QString("core installed\n"));
    }

    void flatpakProgressStillCollapses()
    {
        const QString stream = "\rInstalling 1/2… 8%\rInstalling 1/2… 53%\rInstalling 1/2… 100%\n";
        QCOMPARE(render({stream}), QString("Installing 1/2… 100%\n"));
    }

    void flatpakProgressAfterNormalLines()
    {
        const QStringList chunks {
            "Required runtime for org.gnome.Calculator\r\n",
            "\rInstalling 1/2… 10%\rInstalling 1/2… 90%\rInstalling 1/2… 100%\r\n",
        };
        QCOMPARE(render(chunks),
                 QString("Required runtime for org.gnome.Calculator\nInstalling 1/2… 100%\n"));
    }

    void progressFollowedByResultLine()
    {
        // A status line, a collapsing progress bar, then a final result line.
        const QStringList chunks {
            "Setting up core snap\r\n",
            "\rmounting 20%",
            "\rmounting 70%",
            "\rmounting done\r\n",
            "core installed\n",
        };
        QCOMPARE(render(chunks), QString("Setting up core snap\nmounting done\ncore installed\n"));
    }

    void recoversAfterExternalEdit()
    {
        // The renderer must tolerate the box being edited behind its back (status
        // messages, the core-install retry truncation) and keep collapsing afterwards.
        QPlainTextEdit box;
        OutputRender::OutputRenderer renderer(&box);
        renderer.append("\rDownload 10%");
        renderer.append("\rDownload 80%");

        // Simulate an external edit: append a status line directly to the box.
        QTextCursor cursor = box.textCursor();
        cursor.movePosition(QTextCursor::End);
        cursor.insertText("\nRetrying...\n");

        renderer.append("\rDownload 20%");
        renderer.append("\rDownload 100%");
        renderer.append("\rdone\x1B[K\r\n");
        QCOMPARE(box.toPlainText(), QString("Download 80%\nRetrying...\ndone\n"));
    }
};

QTEST_MAIN(TestOutputRender)
#include "test_outputrender.moc"

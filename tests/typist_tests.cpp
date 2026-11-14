#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QtTest>

#include "backend.h"
#include "theme.h"
#include "translator.h"

class TypistTests : public QObject {
    Q_OBJECT

private slots:
    void typingMarksCharacters()
    {
        Backend backend;
        backend.setTargetForTest("abc");
        backend.type("ax");
        QCOMPARE(backend.states().toList(), QVariantList({1, 2, 0}));
        QCOMPARE(backend.cursor(), 2);
        QCOMPARE(backend.errors(), 1);
        QCOMPARE(backend.accuracy(), 50);
        QVERIFY(backend.started());
        QVERIFY(!backend.finished());
    }

    void backspaceReverts()
    {
        Backend backend;
        backend.setTargetForTest("abc");
        backend.type("ax");
        backend.backspace();
        QCOMPARE(backend.states().toList(), QVariantList({1, 0, 0}));
        QCOMPARE(backend.cursor(), 1);
        QCOMPARE(backend.errors(), 1); // the typo counts even after erasing it
        QCOMPARE(backend.accuracy(), 50);
    }

    void finishingStopsInput()
    {
        Backend backend;
        backend.setTargetForTest("ab");
        backend.type("ab");
        QVERIFY(backend.finished());
        const int wpm = backend.wpm();
        QTest::qWait(1100);
        QCOMPARE(backend.wpm(), wpm); // final WPM is frozen
        backend.type("zzz");
        QCOMPARE(backend.cursor(), 2); // extra input is ignored
    }

    void restartKeepsText()
    {
        Backend backend;
        backend.setTargetForTest("abc");
        backend.type("ab");
        backend.restart();
        QCOMPARE(backend.target(), QString("abc"));
        QCOMPARE(backend.cursor(), 0);
        QCOMPARE(backend.accuracy(), 100);
        QVERIFY(!backend.started());
    }

    void nextTextStaysInSet()
    {
        Backend backend;
        backend.setTargetForTest("abc");
        backend.switchSet(2);
        const QString first = backend.target();
        backend.nextText();
        QVERIFY(!backend.target().isEmpty());
        QCOMPARE(backend.setIndex(), 2);
        backend.switchSet(0);
        QCOMPARE(backend.setIndex(), 0);
        QVERIFY(!backend.target().isEmpty());
        Q_UNUSED(first);
    }

    void themeFallsBack()
    {
        QTemporaryDir dir;
        Theme theme(dir.path());
        QCOMPARE(theme.accent(), QString("#FFD60A"));
        QCOMPARE(theme.accentForeground(), QString("black"));
    }

    void themeReadsAccent()
    {
        QTemporaryDir dir;
        QDir(dir.path()).mkpath("theme");
        QFile colors(dir.path() + "/theme/colors.toml");
        QVERIFY(colors.open(QIODevice::WriteOnly));
        colors.write("accent = \"#112233\"\n");
        colors.close();

        Theme theme(dir.path());
        QCOMPARE(theme.accent(), QString("#112233"));
        QCOMPARE(theme.accentForeground(), QString("white"));
    }

    void translatorFallsBackToEnglish()
    {
        QTemporaryDir dir;
        Translator translator(dir.path() + "/language", dir.path());
        QCOMPARE(translator.language(), QString("en"));
        QVERIFY(translator.translate("Main", "Words").isNull());
    }

    void translatorReadsChinese()
    {
        QTemporaryDir dir;
        write(dir.path() + "/language", "zh-CN\n");
        write(dir.path() + "/zh-CN.json", R"({"Words": "单词"})");

        Translator translator(dir.path() + "/language", dir.path());
        QCOMPARE(translator.language(), QString("zh-CN"));
        QCOMPARE(translator.translate("Main", "Words"), QString("单词"));
        QVERIFY(translator.translate("Main", "Code").isNull());
    }

private:
    static void write(const QString &path, const QByteArray &contents)
    {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(contents);
    }
};

QTEST_MAIN(TypistTests)
#include "typist_tests.moc"

#pragma once

#include <QElapsedTimer>
#include <QObject>
#include <QTimer>
#include <QVariantList>

// The typing drill: a target text, per-character correctness, and live WPM /
// accuracy stats. QML feeds key events through type() and backspace(); all
// scoring lives here so tests can drive it without a window.
class Backend : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString target READ target NOTIFY roundChanged)
    Q_PROPERTY(QVariantList states READ states NOTIFY statesChanged)
    Q_PROPERTY(int cursor READ cursor NOTIFY statesChanged)
    Q_PROPERTY(int setIndex READ setIndex NOTIFY setChanged)
    Q_PROPERTY(int wpm READ wpm NOTIFY statsChanged)
    Q_PROPERTY(int accuracy READ accuracy NOTIFY statsChanged)
    Q_PROPERTY(int errors READ errors NOTIFY statsChanged)
    Q_PROPERTY(int seconds READ seconds NOTIFY statsChanged)
    Q_PROPERTY(bool started READ started NOTIFY statsChanged)
    Q_PROPERTY(bool finished READ finished NOTIFY finishedChanged)

public:
    explicit Backend(QObject *parent = nullptr);

    QString target() const { return m_target; }
    QVariantList states() const;
    int cursor() const { return m_cursor; }
    int setIndex() const { return m_setIndex; }
    int wpm() const;
    int accuracy() const;
    int errors() const { return m_errorKeys; }
    int seconds() const;
    bool started() const { return m_started; }
    bool finished() const { return m_finished; }

    Q_INVOKABLE void type(const QString &text);
    Q_INVOKABLE void backspace();
    Q_INVOKABLE void nextText();
    Q_INVOKABLE void restart();
    Q_INVOKABLE void switchSet(int index);

    // Tests set the target directly so they don't depend on the built-in lists.
    void setTargetForTest(const QString &target);

signals:
    void roundChanged();
    void statesChanged();
    void setChanged();
    void statsChanged();
    void finishedChanged();

private:
    enum CharState { Pending, Correct, Wrong };

    void reset();
    void loadText();
    double elapsedSeconds() const;

    static const QStringList s_sets[3];

    QString m_target;
    QVector<int> m_states;
    int m_cursor = 0;
    int m_setIndex = 0;
    int m_lastTextIndex = -1;
    int m_keystrokes = 0;
    int m_errorKeys = 0;
    int m_correctChars = 0;
    bool m_started = false;
    bool m_finished = false;
    QElapsedTimer m_timer;
    double m_finalSeconds = 0;
    QTimer m_ticker;
};

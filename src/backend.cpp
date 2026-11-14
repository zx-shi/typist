#include "backend.h"

#include <QRandomGenerator>

// Practice texts: common words, plain sentences, and one-line code snippets.
// Kept to one line each so the drill never needs Enter.
const QStringList Backend::s_sets[3] = {
    // Words
    {
        QStringLiteral("the quick brown fox jumps over the lazy dog"),
        QStringLiteral("time code learn type fast focus keep going"),
        QStringLiteral("keyboard practice makes perfect every day"),
        QStringLiteral("hello world open source free software power"),
        QStringLiteral("arch linux simple light fast rolling release"),
        QStringLiteral("think less type more trust your fingers"),
        QStringLiteral("system network window buffer signal thread"),
        QStringLiteral("garden river mountain sunset window bridge"),
        QStringLiteral("orange purple silver shadow crystal melody"),
        QStringLiteral("debug commit push pull merge branch review"),
    },
    // Sentences
    {
        QStringLiteral("The five boxing wizards jump quickly."),
        QStringLiteral("How vexingly quick daft zebras jump!"),
        QStringLiteral("Pack my box with five dozen liquor jugs."),
        QStringLiteral("Small steps every day beat one big leap."),
        QStringLiteral("A clean desk is a sign of a calm mind."),
        QStringLiteral("Good tools make hard work feel lighter."),
        QStringLiteral("Slow is smooth, and smooth is fast."),
        QStringLiteral("Read the error message twice, then fix it."),
        QStringLiteral("Every expert was once a beginner."),
        QStringLiteral("Make it work, make it right, make it fast."),
    },
    // Code
    {
        QStringLiteral("int main() { return 0; }"),
        QStringLiteral("for (int i = 0; i < 10; ++i)"),
        QStringLiteral("if (x > 0 && y != z) { ok = true; }"),
        QStringLiteral("def hello(name): return f\"hi {name}\""),
        QStringLiteral("import os, sys; print(sys.path[0])"),
        QStringLiteral("const add = (a, b) => a + b;"),
        QStringLiteral("git commit -m \"fix the thing\""),
        QStringLiteral("ls -lah /var/log | grep error"),
        QStringLiteral("sudo systemctl restart NetworkManager"),
        QStringLiteral("std::vector<int> v{1, 2, 3, 4, 5};"),
    },
};

Backend::Backend(QObject *parent) : QObject(parent) {
    m_ticker.setInterval(500);
    // Live stats keep moving between keystrokes, and stop at the last char.
    connect(&m_ticker, &QTimer::timeout, this, &Backend::statsChanged);
    loadText();
}

QVariantList Backend::states() const {
    QVariantList list;
    list.reserve(m_states.size());
    for (int state : m_states)
        list.append(state);
    return list;
}

double Backend::elapsedSeconds() const {
    if (m_finished)
        return m_finalSeconds;
    return m_started ? m_timer.elapsed() / 1000.0 : 0.0;
}

int Backend::seconds() const {
    return static_cast<int>(elapsedSeconds());
}

int Backend::wpm() const {
    const double minutes = elapsedSeconds() / 60.0;
    if (minutes < 1.0 / 600.0)
        return 0;
    return static_cast<int>((m_correctChars / 5.0) / minutes + 0.5);
}

int Backend::accuracy() const {
    if (m_keystrokes == 0)
        return 100;
    return 100 * (m_keystrokes - m_errorKeys) / m_keystrokes;
}

void Backend::type(const QString &text) {
    if (m_finished || text.isEmpty())
        return;
    for (const QChar ch : text) {
        if (m_cursor >= m_target.size())
            break;
        if (!m_started) {
            m_started = true;
            m_timer.start();
            m_ticker.start();
        }
        ++m_keystrokes;
        if (ch == m_target.at(m_cursor)) {
            m_states[m_cursor] = Correct;
            ++m_correctChars;
        } else {
            m_states[m_cursor] = Wrong;
            ++m_errorKeys;
        }
        ++m_cursor;
    }
    emit statesChanged();
    emit statsChanged();
    if (m_cursor >= m_target.size()) {
        m_finished = true;
        m_finalSeconds = m_timer.elapsed() / 1000.0;
        m_ticker.stop();
        emit finishedChanged();
        emit statsChanged();
    }
}

void Backend::backspace() {
    if (m_finished || m_cursor == 0)
        return;
    --m_cursor;
    if (m_states[m_cursor] == Correct)
        --m_correctChars;
    m_states[m_cursor] = Pending;
    // Keystroke and error counters keep history: erasing a typo fixes the
    // text, but the slip still counts toward final accuracy.
    emit statesChanged();
    emit statsChanged();
}

void Backend::reset() {
    m_states.fill(Pending, m_target.size());
    m_cursor = 0;
    m_keystrokes = 0;
    m_errorKeys = 0;
    m_correctChars = 0;
    m_started = false;
    m_finished = false;
    m_finalSeconds = 0;
    m_ticker.stop();
}

void Backend::loadText() {
    const QStringList &texts = s_sets[m_setIndex];
    int index = QRandomGenerator::global()->bounded(texts.size());
    // Back-to-back repeats feel like a bug, so re-roll once on a collision.
    if (texts.size() > 1 && index == m_lastTextIndex)
        index = (index + 1) % texts.size();
    m_lastTextIndex = index;
    m_target = texts.at(index);
    reset();
    emit roundChanged();
    emit statesChanged();
    emit statsChanged();
    emit finishedChanged();
}

void Backend::setTargetForTest(const QString &target) {
    m_target = target;
    reset();
    emit roundChanged();
}

void Backend::nextText() {
    loadText();
}

void Backend::restart() {
    reset();
    emit statesChanged();
    emit statsChanged();
    emit finishedChanged();
}

void Backend::switchSet(int index) {
    if (index < 0 || index > 2 || index == m_setIndex)
        return;
    m_setIndex = index;
    emit setChanged();
    loadText();
}

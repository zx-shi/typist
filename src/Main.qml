import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

ApplicationWindow {
    id: win
    width: 960
    height: 680
    minimumWidth: 640
    minimumHeight: 460
    visible: true
    title: "Typist"
    color: "#0e0e10"

    Material.theme: Material.Dark
    Material.accent: theme.accent
    Material.roundedScale: theme.radius

    property bool helpVisible: false

    // Right-to-left languages mirror the window; the Translator sets the
    // application's layout direction.
    Item {
        id: root
        anchors.fill: parent
        LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
        LayoutMirroring.childrenInherit: true
        focus: true

        // Every plain key is drill input, so all routing lives here instead of
        // Shortcut items (a "Q" shortcut would fire while practicing words).
        Keys.onPressed: function(event) {
            if (event.key === Qt.Key_Escape) {
                if (win.helpVisible) win.helpVisible = false
                else Qt.quit()
                event.accepted = true
                return
            }
            if (event.key === Qt.Key_F1) {
                win.helpVisible = !win.helpVisible
                event.accepted = true
                return
            }
            if (win.helpVisible) {
                event.accepted = true
                return
            }
            if (event.modifiers & Qt.ControlModifier) {
                if (event.key === Qt.Key_R) backend.restart()
                else if (event.key === Qt.Key_Q) Qt.quit()
                else if (event.key >= Qt.Key_1 && event.key <= Qt.Key_3)
                    backend.switchSet(event.key - Qt.Key_1)
                event.accepted = true
                return
            }
            if (event.key === Qt.Key_Backspace) {
                backend.backspace()
                event.accepted = true
                return
            }
            if (event.key === Qt.Key_Tab) {
                backend.switchSet((backend.setIndex + 1) % 3)
                event.accepted = true
                return
            }
            if (backend.finished) {
                if (event.key === Qt.Key_Space || event.key === Qt.Key_Return || event.key === Qt.Key_Enter)
                    backend.nextText()
                event.accepted = true
                return
            }
            if (event.text.length > 0 && !(event.modifiers & Qt.AltModifier))
                backend.type(event.text)
            event.accepted = true
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 28
            spacing: 20

            // Top bar: set picker on the left, live stats on the right.
            RowLayout {
                Layout.fillWidth: true
                spacing: 18

                Repeater {
                    model: [qsTr("Words"), qsTr("Sentences"), qsTr("Code")]
                    delegate: Rectangle {
                        radius: Math.min(6, theme.radius)
                        color: index === backend.setIndex ? theme.accent : "#1a1a1e"
                        implicitWidth: setLabel.implicitWidth + 24
                        implicitHeight: setLabel.implicitHeight + 12
                        Text {
                            id: setLabel
                            anchors.centerIn: parent
                            text: modelData
                            font.pixelSize: 15
                            color: index === backend.setIndex ? theme.accentForeground : "#9a9aa2"
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: backend.switchSet(index)
                        }
                    }
                }

                Item { Layout.fillWidth: true }

                Repeater {
                    model: [
                        { label: qsTr("WPM"), value: backend.wpm },
                        { label: qsTr("Accuracy"), value: backend.accuracy + "%" },
                        { label: qsTr("Time"), value: backend.seconds + "s" },
                        { label: qsTr("Errors"), value: backend.errors },
                    ]
                    delegate: Column {
                        spacing: 2
                        Text {
                            text: modelData.label
                            font.pixelSize: 11
                            color: "#6a6a72"
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                        Text {
                            text: modelData.value
                            font.pixelSize: 20
                            font.bold: true
                            color: "#e6e6ea"
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: "#232328"
            }

            // The drill text, one item per character so each can be colored.
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                Flow {
                    id: drill
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 0

                    Repeater {
                        model: backend.target.length
                        delegate: Item {
                            property int charState: backend.states[index]
                            width: charText.implicitWidth
                            height: charText.implicitHeight + 6

                            Rectangle {
                                anchors.fill: parent
                                color: charState === 2 ? "#3d1a24" : "transparent"
                                radius: Math.min(4, theme.radius)
                            }
                            Text {
                                id: charText
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: backend.target[index]
                                font.family: "monospace"
                                font.pixelSize: 26
                                color: charState === 1 ? "#e6e6ea"
                                     : charState === 2 ? "#f7768e"
                                     : "#4a4a52"
                            }
                            Rectangle {
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.bottom: parent.bottom
                                height: 3
                                radius: 2
                                color: theme.accent
                                visible: index === backend.cursor && !backend.finished
                            }
                        }
                    }
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: drill.bottom
                    anchors.topMargin: 32
                    visible: !backend.started && !backend.finished
                    text: qsTr("Start typing — no need to click anything")
                    font.pixelSize: 14
                    color: "#6a6a72"
                }
            }

            Text {
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("Tab: switch set · Ctrl+R: restart · F1: shortcuts · Esc: quit")
                font.pixelSize: 12
                color: "#4a4a52"
            }
        }

        // Finished panel: final score, Space continues.
        Rectangle {
            anchors.centerIn: parent
            visible: backend.finished
            width: finishedColumn.implicitWidth + 64
            height: finishedColumn.implicitHeight + 48
            radius: theme.radius
            color: "#16161a"
            border.color: theme.accent
            border.width: 1

            Column {
                id: finishedColumn
                anchors.centerIn: parent
                spacing: 10

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("Done!")
                    font.pixelSize: 22
                    font.bold: true
                    color: theme.accent
                }
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("%1 WPM · %2% accuracy · %3s")
                            .arg(backend.wpm).arg(backend.accuracy).arg(backend.seconds)
                    font.pixelSize: 17
                    color: "#e6e6ea"
                }
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("Space for the next text")
                    font.pixelSize: 13
                    color: "#9a9aa2"
                }
            }
        }

        // Help overlay: every key, closed with F1 or Esc.
        Rectangle {
            anchors.fill: parent
            visible: win.helpVisible
            color: "#d80e0e10"

            MouseArea { anchors.fill: parent; onClicked: win.helpVisible = false }

            Rectangle {
                anchors.centerIn: parent
                width: helpColumn.implicitWidth + 64
                height: helpColumn.implicitHeight + 48
                radius: theme.radius
                color: "#16161a"
                border.color: "#2a2a30"
                border.width: 1

                Column {
                    id: helpColumn
                    anchors.centerIn: parent
                    spacing: 8

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: qsTr("Shortcuts")
                        font.pixelSize: 20
                        font.bold: true
                        color: theme.accent
                        bottomPadding: 8
                    }
                    Repeater {
                        model: [
                            [qsTr("Type"), qsTr("fill in the text")],
                            [qsTr("Backspace"), qsTr("erase a character")],
                            ["Tab", qsTr("switch practice set")],
                            ["Ctrl+1/2/3", qsTr("jump to a set")],
                            ["Ctrl+R", qsTr("restart this text")],
                            ["Space", qsTr("next text when done")],
                            ["F1", qsTr("toggle this help")],
                            ["Esc", qsTr("close / quit")],
                        ]
                        delegate: Row {
                            spacing: 16
                            Text {
                                text: modelData[0]
                                font.pixelSize: 14
                                font.bold: true
                                color: "#e6e6ea"
                                width: 110
                            }
                            Text {
                                text: modelData[1]
                                font.pixelSize: 14
                                color: "#9a9aa2"
                            }
                        }
                    }
                }
            }
        }
    }
}

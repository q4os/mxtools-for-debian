import QtQuick
import QtQuick.Controls

ToolTip {
    id: control

    SystemPalette { id: systemPalette }

    contentItem: Text {
        text: control.text
        color: systemPalette.windowText
        font: control.font
        wrapMode: Text.Wrap
    }

    background: Rectangle {
        color: systemPalette.window
        radius: 6
        border.width: 1
        border.color: Qt.alpha(systemPalette.windowText, 0.25)
    }
}

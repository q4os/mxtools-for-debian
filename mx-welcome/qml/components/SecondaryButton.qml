import QtQuick
import QtQuick.Controls

Button {
    id: control

    required property color textColor
    required property color surfaceColor
    required property color hoverColor
    required property color borderColor
    required property color accentColor

    hoverEnabled: true
    leftPadding: 14
    rightPadding: 14
    topPadding: 8
    bottomPadding: 8

    HoverHandler { cursorShape: Qt.PointingHandCursor }

    contentItem: Text {
        text: control.text
        color: control.textColor
        font: control.font
        elide: Text.ElideRight
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle {
        radius: 6
        color: control.down ? Qt.darker(control.hoverColor, 1.08)
                            : (control.hovered ? control.hoverColor : control.surfaceColor)
        border.width: control.activeFocus ? 2 : 1
        border.color: control.activeFocus ? control.accentColor : control.borderColor
    }

    Accessible.name: text
    Accessible.role: Accessible.Button
}

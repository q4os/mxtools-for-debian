import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

AbstractButton {
    id: card

    required property string actionId
    required property string title
    required property string description
    required property url iconSource
    required property color surfaceColor
    required property color primaryTextColor
    required property color mutedTextColor
    required property color accentColor
    required property color accentWash
    required property color borderColor

    hoverEnabled: true
    focusPolicy: Qt.StrongFocus

    HoverHandler { cursorShape: Qt.PointingHandCursor }

    background: Rectangle {
        radius: 6
        color: card.down ? Qt.alpha(card.accentColor, 0.18)
                         : (card.hovered ? card.accentWash : card.surfaceColor)
        border.width: card.activeFocus ? 2 : 1
        border.color: card.activeFocus ? card.accentColor
                                       : (card.hovered ? Qt.alpha(card.accentColor, 0.55) : card.borderColor)
        scale: card.down ? 0.985 : 1

        Behavior on color { ColorAnimation { duration: 110 } }
        Behavior on border.color { ColorAnimation { duration: 110 } }
        Behavior on scale { NumberAnimation { duration: 80 } }
    }

    contentItem: RowLayout {
        spacing: 10

        Image {
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: 28
            Layout.preferredHeight: 28
            source: card.iconSource
            sourceSize: Qt.size(56, 56)
            fillMode: Image.PreserveAspectFit
            asynchronous: true
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 1

            Text {
                id: titleText
                Layout.fillWidth: true
                text: card.title
                color: card.primaryTextColor
                font.pixelSize: Application.font.pixelSize > 0 ? Application.font.pixelSize + 1 : 14
                font.weight: Font.DemiBold
                elide: Text.ElideRight
            }

            Text {
                id: descriptionText
                Layout.fillWidth: true
                text: card.description.length > 0 ? card.description : qsTr("Open this tool")
                color: card.mutedTextColor
                font.pixelSize: Application.font.pixelSize > 0 ? Application.font.pixelSize - 1 : 12
                wrapMode: Text.Wrap
                maximumLineCount: 2
                elide: Text.ElideRight
            }
        }
    }

    leftPadding: 14
    rightPadding: 14
    topPadding: 8
    bottomPadding: 8

    ToolTip.visible: hovered && descriptionText.truncated
    ToolTip.text: descriptionText.text
    ToolTip.delay: 600

    Accessible.name: title
    Accessible.description: description
    Accessible.role: Accessible.Button
}

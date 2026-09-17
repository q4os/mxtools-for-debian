import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtCore
import "components"

ApplicationWindow {
    id: root

    required property var backend

    visible: false
    width: Math.min(windowSettings.windowWidth, 760)
    height: windowSettings.windowHeight
    minimumWidth: 680
    minimumHeight: 560
    title: qsTr("MX Welcome")
    color: backgroundColor

    readonly property real baseFontSize: Application.font.pixelSize > 0 ? Application.font.pixelSize : 13

    SystemPalette {
        id: systemPalette
        colorGroup: root.active ? SystemPalette.Active : SystemPalette.Inactive
    }

    readonly property color backgroundColor: systemPalette.window
    readonly property color surfaceColor: systemPalette.base
    readonly property color primaryTextColor: systemPalette.text
    readonly property color accentColor: systemPalette.highlight
    readonly property color mutedTextColor: Qt.alpha(systemPalette.text, 0.82)
    readonly property color borderColor: Qt.alpha(systemPalette.text, 0.18)
    readonly property color accentWash: Qt.alpha(systemPalette.highlight, 0.13)

    property int currentPage: root.backend.startOnAbout ? 1 : 0

    Settings {
        id: windowSettings
        category: "Window"
        property int windowWidth: 760
        property int windowHeight: 720
    }

    onClosing: function(close) {
        windowSettings.windowWidth = width
        windowSettings.windowHeight = height
    }

    function showDocument(title, text, richText) {
        documentDialog.title = title
        documentText.textFormat = richText ? TextEdit.RichText : TextEdit.PlainText
        documentText.text = text
        documentDialog.open()
    }

    Connections {
        target: root.backend
        function onErrorOccurred(title, message) {
            errorDialog.title = title
            errorDialog.text = message
            errorDialog.open()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: root.backend.headerSource.toString().length > 0 ? 112 : 88
            color: root.surfaceColor

            Image {
                anchors.fill: parent
                source: root.backend.headerSource
                fillMode: Image.PreserveAspectCrop
                opacity: 0.22
                visible: source.toString().length > 0
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 24
                anchors.rightMargin: 20
                spacing: 14

                Image {
                    Layout.preferredWidth: 56
                    Layout.preferredHeight: 56
                    source: root.backend.logoSource.toString().length > 0
                            ? root.backend.logoSource : "image://icons/mx-welcome"
                    sourceSize: Qt.size(72, 72)
                    fillMode: Image.PreserveAspectFit
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 3

                    Text {
                        Layout.fillWidth: true
                        text: qsTr("MX Welcome")
                        color: root.primaryTextColor
                        font.pixelSize: root.baseFontSize + 8
                        font.weight: Font.DemiBold
                        elide: Text.ElideRight
                    }
                    Text {
                        Layout.fillWidth: true
                        text: root.backend.distroTitle
                        color: root.mutedTextColor
                        font.pixelSize: root.baseFontSize
                        elide: Text.ElideRight
                    }
                }

                SecondaryButton {
                    text: root.currentPage === 0 ? qsTr("About this system") : qsTr("Welcome")
                    textColor: root.primaryTextColor
                    surfaceColor: root.surfaceColor
                    hoverColor: root.accentWash
                    borderColor: root.borderColor
                    accentColor: root.accentColor
                    onClicked: {
                        if (root.currentPage === 0) {
                            root.currentPage = 1
                            root.backend.requestSystemInfo()
                        } else {
                            root.currentPage = 0
                        }
                    }
                }

                SecondaryButton {
                    text: qsTr("Manual")
                    textColor: root.primaryTextColor
                    surfaceColor: root.surfaceColor
                    hoverColor: root.accentWash
                    borderColor: root.borderColor
                    accentColor: root.accentColor
                    onClicked: root.backend.activate("manual")
                }

                SecondaryButton {
                    text: qsTr("About")
                    textColor: root.primaryTextColor
                    surfaceColor: root.surfaceColor
                    hoverColor: root.accentWash
                    borderColor: root.borderColor
                    accentColor: root.accentColor
                    onClicked: aboutDialog.open()
                }
            }

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 1
                color: root.borderColor
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: liveInfoText.implicitHeight + 20
            visible: root.backend.showLiveUserInfo
            color: root.accentWash

            Text {
                id: liveInfoText
                anchors.fill: parent
                anchors.margins: 10
                text: root.backend.liveUserInfo
                color: root.primaryTextColor
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                Accessible.name: text
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            StackLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: root.currentPage

                Item {
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 22
                        spacing: 14

                        GridView {
                            id: actionGrid
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            clip: true
                            model: root.backend.actions
                            property int columnCount: Math.max(1, Math.floor(width / 270))
                            cellWidth: width / columnCount
                            cellHeight: 105
                            boundsBehavior: Flickable.StopAtBounds
                            keyNavigationEnabled: true

                            // Bypass Flickable's wheel momentum so a touchpad can reverse
                            // direction immediately instead of waiting for deceleration.
                            WheelHandler {
                                target: null
                                acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
                                onWheel: function(event) {
                                    actionGrid.contentY = Math.max(0, Math.min(
                                        Math.max(0, actionGrid.contentHeight - actionGrid.height),
                                        actionGrid.contentY - event.angleDelta.y))
                                }
                            }

                            delegate: Item {
                                required property string identifier
                                required property string title
                                required property string description
                                required property url iconSource
                                required property bool actionEnabled
                                width: actionGrid.cellWidth
                                height: actionGrid.cellHeight

                                ToolCard {
                                    anchors.fill: parent
                                    anchors.margins: 6
                                    actionId: parent.identifier
                                    title: parent.title
                                    description: parent.description
                                    iconSource: parent.iconSource
                                    enabled: parent.actionEnabled
                                    surfaceColor: root.surfaceColor
                                    primaryTextColor: root.primaryTextColor
                                    mutedTextColor: root.mutedTextColor
                                    accentColor: root.accentColor
                                    accentWash: root.accentWash
                                    borderColor: root.borderColor
                                    onClicked: root.backend.activate(actionId)
                                }
                            }

                            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
                        }

                        Text {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            visible: actionGrid.count === 0
                            text: qsTr("No actions are available.")
                            color: root.mutedTextColor
                            font.pixelSize: root.baseFontSize + 2
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        CheckBox {
                            Layout.alignment: Qt.AlignHCenter
                            visible: !root.backend.liveSession
                            text: qsTr("Show this dialog at start up")
                            palette.windowText: root.primaryTextColor
                            checked: root.backend.autoStartup
                            onToggled: root.backend.autoStartup = checked

                            HoverHandler { cursorShape: Qt.PointingHandCursor }
                        }
                    }
                }

                ScrollView {
                    id: systemPage
                    clip: true

                    ColumnLayout {
                        width: Math.max(systemPage.availableWidth, 0)
                        spacing: 18

                        Item { Layout.preferredHeight: 4 }

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.leftMargin: 24
                            Layout.rightMargin: 24

                            Text {
                                Layout.fillWidth: true
                                text: qsTr("About this system")
                                color: root.primaryTextColor
                                font.pixelSize: root.baseFontSize + 7
                                font.weight: Font.DemiBold
                            }
                        }

                        GridLayout {
                            Layout.fillWidth: true
                            Layout.leftMargin: 24
                            Layout.rightMargin: 24
                            columns: root.width >= 760 ? 3 : 1
                            columnSpacing: 12
                            rowSpacing: 12

                            Repeater {
                                model: [
                                    { label: qsTr("MX version"), value: root.backend.distroVersion },
                                    { label: qsTr("Debian version"), value: root.backend.debianVersion },
                                    { label: qsTr("Desktop"), value: root.backend.desktopVersion },
                                    { label: qsTr("Supported until"), value: root.backend.supportedUntil }
                                ]
                                Rectangle {
                                    id: systemDetailCard
                                    required property var modelData
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 82
                                    radius: 6
                                    color: root.surfaceColor
                                    border.width: 1
                                    border.color: root.borderColor

                                    Column {
                                        anchors.fill: parent
                                        anchors.margins: 14
                                        spacing: 5
                                        Text {
                                            width: parent.width
                                            text: systemDetailCard.modelData.label
                                            color: root.mutedTextColor
                                            font.pixelSize: Math.max(10, root.baseFontSize - 1)
                                            elide: Text.ElideRight
                                        }
                                        Text {
                                            width: parent.width
                                            text: systemDetailCard.modelData.value.length > 0
                                                  ? systemDetailCard.modelData.value : qsTr("Unavailable")
                                            color: root.primaryTextColor
                                            font.weight: Font.DemiBold
                                            elide: Text.ElideRight
                                        }
                                    }
                                }
                            }
                        }

                        Text {
                            Layout.fillWidth: true
                            Layout.leftMargin: 24
                            Layout.rightMargin: 24
                            text: qsTr("Short system report")
                            color: root.primaryTextColor
                            font.pixelSize: root.baseFontSize + 2
                            font.weight: Font.DemiBold
                        }

                        TextArea {
                            Layout.fillWidth: true
                            Layout.leftMargin: 24
                            Layout.rightMargin: 24
                            Layout.preferredHeight: Math.max(81, contentHeight + topPadding + bottomPadding)
                            text: root.backend.systemInfo.length > 0 ? root.backend.systemInfo : qsTr("Loading system information…")
                            color: root.primaryTextColor
                            readOnly: true
                            selectByMouse: true
                            wrapMode: TextEdit.Wrap
                            Accessible.name: qsTr("Short system report")
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.leftMargin: 24
                            Layout.rightMargin: 24
                            spacing: 10

                            SecondaryButton {
                                text: qsTr("Quick-System-Info Full Report")
                                textColor: root.primaryTextColor
                                surfaceColor: root.surfaceColor
                                hoverColor: root.accentWash
                                borderColor: root.borderColor
                                accentColor: root.accentColor
                                onClicked: root.backend.openFullSystemReport()
                            }
                            SecondaryButton {
                                text: qsTr("Terms of Use")
                                textColor: root.primaryTextColor
                                surfaceColor: root.surfaceColor
                                hoverColor: root.accentWash
                                borderColor: root.borderColor
                                accentColor: root.accentColor
                                onClicked: {
                                    const text = root.backend.termsText()
                                    if (text.length > 0)
                                        root.showDocument(qsTr("Terms of Use"), text, false)
                                }
                            }
                            Item { Layout.fillWidth: true }
                        }

                        Text {
                            Layout.fillWidth: true
                            Layout.leftMargin: 24
                            Layout.rightMargin: 24
                            Layout.bottomMargin: 24
                            text: root.backend.termsSummary
                            color: root.mutedTextColor
                            wrapMode: Text.Wrap
                        }
                    }
                }
            }
        }
    }

    Dialog {
        id: aboutDialog
        anchors.centerIn: Overlay.overlay
        width: Math.min(520, root.width - 40)
        modal: true
        title: qsTr("About MX Welcome")
        background: Rectangle {
            color: root.backgroundColor
            border.color: root.borderColor
        }
        footer: DialogButtonBox {
            standardButtons: DialogButtonBox.Close
            alignment: Qt.AlignRight
            padding: 12
            background: Item {}
            delegate: SecondaryButton {
                textColor: root.primaryTextColor
                surfaceColor: root.surfaceColor
                hoverColor: root.accentWash
                borderColor: root.borderColor
                accentColor: root.accentColor
            }
            onRejected: aboutDialog.reject()
        }

        ColumnLayout {
            width: parent.width
            spacing: 12
            Image {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 72
                Layout.preferredHeight: 72
                source: root.backend.logoSource.toString().length > 0 ? root.backend.logoSource : "image://icons/mx-welcome"
                sourceSize: Qt.size(96, 96)
                fillMode: Image.PreserveAspectFit
            }
            Text {
                Layout.fillWidth: true
                text: qsTr("MX Welcome")
                color: root.primaryTextColor
                font.pixelSize: root.baseFontSize + 7
                font.weight: Font.DemiBold
                horizontalAlignment: Text.AlignHCenter
            }
            Text {
                Layout.fillWidth: true
                text: qsTr("Version: %1").arg(root.backend.version)
                color: root.mutedTextColor
                horizontalAlignment: Text.AlignHCenter
            }
            Text {
                Layout.fillWidth: true
                text: qsTr("Program for displaying a welcome screen in MX Linux")
                color: root.primaryTextColor
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
            }
            Text {
                Layout.fillWidth: true
                text: qsTr("Copyright (c) MX Linux")
                color: root.mutedTextColor
                horizontalAlignment: Text.AlignHCenter
            }
            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                SecondaryButton {
                    text: qsTr("License")
                    textColor: root.primaryTextColor
                    surfaceColor: root.surfaceColor
                    hoverColor: root.accentWash
                    borderColor: root.borderColor
                    accentColor: root.accentColor
                    onClicked: root.showDocument(qsTr("MX Welcome License"), root.backend.licenseHtml(), true)
                }
                SecondaryButton {
                    text: qsTr("Changelog")
                    textColor: root.primaryTextColor
                    surfaceColor: root.surfaceColor
                    hoverColor: root.accentWash
                    borderColor: root.borderColor
                    accentColor: root.accentColor
                    onClicked: root.showDocument(qsTr("Changelog"), root.backend.changelogText(), false)
                }
            }
        }
    }

    Dialog {
        id: documentDialog
        anchors.centerIn: Overlay.overlay
        width: Math.min(760, root.width - 40)
        height: Math.min(620, root.height - 40)
        modal: true
        background: Rectangle {
            color: root.backgroundColor
            border.color: root.borderColor
        }
        footer: DialogButtonBox {
            standardButtons: DialogButtonBox.Close
            alignment: Qt.AlignRight
            padding: 12
            background: Item {}
            delegate: SecondaryButton {
                textColor: root.primaryTextColor
                surfaceColor: root.surfaceColor
                hoverColor: root.accentWash
                borderColor: root.borderColor
                accentColor: root.accentColor
            }
            onRejected: documentDialog.reject()
        }

        ScrollView {
            anchors.fill: parent
            TextArea {
                id: documentText
                color: root.primaryTextColor
                readOnly: true
                selectByMouse: true
                wrapMode: TextEdit.Wrap
                onLinkActivated: function(link) { Qt.openUrlExternally(link) }
            }
        }
    }

    Dialog {
        id: errorDialog
        property string text
        anchors.centerIn: Overlay.overlay
        width: Math.min(440, root.width - 40)
        modal: true
        background: Rectangle {
            color: root.backgroundColor
            border.color: root.borderColor
        }
        footer: DialogButtonBox {
            standardButtons: DialogButtonBox.Ok
            alignment: Qt.AlignRight
            padding: 12
            background: Item {}
            delegate: SecondaryButton {
                textColor: root.primaryTextColor
                surfaceColor: root.surfaceColor
                hoverColor: root.accentWash
                borderColor: root.borderColor
                accentColor: root.accentColor
            }
            onAccepted: errorDialog.accept()
        }

        Text {
            width: parent.width
            text: errorDialog.text
            color: root.primaryTextColor
            wrapMode: Text.Wrap
        }
    }

    Component.onCompleted: {
        root.x = Screen.virtualX + Math.round((Screen.desktopAvailableWidth - root.width) / 2)
        root.y = Screen.virtualY + Math.round((Screen.desktopAvailableHeight - root.height) / 2)
        root.visible = true
        if (currentPage === 1)
            root.backend.requestSystemInfo()
    }
}

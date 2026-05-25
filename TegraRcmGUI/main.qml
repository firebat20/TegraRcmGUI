import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtCore

ApplicationWindow {
    id: root
    width: 920
    height: 660
    visible: true
    title: qsTr("TegraRcmGUI")
    color: appController.theme === "Light" ? "#f3f4f6" : "#121212"

    palette.window: color
    palette.windowText: appController.theme === "Light" ? "#1f2937" : "#f8fafc"
    palette.button: appController.theme === "Light" ? "#ffffff" : "#1f2937"
    palette.buttonText: appController.theme === "Light" ? "#1f2937" : "#f8fafc"
    palette.base: appController.theme === "Light" ? "#ffffff" : "#1b1b1b"
    palette.alternateBase: appController.theme === "Light" ? "#f8fafc" : "#242424"
    palette.text: appController.theme === "Light" ? "#111827" : "#f8fafc"
    palette.highlight: "#2a82da"
    palette.highlightedText: "#ffffff"

    header: ToolBar {
        contentHeight: 56
        RowLayout {
            anchors.fill: parent
            spacing: 16
            padding: 16
            Label {
                text: qsTr("TegraRcmGUI")
                font.pixelSize: 22
                font.bold: true
                color: root.palette.windowText
            }
            Item { Layout.fillWidth: true }
            Label {
                text: appController.deviceConnected ? qsTr("Device connected") : qsTr("No device")
                color: appController.deviceConnected ? "#4ade80" : "#f97316"
            }
        }
    }

    FileDialog {
        id: payloadDialog
        title: qsTr("Select Payload")
        currentFolder: StandardPaths.writableLocation(StandardPaths.HomeLocation)
        nameFilters: [qsTr("Bin Files (*.bin)"), qsTr("All Files (*)")]
        onAccepted: {
            appController.setPayloadPath(selectedFile)
        }
    }

    ColumnLayout {
        anchors {
            top: parent.top
            topMargin: header.height
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            bottomMargin: 0
        }
        spacing: 0

        TabBar {
            id: tabBar
            Layout.fillWidth: true
            currentIndex: stackLayout.currentIndex

            TabButton {
                text: qsTr("Inject")
                contentItem: Text {
                    text: parent.text
                    font: parent.font
                    opacity: enabled ? 1.0 : 0.3
                    color: parent.checked ? root.palette.highlight : root.palette.windowText
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
                background: Rectangle {
                    implicitWidth: 100
                    implicitHeight: 40
                    opacity: enabled ? 1 : 0.3
                    color: parent.checked ? (appController.theme === "Light" ? "#e5e7eb" : "#2d3748") : "transparent"
                    border.color: parent.checked ? root.palette.highlight : "transparent"
                    border.width: 1
                    radius: 8
                }
            }

            TabButton {
                text: qsTr("Settings")
                contentItem: Text {
                    text: parent.text
                    font: parent.font
                    opacity: enabled ? 1.0 : 0.3
                    color: parent.checked ? root.palette.highlight : root.palette.windowText
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
                background: Rectangle {
                    implicitWidth: 100
                    implicitHeight: 40
                    opacity: enabled ? 1 : 0.3
                    color: parent.checked ? (appController.theme === "Light" ? "#e5e7eb" : "#2d3748") : "transparent"
                    border.color: parent.checked ? root.palette.highlight : "transparent"
                    border.width: 1
                    radius: 8
                }
            }

            TabButton {
                text: qsTr("Logs")
                contentItem: Text {
                    text: parent.text
                    font: parent.font
                    opacity: enabled ? 1.0 : 0.3
                    color: parent.checked ? root.palette.highlight : root.palette.windowText
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
                background: Rectangle {
                    implicitWidth: 100
                    implicitHeight: 40
                    opacity: enabled ? 1 : 0.3
                    color: parent.checked ? (appController.theme === "Light" ? "#e5e7eb" : "#2d3748") : "transparent"
                    border.color: parent.checked ? root.palette.highlight : "transparent"
                    border.width: 1
                    radius: 8
                }
            }
        }

        StackLayout {
            id: stackLayout
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabBar.currentIndex

            // Page 1: Inject
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "transparent"
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 18

                    RowLayout {
                        spacing: 12
                        TextField {
                            id: payloadPathField
                            Layout.fillWidth: true
                            placeholderText: qsTr("No payload selected")
                            text: appController.payloadPath
                            onTextChanged: appController.setPayloadPath(text)
                        }
                        Button {
                            text: qsTr("Browse")
                            onClicked: payloadDialog.open()
                        }
                    }

                    Button {
                        text: qsTr("Inject Payload")
                        enabled: appController.payloadPath.length > 0
                        font.pixelSize: 16
                        onClicked: appController.injectPayload()
                        Layout.maximumWidth: 240
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 100
                        radius: 12
                        color: appController.theme === "Light" ? "#f8fafc" : "#1f2937"
                        border.color: appController.theme === "Light" ? "#d1d5db" : "#334155"
                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 16
                            Label {
                                text: appController.deviceConnected ? qsTr("RCM device detected") : qsTr("No RCM device detected")
                                font.pixelSize: 16
                                color: root.palette.windowText
                            }
                        }
                    }

                    GroupBox {
                        title: qsTr("Recent favorites")
                        Layout.fillWidth: true
                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 8
                            ListView {
                                id: favoritesList
                                Layout.fillWidth: true
                                Layout.preferredHeight: 200
                                model: appController.favorites
                                clip: true
                                delegate: Rectangle {
                                    width: parent.width
                                    height: 48
                                    color: index % 2 === 0 ? (appController.theme === "Light" ? "#ffffff" : "#1b1b1b") : (appController.theme === "Light" ? "#f8fafc" : "#171717")
                                    border.color: appController.theme === "Light" ? "#e5e7eb" : "#334155"
                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.margins: 8
                                        spacing: 12
                                        Text {
                                            text: modelData
                                            elide: Text.ElideRight
                                            color: root.palette.windowText
                                            Layout.fillWidth: true
                                        }
                                        Button {
                                            text: qsTr("Select")
                                            onClicked: appController.setPayloadPath(modelData)
                                        }
                                    }
                                }
                            }
                            Label {
                                text: appController.favorites.length === 0 ? qsTr("No favorites yet.") : ""
                                color: root.palette.windowText
                            }
                        }
                    }
                }
            }

            // Page 2: Settings
            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                contentWidth: parent.width
                ColumnLayout {
                    width: parent.width
                    anchors.margins: 20
                    spacing: 16

                    RowLayout {
                        spacing: 12
                        Label { text: qsTr("Theme") ; color: root.palette.windowText }
                        ComboBox {
                            model: [qsTr("Dark"), qsTr("Light")]
                            currentIndex: appController.theme === "Light" ? 1 : 0
                            onCurrentIndexChanged: appController.setTheme(currentText)
                        }
                    }

                    RowLayout {
                        spacing: 12
                        Label { text: qsTr("Auto inject payload") ; color: root.palette.windowText }
                        Switch { checked: appController.autoInject; onCheckedChanged: appController.setAutoInject(checked) }
                    }

                    RowLayout {
                        spacing: 12
                        Label { text: qsTr("Minimize to tray") ; color: root.palette.windowText }
                        Switch { checked: appController.minimizeToTray; onCheckedChanged: appController.setMinimizeToTray(checked) }
                    }

                    GroupBox {
                        title: qsTr("Favorites management")
                        Layout.fillWidth: true
                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 8
                            TextField {
                                id: favoritePathField
                                Layout.fillWidth: true
                                placeholderText: qsTr("Enter payload path or select from inject tab")
                                text: appController.payloadPath
                                readOnly: true
                            }
                            RowLayout {
                                spacing: 8
                                Button {
                                    text: qsTr("Add current payload")
                                    enabled: appController.payloadPath.length > 0
                                    onClicked: appController.addFavorite(appController.payloadPath)
                                }
                            }
                            ListView {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 220
                                model: appController.favorites
                                clip: true
                                delegate: Rectangle {
                                    width: parent.width
                                    height: 44
                                    color: index % 2 === 0 ? (appController.theme === "Light" ? "#ffffff" : "#1b1b1b") : (appController.theme === "Light" ? "#f8fafc" : "#171717")
                                    border.color: appController.theme === "Light" ? "#e5e7eb" : "#334155"
                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.margins: 8
                                        spacing: 12
                                        Text {
                                            text: modelData
                                            elide: Text.ElideRight
                                            color: root.palette.windowText
                                            Layout.fillWidth: true
                                        }
                                        Button {
                                            text: qsTr("Remove")
                                            onClicked: appController.removeFavorite(modelData)
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Page 3: Logs
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                anchors.margins: 20
                spacing: 12

                TextArea {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    readOnly: true
                    wrapMode: TextArea.WrapAnywhere
                    text: appController.logMessages.join("\n")
                    font.family: "Segoe UI"
                }

                Button {
                    text: qsTr("Clear Logs")
                    onClicked: appController.clearLogs()
                    enabled: appController.logMessages.length > 0
                    Layout.alignment: Qt.AlignRight
                }
            }
        }
    }

    Component.onCompleted: {
        if (appController.theme === "Light") {
            root.color = "#f3f4f6"
        }
    }

    onVisibilityChanged: {
        if (appController.minimizeToTray && visibility === WindowMinimized) {
            visible = false
        }
    }
}

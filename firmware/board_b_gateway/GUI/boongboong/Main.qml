import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: root
    width: 1280
    height: 820
    minimumWidth: 1060
    minimumHeight: 700
    visible: true
    title: qsTr("Board B Gateway Monitor")
    color: "#101418"

    readonly property color panelColor: "#181f26"
    readonly property color panelSoft: "#202831"
    readonly property color panelDark: "#0d1115"
    readonly property color panelBorder: "#2a343f"
    readonly property color textPrimary: "#e8edf2"
    readonly property color textMuted: "#8a96a3"
    readonly property color goodColor: "#28c76f"
    readonly property color warnColor: "#ffb020"
    readonly property color badColor: "#ea5455"
    readonly property color accentColor: "#3ba7ff"

    QtObject {
        id: gateway
        property bool connected: serialBridge.connected
        property int can1Rx: serialBridge.can1Rx
        property int can1Tx: serialBridge.can1Tx
        property int can2Rx: serialBridge.can2Rx
        property int can2Tx: serialBridge.can2Tx
        property int busy: serialBridge.busy
        property int errors: serialBridge.errors
        property bool warning: serialBridge.warning
        property int rpm: serialBridge.rpm
        property int speed: serialBridge.speed
        property int coolant: serialBridge.coolant
        property bool ignition: serialBridge.ignition
        property bool doorFl: serialBridge.doorFl
        property bool doorFr: serialBridge.doorFr
        property bool doorRl: serialBridge.doorRl
        property bool doorRr: serialBridge.doorRr
        property bool turnLeft: serialBridge.turnLeft
        property bool turnRight: serialBridge.turnRight
        property bool highBeam: serialBridge.highBeam
        property bool fogLamp: serialBridge.fogLamp
    }

    ListModel { id: logModel }

    Component.onCompleted: serialBridge.refreshPorts()

    Connections {
        target: serialBridge

        function appendLine(line) {
            if (logModel.count > 300)
                logModel.remove(0)
            logModel.append({ text: line })
            logList.positionViewAtEnd()
        }

        function onGatewayLineReceived(time, line) {
            appendLine(time + " | UART | GW | " + line)
        }

        function onFrameLineReceived(time, bus, dir, frameId, dlc, payload, decoded) {
            appendLine(time + " | " + bus + " | " + dir + " | " + frameId + " | " + dlc + " | " + payload + " | " + decoded)
        }

        function onRawLineReceived(time, line) {
            appendLine(time + " | UART | -- | " + line)
        }
    }

    component MetricCard: Rectangle {
        property string title: ""
        property string value: ""
        property string detail: ""
        property color accent: root.accentColor

        color: root.panelColor
        border.color: root.panelBorder
        border.width: 1
        radius: 8

        Text {
            id: metricTitle
            x: 18
            y: 18
            width: parent.width - 54
            text: title
            color: root.textMuted
            font.pixelSize: 15
            font.weight: Font.DemiBold
            elide: Text.ElideRight
        }

        Rectangle {
            x: parent.width - 28
            y: 24
            width: 10
            height: 10
            radius: 5
            color: accent
        }

        Text {
            x: 18
            y: 48
            width: parent.width - 36
            height: 42
            text: value
            color: root.textPrimary
            font.pixelSize: 32
            font.weight: Font.Bold
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }

        Text {
            x: 18
            y: 98
            width: parent.width - 36
            text: detail
            color: root.textMuted
            font.pixelSize: 14
            elide: Text.ElideRight
        }
    }

    component SmallTile: Rectangle {
        property string label: ""
        property string value: ""

        color: root.panelSoft
        radius: 7

        Text {
            x: 8
            y: 12
            width: parent.width - 16
            text: label
            color: root.textMuted
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 12
            elide: Text.ElideRight
        }

        Text {
            x: 8
            y: 34
            width: parent.width - 16
            text: value
            color: root.textPrimary
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 17
            font.weight: Font.Bold
            elide: Text.ElideRight
        }
    }

    component StatusPill: Rectangle {
        property string label: ""
        property bool active: false

        color: active ? "#203b2d" : root.panelSoft
        border.color: active ? root.goodColor : root.panelBorder
        border.width: 1
        radius: 6

        Rectangle {
            x: 10
            y: parent.height / 2 - 4
            width: 8
            height: 8
            radius: 4
            color: active ? root.goodColor : root.textMuted
        }

        Text {
            x: 26
            y: 0
            width: parent.width - 34
            height: parent.height
            text: label
            color: root.textPrimary
            font.pixelSize: 13
            font.weight: Font.DemiBold
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }

    header: Rectangle {
        height: 104
        color: root.panelDark
        border.color: root.panelBorder
        border.width: 1
        clip: true

        Text {
            id: headerTitle
            x: 22
            y: 0
            width: parent.width - 44
            height: 42
            text: "Board B Gateway Monitor"
            color: root.textPrimary
            font.pixelSize: 21
            font.weight: Font.Bold
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        ComboBox {
            id: portCombo
            x: 22
            y: 52
            width: Math.max(230, Math.min(320, parent.width * 0.26))
            height: 36
            model: serialBridge.portNames
        }

        ComboBox {
            id: baudCombo
            x: portCombo.x + portCombo.width + 16
            y: 52
            width: 116
            height: 36
            model: ["115200", "921600"]
        }

        Button {
            x: baudCombo.x + baudCombo.width + 16
            y: 52
            width: 118
            height: 36
            text: gateway.connected ? "Disconnect" : "Connect"
            onClicked: {
                if (gateway.connected)
                    serialBridge.disconnectPort()
                else
                    serialBridge.connectToPort(portCombo.currentText, parseInt(baudCombo.currentText))
            }
        }

        Button {
            x: baudCombo.x + baudCombo.width + 150
            y: 52
            width: 90
            height: 36
            text: "Refresh"
            onClicked: {
                serialBridge.resetMonitor()
                logModel.clear()
                canIdField.text = ""
            }
        }

        Rectangle {
            id: statusBadge
            x: parent.width - statusText.width - width - 34
            y: 54
            width: 112
            height: 32
            radius: 16
            color: gateway.connected ? "#203b2d" : "#2a2020"
            border.color: gateway.connected ? root.goodColor : root.badColor
            border.width: 1

            Text {
                anchors.centerIn: parent
                text: gateway.connected ? "CONNECTED" : "OFFLINE"
                color: root.textPrimary
                font.pixelSize: 12
                font.weight: Font.Bold
            }
        }

        Text {
            id: statusText
            x: parent.width - width - 18
            y: 52
            width: 190
            height: 36
            text: serialBridge.statusText
            color: root.textMuted
            font.pixelSize: 12
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }

    Flickable {
        id: flick
        anchors.fill: parent
        anchors.margins: 0
        contentWidth: Math.max(width, 1040)
        contentHeight: page.height
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
        }

        Item {
            id: page
            width: flick.contentWidth
            height: 1010

            readonly property int margin: 18
            readonly property int gap: 14
            readonly property int innerWidth: width - margin * 2
            readonly property int cardW: (innerWidth - gap * 2) / 3
            readonly property int halfW: (innerWidth - gap) / 2

            MetricCard {
                x: page.margin
                y: 18
                width: page.cardW
                height: 132
                title: "CAN1 Bus"
                value: "RX " + gateway.can1Rx
                detail: "TX " + gateway.can1Tx + " / Powertrain + Body"
                accent: root.goodColor
            }

            MetricCard {
                x: page.margin + page.cardW + page.gap
                y: 18
                width: page.cardW
                height: 132
                title: "CAN2 Bus"
                value: "TX " + gateway.can2Tx
                detail: "RX " + gateway.can2Rx + " / Cluster + Diagnostic"
                accent: root.accentColor
            }

            MetricCard {
                x: page.margin + (page.cardW + page.gap) * 2
                y: 18
                width: page.cardW
                height: 132
                title: "Gateway Health"
                value: gateway.warning ? "WARNING" : "NORMAL"
                detail: "busy " + gateway.busy + " / err " + gateway.errors
                accent: gateway.warning ? root.warnColor : root.goodColor
            }

            Rectangle {
                id: enginePanel
                x: page.margin
                y: 164
                width: page.halfW
                height: 360
                color: root.panelColor
                border.color: root.panelBorder
                border.width: 1
                radius: 8

                Text {
                    x: 18
                    y: 18
                    width: parent.width - 36
                    text: "Board A Engine"
                    color: root.textPrimary
                    font.pixelSize: 20
                    font.weight: Font.Bold
                    elide: Text.ElideRight
                }

                Text {
                    x: 18
                    y: 64
                    width: parent.width - 36
                    height: 48
                    text: gateway.rpm + " rpm"
                    color: root.textPrimary
                    font.pixelSize: 38
                    font.weight: Font.Bold
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                }

                ProgressBar {
                    x: 18
                    y: 130
                    width: parent.width - 36
                    height: 14
                    value: Math.max(0, Math.min(1, gateway.rpm / 6000))
                }

                SmallTile {
                    x: 18
                    y: 170
                    width: (parent.width - 50) / 2
                    height: 68
                    label: "Speed"
                    value: gateway.speed + " km/h"
                }

                SmallTile {
                    x: 32 + (parent.width - 50) / 2
                    y: 170
                    width: (parent.width - 50) / 2
                    height: 68
                    label: "Coolant"
                    value: gateway.coolant + " C"
                }

                SmallTile {
                    x: 18
                    y: 252
                    width: (parent.width - 50) / 2
                    height: 68
                    label: "IGN"
                    value: gateway.ignition ? "ON" : "OFF"
                }

                SmallTile {
                    x: 32 + (parent.width - 50) / 2
                    y: 252
                    width: (parent.width - 50) / 2
                    height: 68
                    label: "Last RX"
                    value: serialBridge.lastEngineRx
                }
            }

            Rectangle {
                id: bodyPanel
                x: page.margin + page.halfW + page.gap
                y: 164
                width: page.halfW
                height: 360
                color: root.panelColor
                border.color: root.panelBorder
                border.width: 1
                radius: 8

                Text {
                    x: 18
                    y: 18
                    width: parent.width - 36
                    text: "Board D Body"
                    color: root.textPrimary
                    font.pixelSize: 20
                    font.weight: Font.Bold
                    elide: Text.ElideRight
                }

                readonly property int pillW: (width - 60) / 4

                StatusPill { x: 18; y: 60; width: bodyPanel.pillW; height: 34; label: "FL Door"; active: gateway.doorFl }
                StatusPill { x: 28 + bodyPanel.pillW; y: 60; width: bodyPanel.pillW; height: 34; label: "FR Door"; active: gateway.doorFr }
                StatusPill { x: 38 + bodyPanel.pillW * 2; y: 60; width: bodyPanel.pillW; height: 34; label: "RL Door"; active: gateway.doorRl }
                StatusPill { x: 48 + bodyPanel.pillW * 3; y: 60; width: bodyPanel.pillW; height: 34; label: "RR Door"; active: gateway.doorRr }
                StatusPill { x: 18; y: 104; width: bodyPanel.pillW; height: 34; label: "Left"; active: gateway.turnLeft }
                StatusPill { x: 28 + bodyPanel.pillW; y: 104; width: bodyPanel.pillW; height: 34; label: "Right"; active: gateway.turnRight }
                StatusPill { x: 38 + bodyPanel.pillW * 2; y: 104; width: bodyPanel.pillW; height: 34; label: "High"; active: gateway.highBeam }
                StatusPill { x: 48 + bodyPanel.pillW * 3; y: 104; width: bodyPanel.pillW; height: 34; label: "Fog"; active: gateway.fogLamp }

                Rectangle {
                    x: 18
                    y: 160
                    width: parent.width - 36
                    height: 176
                    radius: 8
                    color: root.panelSoft

                    Text {
                        x: 14
                        y: 12
                        width: parent.width - 28
                        text: "Cluster / CAN2 Output"
                        color: root.textPrimary
                        font.pixelSize: 16
                        font.weight: Font.Bold
                        elide: Text.ElideRight
                    }

                    Repeater {
                        model: [
                            ["0x280", "Motor_1 RPM", "50 ms", serialBridge.clusterRpmActive],
                            ["0x1A0", "Bremse_1 Speed", "500 ms", serialBridge.clusterSpeedActive],
                            ["0x390", "Body Forward", "100 ms", serialBridge.clusterBodyActive],
                            ["0x480", "Warning", "event", gateway.warning]
                        ]

                        delegate: Item {
                            x: 14
                            y: 48 + index * 30
                            width: parent.width - 28
                            height: 24

                            Text {
                                x: 0
                                y: 0
                                width: 58
                                height: parent.height
                                text: modelData[0]
                                color: root.accentColor
                                font.pixelSize: 14
                                font.weight: Font.Bold
                                verticalAlignment: Text.AlignVCenter
                                elide: Text.ElideRight
                            }

                            Text {
                                x: 68
                                y: 0
                                width: parent.width - 164
                                height: parent.height
                                text: modelData[1]
                                color: root.textPrimary
                                font.pixelSize: 14
                                verticalAlignment: Text.AlignVCenter
                                elide: Text.ElideRight
                            }

                            Text {
                                x: parent.width - 86
                                y: 0
                                width: 64
                                height: parent.height
                                text: modelData[2]
                                color: root.textMuted
                                font.pixelSize: 13
                                verticalAlignment: Text.AlignVCenter
                                elide: Text.ElideRight
                            }

                            Rectangle {
                                x: parent.width - 10
                                y: 7
                                width: 10
                                height: 10
                                radius: 5
                                color: modelData[3] ? root.goodColor : root.textMuted
                            }
                        }
                    }
                }
            }

            Rectangle {
                id: logPanel
                x: page.margin
                y: 538
                width: page.innerWidth
                height: 430
                color: root.panelColor
                border.color: root.panelBorder
                border.width: 1
                radius: 8

                Text {
                    x: 18
                    y: 18
                    width: 96
                    height: 36
                    text: "CAN Log"
                    color: root.textPrimary
                    font.pixelSize: 18
                    font.weight: Font.Bold
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }

                TextField {
                    id: canIdField
                    x: 124
                    y: 18
                    width: 160
                    height: 36
                    placeholderText: "CAN ID, e.g. 100"
                    color: root.textPrimary
                    placeholderTextColor: root.textMuted
                    inputMethodHints: Qt.ImhUppercaseOnly
                    onAccepted: applyCanFilterButton.clicked()
                }

                Button {
                    id: applyCanFilterButton
                    x: 304
                    y: 18
                    width: 92
                    height: 36
                    text: "Apply ID"
                    onClicked: {
                        var idText = canIdField.text.trim()
                        if (idText.length === 0)
                            return
                        if (idText.startsWith("0x") || idText.startsWith("0X"))
                            idText = idText.slice(2)
                        serialBridge.sendCommand("canlog id " + idText)
                        serialBridge.sendCommand("canlog on")
                    }
                }

                Button {
                    x: 416
                    y: 18
                    width: 64
                    height: 36
                    text: "All"
                    onClicked: {
                        serialBridge.sendCommand("canlog all")
                        serialBridge.sendCommand("canlog on")
                    }
                }

                Button {
                    x: 500
                    y: 18
                    width: 84
                    height: 36
                    text: "CAN Off"
                    onClicked: serialBridge.sendCommand("canlog off")
                }

                Button {
                    x: 604
                    y: 18
                    width: 76
                    height: 36
                    text: "GW Log"
                    onClicked: serialBridge.sendCommand("log on")
                }

                Button {
                    x: 700
                    y: 18
                    width: 76
                    height: 36
                    text: "GW Off"
                    onClicked: serialBridge.sendCommand("log off")
                }

                Button {
                    x: parent.width - 92
                    y: 18
                    width: 74
                    height: 36
                    text: "Clear"
                    onClicked: logModel.clear()
                }

                Rectangle {
                    id: latestPanel
                    x: 18
                    y: 70
                    width: parent.width - 36
                    height: 96
                    radius: 6
                    color: root.panelSoft
                    border.color: root.panelBorder

                    Text {
                        x: 14
                        y: 13
                        width: 160
                        text: "Latest Frame"
                        color: root.textMuted
                        font.pixelSize: 12
                        font.weight: Font.DemiBold
                        elide: Text.ElideRight
                    }

                    Text {
                        x: 14
                        y: 38
                        width: 160
                        text: serialBridge.latestFrameBus + " " + serialBridge.latestFrameDir + " " + serialBridge.latestFrameId
                        color: root.textPrimary
                        font.pixelSize: 18
                        font.weight: Font.Bold
                        elide: Text.ElideRight
                    }

                    Text {
                        x: 14
                        y: 68
                        width: 160
                        text: "DLC " + serialBridge.latestFrameDlc
                        color: root.textMuted
                        font.pixelSize: 12
                        elide: Text.ElideRight
                    }

                    Repeater {
                        model: serialBridge.latestFrameBytes

                        delegate: Rectangle {
                            x: 200 + index * 49
                            y: 27
                            width: 42
                            height: 42
                            radius: 6
                            color: root.panelDark
                            border.color: root.panelBorder

                            Text {
                                anchors.centerIn: parent
                                text: modelData
                                color: root.textPrimary
                                font.family: "Menlo"
                                font.pixelSize: 14
                                font.weight: Font.Bold
                            }
                        }
                    }

                    Rectangle {
                        x: 604
                        y: 14
                        width: 1
                        height: parent.height - 28
                        color: root.panelBorder
                    }

                    Text {
                        x: 630
                        y: 13
                        width: parent.width - 650
                        text: "Decoded"
                        color: root.textMuted
                        font.pixelSize: 12
                        font.weight: Font.DemiBold
                        elide: Text.ElideRight
                    }

                    Text {
                        x: 630
                        y: 38
                        width: parent.width - 650
                        text: serialBridge.latestFrameDecoded
                        color: root.textPrimary
                        font.pixelSize: 15
                        font.weight: Font.DemiBold
                        elide: Text.ElideRight
                    }

                    Text {
                        x: 630
                        y: 68
                        width: parent.width - 650
                        text: "raw: " + serialBridge.latestFrameRaw
                        color: root.textMuted
                        font.family: "Menlo"
                        font.pixelSize: 12
                        elide: Text.ElideRight
                    }
                }

                Rectangle {
                    x: 18
                    y: 184
                    width: parent.width - 36
                    height: parent.height - 202
                    radius: 6
                    color: root.panelDark
                    border.color: root.panelBorder

                    ListView {
                        id: logList
                        anchors.fill: parent
                        anchors.margins: 8
                        clip: true
                        model: logModel

                        delegate: Text {
                            width: ListView.view.width
                            height: 24
                            text: model.text
                            color: root.textPrimary
                            font.family: "Menlo"
                            font.pixelSize: 12
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                        }
                    }
                }
            }
        }
    }
}

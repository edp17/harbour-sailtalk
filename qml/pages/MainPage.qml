import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    Connections {
        target: appEngine

        onErrorOccurred: {
            console.log("SailTalk error: " + message)
        }
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: contentColumn.height + Theme.paddingLarge

        VerticalScrollDecorator {}

        Column {
            id: contentColumn
            width: parent.width - 2 * Theme.horizontalPageMargin
            x: Theme.horizontalPageMargin
            y: Theme.paddingLarge
            spacing: Theme.paddingLarge

            PageHeader {
                title: qsTr("SailTalk")
            }

            Label {
                width: parent.width
                wrapMode: Text.Wrap
                color: Theme.highlightColor
                text: qsTr("Prototype build")
            }

            Label {
                width: parent.width
                wrapMode: Text.Wrap
                text: qsTr("Enter the same signaling server on both devices. Then copy the other device's ID into the field below to place a call. ws://192.168.1.85:8080")
            }

            TextField {
                id: signalingField
                width: parent.width
                label: qsTr("Signaling server")
                placeholderText: qsTr("wss://your-server.example/ws")
                text: appEngine.signalingUrl
                onTextChanged: appEngine.signalingUrl = text
            }

            SectionHeader {
                text: qsTr("This device")
            }

            TextArea {
                width: parent.width
                readOnly: true
                label: qsTr("Your device ID")
                text: appEngine.ownId
            }

            SectionHeader {
                text: qsTr("Outgoing call")
            }

Label {
text: qsTr("For X10: b7399551-4238-4ac1-a522-1fb73e253a58 \ln for MotoG7: c496c7c9-703a-4c0c-8b6b-15d212d5513a")
}

            TextField {
                id: peerField
                width: parent.width
                label: qsTr("Call this device ID")
                placeholderText: qsTr("Paste the other device's ID")
                text: appEngine.peerId
                onTextChanged: appEngine.peerId = text
            }

            Label {
                width: parent.width
                text: qsTr("Status: %1").arg(appEngine.callState)
                color: Theme.primaryColor
            }

            Button {
                width: parent.width
                text: appEngine.connectedToSignaling
                      ? qsTr("Disconnect from server")
                      : qsTr("Connect to server")
                onClicked: {
                    if (appEngine.connectedToSignaling)
                        appEngine.disconnectSignaling()
                    else
                        appEngine.connectSignaling()
                }
            }

            Button {
                width: parent.width
                text: qsTr("Start audio call")
                enabled: appEngine.connectedToSignaling && peerField.text.length > 0
                onClicked: appEngine.startOutgoingCall()
            }

            Button {
                width: parent.width
                text: qsTr("Hang up")
                onClicked: appEngine.hangUp()
            }
        }
    }
}

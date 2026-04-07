import QtQuick 2.0
import QtQml 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    Connections {
        target: appEngine

        onErrorOccurred: {
            console.log("SailTalk error: " + message)
        }
    }

    Component.onCompleted: {
        if (!appEngine.connectedToSignaling)
            appEngine.connectSignaling()
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
                text: qsTr("Enter the same signaling server on both devices. Then copy the other device's ID into the field below to place a call.")
            }

            TextArea {
                width: parent.width
                readOnly: true
                label: qsTr("Signaling server")
                text: appEngine.signalingUrl
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
                enabled: appEngine.connectedToSignaling
                         && peerField.text.length > 0
                         && appEngine.callState === "idle"
                visible: appEngine.callState === "idle"
                onClicked: appEngine.startOutgoingCall()
            }

            Button {
                width: parent.width
                text: qsTr("Accept incoming call")
                visible: appEngine.callState === "incoming"
                onClicked: appEngine.acceptIncomingCall()
            }

            Button {
                width: parent.width
                text: qsTr("Reject incoming call")
                visible: appEngine.callState === "incoming"
                onClicked: appEngine.rejectIncomingCall()
            }

            Button {
                width: parent.width
                text: qsTr("Hang up")
                visible: appEngine.callState !== "idle" && appEngine.callState !== "incoming"
                onClicked: appEngine.hangUp()
            }
        }
    }
}

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
                color: appEngine.onlinePeers.indexOf(peerField.text) >= 0
                       ? Theme.highlightColor
                       : Theme.secondaryColor
                text: appEngine.onlinePeers.indexOf(peerField.text) >= 0
                      ? qsTr("Peer is online")
                      : qsTr("Peer is offline")
            }

            Label {
                width: parent.width
                text: qsTr("Status: %1").arg(appEngine.callState)
                color: Theme.primaryColor
            }

            Label {
                width: parent.width
                visible: appEngine.callState === "connected"
                text: qsTr("Call duration: %1").arg(appEngine.callDuration)
                color: Theme.highlightColor
            }

            Label {
                width: parent.width
                visible: appEngine.callState === "call-lost"
                color: Theme.errorColor
                text: qsTr("Call lost")
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
                         && (appEngine.callState === "idle"
                             || appEngine.callState === "call-lost"
                             || appEngine.callState === "rejected")
                visible: appEngine.callState === "idle"
                         || appEngine.callState === "call-lost"
                         || appEngine.callState === "rejected"
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

            TextSwitch {
                width: parent.width
                visible: appEngine.callState === "connected" || appEngine.callState === "connecting"
                text: qsTr("Use loudspeaker")
                checked: appEngine.speakerMode
                onCheckedChanged: appEngine.speakerMode = checked
            }

            TextSwitch {
                width: parent.width
                visible: appEngine.callState === "connected" || appEngine.callState === "connecting"
                text: qsTr("Mute microphone")
                onCheckedChanged: appEngine.setMute(checked)
            }

            Button {
                width: parent.width
                text: qsTr("Hang up")
                visible: appEngine.callState !== "idle"
                         && appEngine.callState !== "incoming"
                         && appEngine.callState !== "call-lost"
                         && appEngine.callState !== "rejected"
                onClicked: appEngine.hangUp()
            }
        }
    }
}

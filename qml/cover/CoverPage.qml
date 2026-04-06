import QtQuick 2.0
import Sailfish.Silica 1.0

CoverBackground {
    Column {
        anchors.centerIn: parent
        width: parent.width - Theme.paddingLarge * 2
        spacing: Theme.paddingMedium

        Label {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: "SailTalk"
            color: Theme.highlightColor
        }

        Label {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: appEngine.callState
            color: Theme.primaryColor
        }
    }
}

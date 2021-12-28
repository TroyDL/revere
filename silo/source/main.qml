import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.2
import QtGraphicalEffects 1.12
import Qt.labs.platform 1.1 as Platform

Window
{
    id: win
    visible: true
    title: qsTr("Silo")
    color: "white"
    width: 640; height: 480

    StackView {
        id: stack
        initialItem: mainView
        anchors.fill: parent
    }

    Component {
        id: mainView

        Row {
            spacing: 10

            Button {
                text: "Cameras"
                onClicked: stack.push(camerasView)
            }
        }
    }

    Cameras {
        id: camerasView
    }

    onClosing: {
        close.accepted = false
        gotoTray.open()
        gotoTray.x = (win.width / 2) - (gotoTray.width / 2)
        gotoTray.y = (win.height / 2) - (gotoTray.height / 2)
    }

    Dialog {
        id: gotoTray
        title: "Minimize to tray?"
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel

        onAccepted: {
            win.hide()
        }
    }

    Platform.SystemTrayIcon {
        visible: true
        icon.source: "qrc:///heart.png"

        onActivated: {
            win.show()
            win.raise()
            win.requestActivate()
        }

        menu: Platform.Menu {
            Platform.MenuItem {
                text: qsTr("Show")
                onTriggered: win.show()
            }
            Platform.MenuItem {
                text: qsTr("Hide")
                onTriggered: win.hide()
            }

            Platform.MenuItem {
                text: qsTr("Quit")
                onTriggered: Qt.quit()
            }
        }
    }
}

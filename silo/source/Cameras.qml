
import QtQuick 2.12
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.11

Component {
    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: parent.width / 2
        Text {
            width: parent.width
            text: "Cameras"
        }
        Button {
            id: popButtonID
            text: "Pop"
            onClicked: stack.pop()
        }
        ListView {
            id: mListViewId
            model: mModelId
            delegate: delegateId
            anchors.top: popButtonID.bottom
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
        }
        ListModel {
            id: mModelId
            ListElement {
                country: "Rwanda"; capital: "Kigali"
            }
            ListElement {
                country: "China"; capital: "Beijing"
            }
            ListElement {
                country: "Japan"; capital: "Tokyo"
            }
            ListElement {
                country: "Nigeria"; capital: "Dakar"
            }
            ListElement {
                country: "France"; capital: "Paris"
            }
        }
        Component {
            id: delegateId
            Rectangle {
                id: rectangleId
                width: parent.width
                height: 50
                color: "beige"
                border.color: "yellowgreen"
                radius: 10

                Text {
                    id: textId
                    anchors.centerIn: parent
                    font.pointSize: 20
                    text: country + " : " + capital
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        console.log("clicked on " + country + " : " + capital)
                        mListViewId.currentIndex = index
                    }
                }
            }
        }
    }
}

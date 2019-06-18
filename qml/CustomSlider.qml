import QtQuick 2.0
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1
import "."  // QTBUG-34418, singletons require explicit import to load qmldir file

Slider {
    style: SliderStyle {
        groove: Rectangle {
            implicitWidth: 100
            implicitHeight: 8*Style.sizeScale
            color: "transparent"
            radius: 8*Style.sizeScale
            border.color: "white"
            border.width: 1*Style.sizeScale
        }
        handle: Rectangle {
            anchors.centerIn: parent
            color: control.pressed ? "lightgray" : "white"
            width: Style.controlSize
            height: Style.controlSize
            radius: 9*Style.sizeScale
        }
    }
}

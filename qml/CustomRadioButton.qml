import QtQuick 2.0
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1
import "."  // QTBUG-34418, singletons require explicit import to load qmldir file

RadioButton {
    style: RadioButtonStyle {
        indicator: Rectangle {
            implicitWidth: Style.controlSize
            implicitHeight: Style.controlSize
            radius: 9*Style.sizeScale
            color: "transparent"
            border.color: control.enabled ? "white" : "darkgrey"
            border.width: 1*Style.sizeScale
            Rectangle {
                anchors.fill: parent
                visible: control.checked
                color: control.enabled ? "white" : "darkgrey"
                radius: 9*Style.sizeScale
                anchors.margins: 4*Style.sizeScale
            }
        }
        label: CustomText {
            text: control.text
            color: control.enabled ? "white" : "darkgrey"
        }
    }
 }

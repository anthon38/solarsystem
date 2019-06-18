import QtQuick 2.0
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1
import "."  // QTBUG-34418, singletons require explicit import to load qmldir file

CheckBox {
    style: CheckBoxStyle {
        indicator: Rectangle {
            implicitWidth: Style.controlSize
            implicitHeight: Style.controlSize
            radius: 3*Style.sizeScale
            color: "transparent"
            border.color: "white"
            border.width: 1*Style.sizeScale
            Rectangle {
                visible: control.checked
                color: "white"
                radius: 1*Style.sizeScale
                anchors.margins: 4*Style.sizeScale
                anchors.fill: parent
            }
        }
        label: CustomText {
            text: control.text
        }
    }
}

import QtQuick 2.0

Item {
    id: fadingTextItem
    property string text: ""
    property string color: "black"
//    property int aTextFontSize: 10
    property int animationTime : 100

    Behavior on opacity { NumberAnimation { duration: animationTime } }

    onTextChanged: {
         fadingTextItem.opacity = 0
         timer.running = true
    }

    Timer {
       id: timer
       running: false
       repeat: false
       interval: animationTime
       onTriggered: {
           junkText.text = text
           fadingTextItem.opacity = 1
       }
    }

    Text {
        id: junkText
        anchors.fill: parent
        text: ""
//        font.pixelSize: aTextFontSize
//        color: aTextColor
        wrapMode: Text.WordWrap
        font.pointSize: 18
        font.weight: Font.Normal
        font.bold: true
        font.capitalization: Font.Capitalize
        color: "white"
    }
}

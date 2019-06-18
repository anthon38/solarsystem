import QtQuick 2.0
import QtQuick.Controls 1.1
import SolarSystem 1.0
import "."  // QTBUG-34418, singletons require explicit import to load qmldir file

Item {
    id: mainItem
    width: 800
    height: 600

    Component.onCompleted: {
        if(OSdetails.getOSName() === 2){
            // Android
            renderer.blurPass = 0;
        }
        Style.sizeScale = Math.round(OSdetails.getDPI()/96);
    }

    Renderer {
        id: renderer
        anchors.fill: parent
        focus: true
        blurPass: 4
        antiAliasingType: Renderer.NOAA

        onAaTypesChanged: {
//            antiAliasingType = Renderer.FXAA
            antiAliasingType = (aaTypes.indexOf(Renderer.MSAA) !== -1)
                    ?  antiAliasingType = Renderer.MSAA
                    :  antiAliasingType = Renderer.SSAA
            aagroup.checkAAAvailability(aaTypes)
        }
    }

    CustomText {
        id: labelSelection
        anchors.top: renderer.top
        anchors.left: renderer.left
        anchors.margins: 20
        font.pointSize: Style.pointSizeBig
        style: Text.Outline
        styleColor: "black"
        text: renderer.selection
    }

    CustomText {
        id: labelInfo
        anchors.top: labelSelection.bottom
        anchors.left: renderer.left
        anchors.margins: 20
        style: Text.Outline
        styleColor: "black"
        text: "Distance: "+renderer.distanceToGround.toFixed(3)+"Km"
    }

    CustomText {
        id: labelDate
        anchors.bottom: renderer.bottom
        anchors.left: renderer.left
        anchors.margins: 20
        style: Text.Outline
        styleColor: "black"
        text: renderer.date
    }

    Image {
        id: timeLineIcon
        anchors.bottom: renderer.bottom
        anchors.right: renderer.right
        anchors.margins: 20
        width: Style.iconSize
        height: width
        source: "../icons/clock_icon.png"
        MouseArea {
            anchors.fill: parent
            onClicked: {
                mainItem.state = (mainItem.state == "timeline") ? "clean" : "timeline";
            }
        }
    }

    Row {
        id: timeLineRow
        spacing: Style.iconSize/2
        anchors.right: timeLineIcon.left
        anchors.bottom: timeLineIcon.bottom
        anchors.rightMargin: spacing
//        visible: false

        Image {
            id: realTimeIcon
            width: Style.iconSize
            height: width
            source: "../icons/playback_reload_icon.png"
            MouseArea {
                anchors.fill: parent
                onClicked: renderer.realTime();
            }
        }

        Image {
            id: speedDownIcon
            width: Style.iconSize
            height: width
            source: "../icons/playback_rew_icon.png"
            MouseArea {
                anchors.fill: parent
                onClicked: renderer.speedDown();
            }
        }

        Image {
            id: playPauseIcon
            width: Style.iconSize
            height: width
            source: (renderer.timeLineRate === 0) ? "../icons/playback_play_icon.png" : "../icons/playback_pause_icon.png"
            MouseArea {
                anchors.fill: parent
                onClicked: (renderer.timeLineRate === 0) ? renderer.realRate() : renderer.pause();
            }
        }

        Image {
            id: speedUpIcon
            width: Style.iconSize
            height: width
            source: "../icons/playback_ff_icon.png"
            MouseArea {
                anchors.fill: parent
                onClicked: renderer.speedUp();
            }
        }
    }

    Image {
        id: listIcon
        anchors.bottom: timeLineIcon.top
        anchors.right: renderer.right
        anchors.margins: 20
        width: Style.iconSize
        height: width
        source: "../icons/star_fav_icon.png"
        MouseArea {
            anchors.fill: parent
            onClicked: {
                mainItem.state = (mainItem.state == "browse") ? "clean" : "browse";
            }
        }
    }

    Rectangle {
        id: bodyList
        anchors.bottom: renderer.bottom
        anchors.right: listIcon.left
        anchors.rightMargin: 20
        width: 100*Style.sizeScale
        height: 200*Style.sizeScale
//        visible: false
        clip: true
        radius: 5
        color: Style.panelColor
        border.color: Style.panelBorderColor
        border.width: 2
        ListView {
            anchors.fill: parent
            anchors.topMargin: 2
            anchors.bottomMargin: anchors.topMargin
            anchors.leftMargin: 5
            anchors.rightMargin: anchors.leftMargin
            boundsBehavior: Flickable.StopAtBounds
            model: renderer.bodies
            delegate: CustomText {
//                style: Text.Outline
//                styleColor: "black"
                text: modelData
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        renderer.goToObject(modelData)
                    }
                }
            }
        }
    }

    Image {
        id: configIcon
        anchors.bottom: listIcon.top
        anchors.right: renderer.right
        anchors.margins: 20
        width: Style.iconSize
        height: width
        source: "../icons/wrench_plus_2_icon.png"
        MouseArea {
            anchors.fill: parent
            onClicked: {
                mainItem.state = (mainItem.state == "config") ? "clean" : "config";
            }
        }
    }

    Rectangle {
        id: configPanel
        anchors.bottom: renderer.bottom
        anchors.right: configIcon.left
        anchors.rightMargin: 20
        width: Math.max(visualsGroupBox.width, aaGroupBox.width, visualsGroupBoxTitle.width, aaGroupBoxTitle.width)
        height: visualsGroupBox.height+aaGroupBox.height+visualsGroupBoxTitle.height+aaGroupBoxTitle.height-2*10+4
                +glowGroupBoxTitle.height+glowSlider.height
//        visible: false
        clip: true
        radius: 5
        color: Style.panelColor
        border.color: Style.panelBorderColor
        border.width: 2

        CustomText {
            id: visualsGroupBoxTitle
            anchors.top: parent.top
            anchors.topMargin: 4
            anchors.left: parent.left
            anchors.leftMargin: 4
//            anchors.right: parent.right
            anchors.rightMargin: 4
            anchors.bottomMargin: 0
            text:qsTr("Display")
        }
        GroupBox {
            id: visualsGroupBox
            anchors.top: visualsGroupBoxTitle.top
            anchors.topMargin: visualsGroupBoxTitle.height-10
//            anchors.top: visualsGroupBoxTitle.bottom
//            title: qsTr("Display") // No customisation
            flat: true
            Column {
                id: visualsGroup
                CustomCheckBox {
                    text: qsTr("Axis")
                    checked: renderer.showAxis
                    onClicked: renderer.showAxis = !renderer.showAxis
                }
                CustomCheckBox {
                    text: qsTr("Orbits")
                    checked: renderer.showOrbits
                    onClicked: renderer.showOrbits = !renderer.showOrbits
                }
            }
        }

        CustomText {
            id: glowGroupBoxTitle
            anchors.top: visualsGroupBox.bottom
            anchors.topMargin: 0
            anchors.left: parent.left
            anchors.leftMargin: 4
//            anchors.right: parent.right
            anchors.rightMargin: 4
            anchors.bottomMargin: 0
            text:qsTr("Glow")
        }
        CustomSlider {
            id: glowSlider
            anchors.top: glowGroupBoxTitle.bottom
            anchors.topMargin: 0
            anchors.left: parent.left
            anchors.leftMargin: 8
            anchors.right: parent.right
            anchors.rightMargin: 8
            width: parent.width-anchors.leftMargin-anchors.rightMargin
            minimumValue: 0
            maximumValue: 8
            stepSize: 1
            onValueChanged: renderer.blurPass = value
//            value: renderer.blurPass
            Component.onCompleted: value = renderer.blurPass
        }

        CustomText {
            id: aaGroupBoxTitle
//            anchors.top: visualsGroupBox.bottom
            anchors.top: glowSlider.bottom
            anchors.topMargin: 0
            anchors.left: parent.left
            anchors.leftMargin: 4
//            anchors.right: parent.right
            anchors.rightMargin: 4
            anchors.bottomMargin: 0
            text:qsTr("Anti-aliasing")
        }
        GroupBox {
            id: aaGroupBox
            anchors.top: aaGroupBoxTitle.top
            anchors.topMargin: aaGroupBoxTitle.height-10
//            title: qsTr("Anti-aliasing") // No customisation
            flat: true
            Column {
                ExclusiveGroup {
                    id: aagroup
                    function checkAAAvailability(typeList) {
                        if (typeList.indexOf(Renderer.MSAA) === -1)
                            msaaButton.enabled = false;
                    }
                }
                CustomRadioButton {
                    id: noaaButton
                    text: qsTr("None")
                    exclusiveGroup: aagroup
                    checked: renderer.antiAliasingType === Renderer.NOAA
                    onClicked: renderer.antiAliasingType = Renderer.NOAA
                }
                CustomRadioButton {
                    id: msaaButton
                    text: qsTr("Multisample")
                    exclusiveGroup: aagroup
                    checked: renderer.antiAliasingType === Renderer.MSAA
                    onClicked: renderer.antiAliasingType = Renderer.MSAA
                }
                CustomRadioButton {
                    id: ssaaButton
                    text: qsTr("Supersample")
                    exclusiveGroup: aagroup
                    checked: renderer.antiAliasingType === Renderer.SSAA
                    onClicked: renderer.antiAliasingType = Renderer.SSAA
                }
                CustomRadioButton {
                    id: fxaaButton
                    text: qsTr("FXAA")
                    exclusiveGroup: aagroup
                    checked: renderer.antiAliasingType === Renderer.FXAA
                    onClicked: renderer.antiAliasingType = Renderer.FXAA
                }
            }
        }
    }

    state: "clean"
    states: [
        State {
            name: "clean"
            AnchorChanges {
                target: configPanel
                anchors.top: renderer.bottom
                anchors.bottom: undefined
            }
            AnchorChanges {
                target: bodyList
                anchors.top: renderer.bottom
                anchors.bottom: undefined
            }
            AnchorChanges {
                target: timeLineRow
                anchors.top: renderer.bottom
                anchors.bottom: undefined
            }
        },
        State {
            name: "timeline"
            AnchorChanges {
                target: configPanel
                anchors.top: renderer.bottom
                anchors.bottom: undefined
            }
            AnchorChanges {
                target: bodyList
                anchors.top: renderer.bottom
                anchors.bottom: undefined
            }
            AnchorChanges {
                target: timeLineRow
                anchors.top: undefined
                anchors.bottom: timeLineIcon.bottom
            }
        },
        State {
            name: "browse"
            AnchorChanges {
                target: configPanel
                anchors.top: renderer.bottom
                anchors.bottom: undefined
            }
            AnchorChanges {
                target: bodyList
                anchors.top: undefined
                anchors.bottom: renderer.bottom
            }
            AnchorChanges {
                target: timeLineRow
                anchors.top: renderer.bottom
                anchors.bottom: undefined
            }
        },
        State {
            name: "config"
            AnchorChanges {
                target: configPanel
                anchors.top: undefined
                anchors.bottom: renderer.bottom
            }
            AnchorChanges {
                target: bodyList
                anchors.top: renderer.bottom
                anchors.bottom: undefined
            }
            AnchorChanges {
                target: timeLineRow
                anchors.top: renderer.bottom
                anchors.bottom: undefined
            }
        }
    ]
    transitions: Transition {
        to: "*"
        AnchorAnimation { duration: 100 }
    }
}

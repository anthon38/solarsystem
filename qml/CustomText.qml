import QtQuick 2.0
import "."  // QTBUG-34418, singletons require explicit import to load qmldir file

Text {
    font.pointSize: Style.pointSizeMedium
    font.capitalization: Font.Capitalize
    color: "white"
}

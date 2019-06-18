#include "viewitem.h"
#include "path.h"
#include "osdetails.h"

#include <QGuiApplication>
#include <QQuickView>
#include <QQmlContext>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QFont font("Trebuchet MS");
//    QFont font("Droid Sans");
    app.setFont(font);

    qmlRegisterType<ViewItem>("SolarSystem", 1, 0, "Renderer");

    QQuickView view;
    view.setIcon(QIcon(resPath()+"icons/icon.png"));
    view.setTitle("Solar system");
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    OSDetails osdetails(view.screen());
    view.rootContext()->setContextProperty("OSdetails",&osdetails);
    view.setSource(QUrl(resPath()+"qml/main.qml"));
    view.showMaximized();

    return app.exec();
}

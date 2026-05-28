#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("TegraRcm");
    QCoreApplication::setApplicationName("TegraRcmGUI");

    QApplication app(argc, argv);

    QQuickStyle::setStyle("Fusion");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/qt/qml/TegraRcm/main.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}


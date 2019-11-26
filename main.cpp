#include "gratuitous.h"
#include "palettes.h"

#include <QApplication>
#include <QObject>
#include <QStyleFactory>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setStyle("QtCurve");
    QIcon::setThemeName("breeze-dark");
    QApplication::setPalette(GPalette::dark());
    a.setStyleSheet(GPalette::dark_palette_stylesheet);

    QCoreApplication::setOrganizationName("gratuitous");
    QCoreApplication::setOrganizationDomain("jechasteen.com");
    QCoreApplication::setApplicationName("gratuitous");

    Gratuitous w;
    w.show();

    // TODO: fix this garbage, w.quit() should handle everything
    QObject::connect(&a, &QApplication::aboutToQuit, &w, [&w]() { w.quit(); qDebug() << "force quit"; });

    return a.exec();
}

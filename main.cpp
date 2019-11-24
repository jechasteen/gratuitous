#include "gratuitous.h"
#include "palettes.h"

#include <QApplication>
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
    return a.exec();
}

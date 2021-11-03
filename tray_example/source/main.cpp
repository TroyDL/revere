#include <QApplication>

#include <QMessageBox>
#include "window.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(tray_example);

    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication app(argc, argv);

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(nullptr, QObject::tr("Systray"),
                              QObject::tr("I couldn't detect any system tray "
                                          "on this system."));
        return 1;
    }
    QApplication::setQuitOnLastWindowClosed(false);

    Window window;
    window.show();
    window.setVisible(false);
    return app.exec();
}

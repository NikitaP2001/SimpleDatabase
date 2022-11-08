#include <memory>

#include <QApplication>
#include <QLabel>
#include <QWidget>

#include "mainwindow.h"

int main(int argc, char *argv[ ])
{
QApplication app(argc, argv);
auto wnd = std::make_unique<MainWindow>();
wnd->show();

return app.exec();
}

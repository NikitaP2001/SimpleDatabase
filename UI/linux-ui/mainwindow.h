#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <memory>

#include <QFrame>
#include <QMainWindow>

#include "lexer.h"
#include "database.h"
#include "parser.h"
#include "error.hpp"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
        Q_OBJECT

        enum Forms {
                CREATE_FORM,
                QUERY_FORM
        };

public:
        explicit MainWindow(QWidget *parent = nullptr);
        ~MainWindow();

protected:
        void keyPressEvent(QKeyEvent *event);

private:

        void executeQuery();

        void showOnTop(Forms form);

        void displayOnGrid(database::Response &res);

        void showOpen();

        void openLocal();

        void clearGrid();

private:
        std::unique_ptr<Ui::MainWindow> m_ui;
        std::vector<QFrame*> m_windows;
        std::string m_databasePath;
};

#endif // MAINWINDOW_H

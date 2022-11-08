#include <vector>

#include <QMessageBox>
#include <QTranslator>
#include <QFileDialog>
#include <QKeyEvent>



#include "mainwindow.h"
#include "ui_mainwindow.h"

using namespace lexer;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow)
{
        m_ui->setupUi(this);

        Qt::WindowFlags flags = Qt::Widget;
        setWindowFlags(flags);

        m_windows.push_back(m_ui->create_Frame);
        m_windows.push_back(m_ui->query_Frame);

        for (auto *wnd : m_windows)
        {
                wnd->hide();
                wnd->move(0, 0);
        }

        connect(m_ui->create_btnLocal, &QPushButton::clicked, this, &MainWindow::openLocal);
        connect(m_ui->query_btnOpen, &QPushButton::clicked, this, &MainWindow::showOpen);
        connect(m_ui->query_btnonExec, &QPushButton::clicked, this, &MainWindow::executeQuery);

        showOnTop(CREATE_FORM);
}


void MainWindow::showOnTop(Forms form)
{
        for (auto *wnd : m_windows)
                wnd->hide();
        setFixedSize(m_windows[form]->size());
        m_windows[form]->show();
}

void MainWindow::displayOnGrid(database::Response &res)
{
        Column col;
        int c = 0;

        m_ui->query_gridCols->setRowCount(res.getRowCount());
        m_ui->query_gridCols->setColumnCount(res.getColCount());

        QStringList labels;
        for ( ; res.getColumn(col); c++) {
                labels << QString::fromStdString(col.name);
                m_ui->query_gridCols->setHorizontalHeaderLabels(labels);
                for (uint32_t r = 0; r < col.values.size(); r++) {
                        auto *item = new QTableWidgetItem(QString::fromStdString(
                        col.values[r]->GetValue()));
                        item->setFlags(item->flags() & !Qt::ItemIsEditable);
                        m_ui->query_gridCols->setItem(r, c, item);
                }
        }
}

void MainWindow::executeQuery()
{
        Lexer lex;
        Storage st(m_databasePath);
        parser::Parser prs(std::make_shared<Storage>(st));
        std::vector<lexer::token> tokens;
        lexer::token tok(stmt::type::type_error, token::value(""));

        std::string squery = m_ui->query_Edit->text().toStdString();
        auto sit = squery.begin();
        m_ui->query_Edit->clear();

        while (sit != squery.end()) {
                sit = lex.getToken(sit, squery.end(), tok);
                if (sit == squery.end()) {
                        QMessageBox::warning(this, tr("Failture"),
                        tr("syntax error"));
                        break;
                }
                sit++;
                tokens.push_back(tok);
        }

        clearGrid();
        if (tokens.begin() != tokens.end()) {
                auto itTokBeg = tokens.begin();
                auto root =
                prs.getTree(stmt::type::query, itTokBeg, tokens.end());
                if (root && itTokBeg == tokens.end()) {
                        try {
                                auto res = root->execute("");
                                displayOnGrid(res);

                        } catch (std::exception &ex) {
                                QMessageBox::critical(this, tr("Error"), ex.what());
                        }

            } else {
                    QMessageBox::warning(this, tr("Failture"),
                    tr("Parsing error"));
            }
        }
}


void MainWindow::keyPressEvent(QKeyEvent *event)
{
        if((event->key() == Qt::Key_Enter) || (event->key() == Qt::Key_Return))
                executeQuery();
}


void MainWindow::openLocal()
{
        QString dbDir = QFileDialog::getExistingDirectory(this,
                                        tr("Open Directory"),
                                        QDir::home().absolutePath(),
                                        QFileDialog::ShowDirsOnly
                                        | QFileDialog::DontResolveSymlinks);
        if (dbDir.size() == 0) {
                QMessageBox::critical(this, tr("Error"),
                tr("Failed to get database path"));
        } else {
                m_databasePath = dbDir.toStdString();
                clearGrid();
                showOnTop(QUERY_FORM);
        }
}

void MainWindow::clearGrid()
{
        m_ui->query_gridCols->setRowCount(0);
}

void MainWindow::showOpen()
{
        m_databasePath.clear();
        showOnTop(CREATE_FORM);
}

MainWindow::~MainWindow()
{
    clearGrid();
}

#include "mainwindow.h"

#include <QApplication>

//
// NOTE
// Adding a QPushButton to a table cell:
//
//    QWidget* pWidget = new QWidget();
//    QPushButton* btn_edit = new QPushButton();
//    btn_edit->setText("Edit");
//    QHBoxLayout* pLayout = new QHBoxLayout(pWidget);
//    pLayout->addWidget(btn_edit);
//    pLayout->setAlignment(Qt::AlignCenter);
//    pLayout->setContentsMargins(0, 0, 0, 0);
//    pWidget->setLayout(pLayout);
//    ui.table->setCellWidget(i, 3, pWidget);
//

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(silo);

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
